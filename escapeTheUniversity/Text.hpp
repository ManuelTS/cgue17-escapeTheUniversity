#pragma once
#include <GLM\glm.hpp>
#include <string>
#include <map>

class Shader;

/*Renders text to the screen with signed distance field character map. This class has the singelton design pattern implemented.*/
class Text{
private:
	// Image path and name
	const std::string loadingImagePath = "characters-df.png";
	const unsigned int charSize = 128; // Pixelchar size in image
	// Locations in the text.vert and -.frag shaders.
	const unsigned int positionLocation = 0; // Usage in text.vert
	const unsigned int textCoordsLocation = 1; // Usage in text.vert
	const unsigned int transformLocation = 0; // Usage in text.vert
	const unsigned int scaleLocation = 4; // Usage in text.frag
	// Handles
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
	//Character texture coordinates
	std::map<unsigned char, glm::mat4x2, std::less<unsigned char> > charLocations;
	// Max buffer size
	unsigned int previousMaxBufferSize = 0;

	Shader* textShader;
	unsigned int characterTextureHandle = 0;

	Text(void){}; // Private constructor to allow only one instance
	Text(Text const&); // Private constructor to prevent copies
	void operator=(Text const&); // Private constructor to prevent assignments

public:
	~Text();

	//First supported extended ASCII character
	static const unsigned char FIRST_CHARACTER = ' ';
	//Last supported extended ASCII character
	static const unsigned char LAST_CHARACTER = 'ÿ';

	/*Returns the pointer to the unique instance of this class.*/
	static Text* Text::getInstance()
	{
		static Text instance;// lazy singleton, instantiated on first use
		return &instance;
	}

	/*Initializes this class for rendering text to the screen.*/
	void init();
	/*Renders text to the screen.*/
	void write(const char* text);
};