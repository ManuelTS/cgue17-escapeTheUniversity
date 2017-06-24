#include "Debugger.hpp"
#include "../Model/ModelLoader.hpp"
#include "../Model/Node/ModelNode.hpp"
#include "../RenderLoop.hpp"
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GLM\gtc\type_ptr.hpp>
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include "MemoryLeakTracker.h"

using namespace std;

Debugger::~Debugger()
{
	if (shadowDebug != nullptr)
		delete shadowDebug;

	if(quadVAO != 0)
	{
		glDeleteBuffers(1, &quadVBO);
		glDeleteVertexArrays(1, &quadVAO);
	}
}

void Debugger::renderShadowMap(float farPlane, unsigned int depthMapTextureHandle)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (shadowDebug == nullptr)
		shadowDebug = new Shader("shadowDebug");

	const unsigned int NEAR_PLANE_LOCATION = 0; // in shadow_debug_frag
	const unsigned int FAR_PLANE_LOCATION = 1;
	const unsigned int DEPTH_MAP_LOCATION = 0;

	shadowDebug->useProgram();

	glUniform1f(NEAR_PLANE_LOCATION, 0.1f); // Check also shadowMapping#renderInDepthMap
	glUniform1f(FAR_PLANE_LOCATION, farPlane * 2);
	glActiveTexture(GL_TEXTURE0 + DEPTH_MAP_LOCATION);
	glBindTexture(GL_TEXTURE_2D, depthMapTextureHandle);

	if (quadVAO == 0) // if not generated generate
	{
		const unsigned int POSITON_LOCATION = 0; // in shadow_debug_vert
		const unsigned int IN_TEXT_COORDS_LOCATION = 1;
		const float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};

		glGenVertexArrays(1, &quadVAO);		// setup plane VAO
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);

		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(POSITON_LOCATION);
		glVertexAttribPointer(POSITON_LOCATION, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(IN_TEXT_COORDS_LOCATION);
		glVertexAttribPointer(IN_TEXT_COORDS_LOCATION, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
	glActiveTexture(GL_TEXTURE0 + DEPTH_MAP_LOCATION);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Debugger::drawLightBoundingSpheres(LightNode* ln) // Draws the light spheres based on their location and intensity radius
{
	ModelLoader* ml = ModelLoader::getInstance();
	bool actualBounding = ml->sphere01->bounding;
	ml->sphere01->bounding = false; // Draw regardless of bounding

	glm::mat4 m = glm::scale(glm::translate(glm::mat4(), glm::vec3(ln->light.position)), glm::vec3(ln->lightSphereRadius)); // Translate to light center and then scale the sphere to the light radius
	ml->sphere01->hirachicalModelMatrix = m; // Set new model matrix
	ml->sphere01->inverseHirachicalModelMatrix = glm::inverseTranspose(m);
	RenderLoop::getInstance()->pureDraw(ml->sphere01);

	ml->sphere01->bounding = actualBounding; // Sphere position overwritten on its next rendering
}

void Debugger::writeAllVertices(vector<float>* vertices, string fileNameWithoutEnding)
{
	string coords = "";

	for (int i = 0; i < vertices->size(); i += 3)
	{
		char yes[50];
		sprintf(yes, "i = %4d, x = %4.2f, y = %4.2f, z = %4.2f\n", i, vertices->at(i), vertices->at(i + 1), vertices->at(i + 2));
		coords = coords.append(yes);
	}

	writeLogFile(fileNameWithoutEnding, coords);
}

void Debugger::writeLogFile(string fileNameWithoutEnding, string text) {
	ofstream out(logFilePath + fileNameWithoutEnding + ".log");

	out << text << endl;

	out.close();
}

//In case a GLFW function fails, an error is reported to the GLFW error callback
void Debugger::errorCallback(int error, const char* description)
{
	cerr << description << " with the error code: " << error << endl;
}
// To see how long something needed to execute on the GPU, from: http://www.lighthouse3d.com/tutorials/opengl-timer-query/
void Debugger::startGPUTimer(){
	glGenQueries(1, &query);
	glBeginQuery(GL_TIME_ELAPSED, query);

	// call some OpenGL commands
}

/*Stops the timing function, waits until the result is ready and then callculates the time and pauses afterwards.*/
void Debugger::stopGPUTimer(){
	// After call of some OpenGL commands
	glEndQuery(GL_TIME_ELAPSED);

	// retrieving the recorded elapsed time
	// wait until the query result is available
	unsigned int done = 0;
	while (!done)
		glGetQueryObjectuiv(query, GL_QUERY_RESULT_AVAILABLE, &done);

	// get the query result
	unsigned long long elapsed_time;
	glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
	printf("Milliseconds Elapsed: %f\n", elapsed_time / 1000000.0);
	system("PAUSE");
}

// Use this to store the current time for the CPU.
void Debugger::startCPUTimer(){
	cpuStart = time(0);
}
// Stop get the current CPU time and calculate it with the previously stored one.
void Debugger::stopCPUTimer(){
	time_t cpuStop = time(0);

	printf("Milliseconds Elapsed: %f\n", difftime(cpuStop,cpuStart) * 1000.0);
	system("PAUSE");
}

string formatDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, const char* msg) {
	string sourceString;
	string typeString;
	string severityString;

	// The AMD variant of this extension provides a less detailed classification of the error,
	// which is why some arguments might be "Unknown".
	switch (source) {
	case GL_DEBUG_CATEGORY_API_ERROR_AMD:
	case GL_DEBUG_SOURCE_API: {
		sourceString = "API";
		break;
	}
	case GL_DEBUG_CATEGORY_APPLICATION_AMD:
	case GL_DEBUG_SOURCE_APPLICATION: {
		sourceString = "Application";
		break;
	}
	case GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD:
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: {
		sourceString = "Window System";
		break;
	}
	case GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD:
	case GL_DEBUG_SOURCE_SHADER_COMPILER: {
		sourceString = "Shader Compiler";
		break;
	}
	case GL_DEBUG_SOURCE_THIRD_PARTY: {
		sourceString = "Third Party";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_SOURCE_OTHER: {
		sourceString = "Other";
		break;
	}
	default: {
		sourceString = "Unknown";
		break;
	}
	}

	switch (type) {
	case GL_DEBUG_TYPE_ERROR: {
		typeString = "Error";
		break;
	}
	case GL_DEBUG_CATEGORY_DEPRECATION_AMD:
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: {
		typeString = "Deprecated Behavior";
		break;
	}
	case GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD:
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: {
		typeString = "Undefined Behavior";
		break;
	}
	case GL_DEBUG_TYPE_PORTABILITY_ARB: {
		typeString = "Portability";
		break;
	}
	case GL_DEBUG_CATEGORY_PERFORMANCE_AMD:
	case GL_DEBUG_TYPE_PERFORMANCE: {
		typeString = "Performance";
		break;
	}
	case GL_DEBUG_CATEGORY_OTHER_AMD:
	case GL_DEBUG_TYPE_OTHER: {
		typeString = "Other";
		break;
	}
	default: {
		typeString = "Unknown";
		break;
	}
	}

	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH: {
		severityString = "High";
		break;
	}
	case GL_DEBUG_SEVERITY_MEDIUM: {
		severityString = "Medium";
		break;
	}
	case GL_DEBUG_SEVERITY_LOW: {
		severityString = "Low";
		break;
	}
	default: {
		severityString = "Unknown";
		break;
	}
	}

	stringstream ss;
	ss << "OpenGL Error: " << msg;
	ss << " [Source = " << sourceString;
	ss << ", Type = " << typeString;
	ss << ", Severity = " << severityString;
	ss << ", ID = " << id << "]";

	return ss.str();
}

void APIENTRY debugCallbackAMD(GLuint id, GLenum category, GLenum severity, GLsizei length, const GLchar* message, GLvoid* userParam) {
	if (id != 131185) // TODO bugfix that error, happens for now only in vislab 
		cerr << formatDebugOutput(category, category, id, severity, message) << endl;
}

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if (id != 131185) // TODO bugfix that error, happens for now only in vislab 
		cerr << formatDebugOutput(source, type, id, severity, message) << endl;
}

