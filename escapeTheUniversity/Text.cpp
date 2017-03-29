#include "Text.hpp"
#include "Model/ModelLoader.hpp"
#include "Shader.hpp"
#include "RenderLoop.hpp"
#include "Debug\Debugger.hpp"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace std;

Text::~Text(){
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Text::init(){
	ModelLoader* ml = ModelLoader::getInstance();
	textShader = new Shader("text");
	textShader->useProgram();

	/// Quad set-up
	float vertices[] = {
		//Position x(lr)y(tb)//Texture coordinates xy
		-0.5f, 0.5f, 0.0f, 0.0f, //left-top, 0
		-0.5f,-0.5f, 1.0f, 1.0f, //left-bottom, 1
		 0.5f, 0.5f, 1.0f, 0.0f, //right-top, 2 
		 0.5f,-0.5f, 0.0f, 1.0f  //right-bottom, 3
	};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	const unsigned int elements[] = {
		0, 1, 2,
		2, 1, 3
	};

	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, previousMaxBufferSize = sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (GLvoid*)0);
	glEnableVertexAttribArray(positionLocation);

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
	glBindVertexArray(0); // Unbind VAO

	characterTextureHandle =ml->loadPicture(ml->MODEL_DIR + loadingImagePath); // Load and bind textures

	if (characterTextureHandle < 0)
		Debugger::getInstance()->pauseExit("Malfunction: Character image " + ml->MODEL_DIR + loadingImagePath + " not found.");

	// Character texture coordination calculation
	const float imageSize = 2048.0f; // In pixels
	const unsigned int maxColumn = imageSize / charSize;

	for (unsigned int i = FIRST_CHARACTER, column = 0, row = 0, x = 0, y = 0; 
		i < LAST_CHARACTER; 
		i++, column = (i - FIRST_CHARACTER) % maxColumn)
	{
		x = column * charSize;
		y = imageSize - row * charSize; 
		std::vector<float> coords = { 
			//left-top    --  right-top
			//  |         /      |
			//left-bottom -- right-bottom, 4 points form 2 triangles, draw as triangle strip
			float(x) / imageSize, float(y) / imageSize,            //left-top, 0
			float(x) / imageSize, float(y - charSize) / imageSize, //left-bottom, 1
			float(x + charSize) / imageSize, float(y) / imageSize,            //right-top, 2 
			float(x + charSize) / imageSize, float(y - charSize) / imageSize  //right-bottom, 3
		};

		charLocations[(unsigned char)i] = glm::make_mat4x2(&coords[0]);
		
		if (column == maxColumn - 1)
			row++;
	}
}

void Text::write(const char* text, float x, float y, const float scale, const float angle){
	textShader->useProgram();

	glm::mat4 trans = glm::mat4(1);
	trans = glm::scale(trans, glm::vec3(scale, scale, 1));
	trans = glm::rotate(trans, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));

	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
	glUniform4f(colorScaleLocation, color.x, color.y, color.z, scale);

	vector<float>* vertices = new vector<float>;
	float cursor = 0.0f, row = 0.0f;
	const float advance = scale * charSize / (float) RenderLoop::getInstance()->width;
	x /= scale;
	y /= scale;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, characterTextureHandle);
	glBindVertexArray(VAO);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (const char* p = text; *p; p++, cursor += advance)
	{
		if (*p == '\n')
		{
			row -= advance;
			cursor = -advance;
			writeVertices(vertices);
			vertices->clear();
			continue;
		}

		const glm::mat4x2 uv = charLocations[(unsigned char)*p];
		//Position.x(lr)y(tb),Texture coordinates uv.xy
		vertices->push_back(x + cursor);//left-top, 0 x
		vertices->push_back(y + row + advance); // y
		vertices->push_back(uv[0].x); //uv.x
		vertices->push_back(uv[0].y); //uv.y
		vertices->push_back(x + cursor); //left-bottom, 1 x
		vertices->push_back(y + row); // ...
		vertices->push_back(uv[1].x);
		vertices->push_back(uv[1].y);
		vertices->push_back(x + advance + cursor); //right-top, 2 x
		vertices->push_back(y + row + advance);
		vertices->push_back(uv[2].x);
		vertices->push_back(uv[2].y);
		vertices->push_back(x + advance + cursor); //right-bottom, 3 x
		vertices->push_back(y + row);
		vertices->push_back(uv[3].x);
		vertices->push_back(uv[3].y);

	}
	writeVertices(vertices);
	delete vertices;

	glDisable(GL_BLEND);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}   

