#pragma once
#include <GLM\glm.hpp>
#include <string>
#include <map>
#include <vector>

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
	const unsigned int colorScaleLocation = 4; // Usage in text.frag
	// Handles
	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
	//Character texture coordinates
	std::map<unsigned char, glm::mat4x2, std::less<unsigned char> > charLocations;
	// Max buffer size
	unsigned int previousMaxBufferSize = 0;
	// Last time since draw, used in FPS rendering to update the FPS only each second
	double timeThreashold = 0.0;
	float displayTime = 0.0f; // Contains the remaining time in mili seconds to display text on the screen
	// Buffers
	char fpsBuffer[30]; // FPS character buffer
	const glm::vec3 DEFAULT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f); // Default color of text
	glm::vec3 color = DEFAULT_COLOR; // Color of text

	Shader* textShader;
	unsigned int characterTextureHandle = 0;

	Text(void){}; // Private constructor to allow only one instance
	Text(Text const&); // Private constructor to prevent copies
	void operator=(Text const&); // Private constructor to prevent assignments

	unsigned int copyInBuffer(char buffer[], unsigned int i, const unsigned char* toCopy, const bool linebreak);// Copies a char* into an char []
	void writeVertices(std::vector<float>* vertices); // Writes the actual caracters denoted by the vector onto the screen
public:
	~Text();

	//First supported extended ASCII character
	static const unsigned char FIRST_CHARACTER = ' ';
	//Last supported extended ASCII character
	static const unsigned char LAST_CHARACTER = '�';

	/*Returns the pointer to the unique instance of this class.*/
	static Text* Text::getInstance()
	{
		static Text instance;// lazy singleton, instantiated on first use
		return &instance;
	}

	/*Initializes this class for rendering text to the screen.*/
	void init();
	/*Renders text to the screen. "\n" for a linebreak is possible.
		text to write
		xy position in screen coordinates
		scale of the text
		angle of the text, zero is horizontal*/
	void write(const char* text, float x, float y, const float scale, const float angle);
	void fps(const double pastTime, const double deltaTime, const unsigned int drawnTriangles); // Shows the FPS on screen and updates them each second
	void loadingScreenInfo(); // Displays the loading text computer info on screen
	void pause(); // Displays the pause or resume text on screen, the latter for one second more
	void help(); // Displays the help on screen
	void gameOver(const double deltaTime); // Displays the game over text on screen
	void Text::setDisplayTime(const double miliSeconds); // Sets the display time of text on screen which is decremented over time until it becomes the first time negative
	bool Text::hasTimeLeft(); // Returns true if there is displayTime left, otherwise false
};