void Debugger::setDebugContext()
{
	PFNGLDEBUGMESSAGECALLBACKPROC _glDebugMessageCallback = (PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback");
	PFNGLDEBUGMESSAGECALLBACKARBPROC _glDebugMessageCallbackARB = (PFNGLDEBUGMESSAGECALLBACKARBPROC)wglGetProcAddress("glDebugMessageCallbackARB");
	PFNGLDEBUGMESSAGECALLBACKAMDPROC _glDebugMessageCallbackAMD = (PFNGLDEBUGMESSAGECALLBACKAMDPROC)wglGetProcAddress("glDebugMessageCallbackAMD");

	// Register your callback function.
	if (_glDebugMessageCallback != NULL)
		_glDebugMessageCallback(debugCallback, NULL);
	else if (_glDebugMessageCallbackARB != NULL)
		_glDebugMessageCallbackARB(debugCallback, NULL);
	else if (_glDebugMessageCallbackAMD != NULL)
		_glDebugMessageCallbackAMD(debugCallbackAMD, NULL);

	// Enable synchronous callback. This ensures that your callback function is called
	// right after an error has occurred. This capability is not defined in the AMD
	// version.
	if ((_glDebugMessageCallback != NULL) || (_glDebugMessageCallbackARB != NULL))
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}


/*Terminates GLFW, pauses the System and exits afterwards.*/
void Debugger::pauseExit()
{
	pauseExit(nullptr);
}

/*Prints the msg if not nullptr, pauses the System and exits afterwards.*/
void Debugger::pauseExit(string msg)
{
	pauseExit(msg.c_str());
}

/*Prints the msg if not nullptr, pauses the System and exits afterwards.*/
void Debugger::pause(const char* msg)
{
	if (msg)
		cerr << msg << endl;

	system("PAUSE");
}

/*Prints the msg if not nullptr, pauses the System and exits afterwards.*/
void Debugger::pauseExit(const char* msg)
{
	if (msg)
		cerr << "Malfunction: " << msg << endl;

	system("PAUSE");
	exit(EXIT_FAILURE);
}

/*Checks the whole framebuffer completeness and prints error messages if incomplete.*/
void Debugger::checkWholeFramebufferCompleteness(){
	switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
	{
		case GL_FRAMEBUFFER_UNDEFINED:
			pauseExit("The default framebuffer is undefined.");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT :
			pauseExit("A framebuffer attachment point is framebuffer attachment incomplete.");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			pauseExit("There is at least one image unattached to the framebuffer, or one value of the framebuffer’s FRAMEBUFFER_DEFAULT_WIDTH or FRAMEBUFFER_DEFAULT_HEIGHT parameters is zero.");
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			pauseExit("The combination of internal formats of the attached images violates an implementation or a dependent set of restrictions, respectively.");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			pauseExit("The value of RENDERBUFFER_SAMPLES is not the same for all attached render-buffers; the value of TEXTURE_SAMPLES is not the same for all attached textures; or, if the attached images are a mix of renderbuffers and textures, the value of RENDERBUFFER_SAMPLES matches not the value of TEXTURE_SAMPLES");
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			pauseExit("If any framebuffer attachment is layered, all populated attachments must be layered.Additionally, all populated color attachments must be from textures of the same target(three - dimensional, one - or two - dimensional array, cube map, or cube map array textures).");
			break;
		case 0:
			pauseExit("glCheckFramebufferStatus(GL_FRAMEBUFFER) generated an error.");
			break;
		case GL_FRAMEBUFFER_COMPLETE:
			// All good
			break;
		default:
			pauseExit("Invalid return value from glCheckFramebufferStatus(GL_FRAMEBUFFER).");
			break;
	}
}

/*Prints all framebuffer attachments.*/
void Debugger::printFrameBufferAttachments()
{
	int attachmentNumber = 0, result = 0;
	
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &attachmentNumber);
	
	cout << "---" << endl;
	cout << "Displaing all " << attachmentNumber << " framebuffer attachments:" << endl;

	for (int a = 0; a < attachmentNumber; a++)
	{
		cout << "GL_COLOR_ATTACHMENT" << a << ":" << endl;
		cout << " Zero value means the component is absent in GL_COLOR_ATTACHMENT" << a << "." << endl;
		cout << " Color RGBA, depth, and stencil:" << a << "." << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE: " << result << " bits" << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE: " << result << " bits" << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE: " << result << " bits" << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE: " << result << " bits" << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE: " << result << " bits" << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE: " << result << " bits" << endl;
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING: " << getAttachmentColorEncoding(result) << endl;

		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE: " << getAttachmentComponentType(result) << endl;

		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE: " << getAttachmentObjectType(result) << endl;
		
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME: " << result << endl;
		
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL mipmap level: " << result << endl;
		
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE: " << result << endl;
		
		glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a, GL_FRAMEBUFFER_ATTACHMENT_LAYERED, &result);
		cout << " GL_FRAMEBUFFER_ATTACHMENT_LAYERED bool: " << result << endl;
	}
}


std::string Debugger::getAttachmentComponentType(int param){
	switch (param)
	{
		case GL_FLOAT: return "GL_FLOAT";
		case GL_INT:   return "GL_INT";
		case GL_UNSIGNED_INT: return "GL_UNSIGNED_INT";
		case GL_SIGNED_NORMALIZED: return "GL_SIGNED_NORMALIZED";
		case GL_UNSIGNED_NORMALIZED: return "GL_UNSIGNED_NORMALIZED";
		default: return"Unkown GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE";
   }
}

std::string Debugger::getAttachmentObjectType(int param){
	switch (param)
	{
		case GL_NONE: return "GL_NONE";
		case GL_FRAMEBUFFER_DEFAULT:   return "GL_FRAMEBUFFER_DEFAULT";
		case GL_TEXTURE: return "GL_TEXTURE";
		case GL_RENDERBUFFER: return "GL_RENDERBUFFER";
		default: return"Unkown GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE";
	}
}

std::string Debugger::getAttachmentColorEncoding(int param){
	switch (param)
	{
	case GL_LINEAR: return "GL_LINEAR";
	case GL_SRGB:   return "GL_SRGB";
	default: return"Unkown GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING";
	}
}