void Text::writeVertices(vector<float>*vertices) 
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	const unsigned int size = vertices->size();
	unsigned int currentBufferSize = size * sizeof(float);

	if (currentBufferSize > previousMaxBufferSize) // Buffer size too small, allocate more space
		glBufferData(GL_ARRAY_BUFFER, previousMaxBufferSize = currentBufferSize, vertices->data(), GL_DYNAMIC_DRAW);
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, currentBufferSize, vertices->data());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, size/4); //Each vertex has two xy and two uv entries, ergo divide by four for correct vertex amount	
}


void Text::fps(const double pastTime, const double deltaTime, const unsigned int drawnTriangles)
{
	if (timeThreashold < pastTime)
	{
		timeThreashold = pastTime + 1; // Only print all seconds not MS (it is frames per second not seconds per frame)
		snprintf(fpsBuffer, 30, "FPS:%.0f\nTriangles:%u", (1.0 / deltaTime), drawnTriangles);
	}
	
	write(fpsBuffer, -0.9f, 0.9f, 0.5f, 0.0f);
}

void Text::loadingScreenInfo(){
	char infoText[1024] = "Working on:\n";
	unsigned int i;
	for (i = 0; infoText[i] != '\0'; i++);

	i = copyInBuffer(infoText, i, glGetString(GL_VENDOR));
	i = copyInBuffer(infoText, i, glGetString(GL_VERSION));
	i = copyInBuffer(infoText, i, glGetString(GL_RENDERER));

	string temp = "\nOpenGL Version: ";
	temp.append((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)); // TODO buggy linebreak

	int param = 0;
	glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &param);
	temp.append("\nMaximal 3D Texture Size: " + param);
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &param);
	temp.append("\nMaximal texture size: " + param);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &param);
	temp.append("\nMaximal texture image units: " + param);
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &param);
	temp.append("\nMaximal uniform buffer binding points: " + param);
	////glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &param); // Prodcues error
	////temp.append("\nMaximal uniform block size: " + param);
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &param);
	temp.append("\nMaximal color attachments: " + param);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_HEIGHT, &param);
	temp.append("\nMaximal framebuffer height: " + param);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_WIDTH, &param);
	temp.append("\nMaximal framebuffer width: " + param);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_SAMPLES, &param);
	temp.append("\nMaximal framebuffer samples: " + param);
	glGetIntegerv(GL_MAX_FRAMEBUFFER_LAYERS, &param);
	temp.append("\nMaximal framebuffer layers: " + param);

	i = copyInBuffer(infoText, i, (const unsigned char*)temp.c_str());
	write(infoText, -1.0f, 1.0f, 0.45f, 0.0f);
}

unsigned int Text::copyInBuffer(char buffer[], unsigned int i, const unsigned char* toCopy)
{
	while (*toCopy)
		buffer[i++] = *toCopy++;
	buffer[i++] = '\n';
	return i;
}

void Text::pause(){
	write("Game paused", -0.3f, -0.01f, 0.6f, 0.0f);
}

void Text::help()
{
	char* help = "Keybindings";
	write(help, -1.0, 0.9, 0.7, 0.0f);
	help = "F1 = Help\nF2 = Toggle FPS\n..."; // TODO complete the list
	write(help, -1.0, 0.8, 0.5, 0.0f);
}