#include <algorithm>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Text.hpp"
#include "Model/ModelLoader.hpp"
#include "Shader.hpp"
#include "RenderLoop.hpp"
#include "Debug\Debugger.hpp"


using namespace std;

Text::~Text(){
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Text::init(){
	ModelLoader* ml = ModelLoader::getInstance();
	textShader = new Shader("text.vert", "text.frag");
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

		//for_each(coords.begin(), coords.end(), [imageSize](float& f){f /= imageSize;});
		charLocations[(unsigned char)i] = glm::make_mat4x2(&coords[0]);
		
		if (column == maxColumn - 1)
			row++;
	}
}

void Text::write(const char* text){
	textShader->useProgram();

	float scale = 1.0f; // Text transformation params
	float angle = 0.0f;
	glm::mat4 trans = glm::mat4(1);
	trans = glm::scale(trans, glm::vec3(scale, scale, 1));
	trans = glm::rotate(trans, glm::radians(angle), glm::vec3(0.0, 0.0, 1.0));

	glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans));
	glUniform1f(scaleLocation, scale);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, characterTextureHandle);
	glBindVertexArray(VAO);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	vector<float> vertices;
	float x = 0.1f; // Position of first character on screen in screen coords
	float y = 0.1f;
	float cursor = 0.0f;
	const float advance = charSize / (float) RenderLoop::getInstance()->width;

	for (const char* p = text; *p; p++, cursor += advance)
	{
		const glm::mat4x2 uv = charLocations[(unsigned char)*p];
		//Position.x(lr)y(tb),Texture coordinates uv.xy
		vertices.push_back(-x + cursor);//left-top, 0 x
		vertices.push_back(y); // y
		vertices.push_back(uv[0].x); //uv.x
		vertices.push_back(uv[0].y); //uv.y
		vertices.push_back(-x + cursor); //left-bottom, 1 x
		vertices.push_back(-y); // ...
		vertices.push_back(uv[1].x);
		vertices.push_back(uv[1].y); 
		vertices.push_back(x + cursor); //right-top, 2 x
		vertices.push_back(y);
		vertices.push_back(uv[2].x);
		vertices.push_back(uv[2].y); 
		vertices.push_back(x + cursor); //right-bottom, 3 x
		vertices.push_back(-y);
		vertices.push_back(uv[3].x);
		vertices.push_back(uv[3].y);
	}

	// TODO skalierung in die position mit einbauen

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	unsigned int currentBufferSize = vertices.size() * sizeof(float);

	if (currentBufferSize > previousMaxBufferSize) // Buffer size too small, allocate more space
		glBufferData(GL_ARRAY_BUFFER, previousMaxBufferSize = currentBufferSize, &vertices[0], GL_DYNAMIC_DRAW);
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, currentBufferSize, &vertices[0]);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertices.size()/4); //Each vertex has two xy and two uv entries, ergo divide by four for correct vertex amount
	glDisable(GL_BLEND);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               