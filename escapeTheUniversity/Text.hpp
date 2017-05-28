#pragma once
#include <GLM\glm.hpp>
#include <string>
#include <map>
#include <vector>

class Shader;
class Camera;

/*Renders text to the screen with signed distance field character map. This class has the singelton design pattern implemented.*/
class Text{
public:
	~Text();

	const unsigned char FIRST_CHARACTER = ' '; //First supported extended ASCII character
	const unsigned char LAST_CHARACTER = 'ÿ'; //Last supported extended ASCII character
	static const int GAME_OVER = 4000; // Unique constant time in milliseconds to display text on screen, added to displayTime vector if used
	static const int SCREENY = 4001; // Unique constant time in milliseconds to display text on screen, added to displayTime vector if used

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
	void addText2Display(const int toAdd); // Adds a unique constant time in milliseconds to display it on screen
	bool hasTimeLeft(const unsigned int i = 0, const double deltaTime = 0); // Returns true if there is displayTime left, otherwise false when no arguments are specified. If the two arguments are specified the deltaTime on index i + 1 in the displayTime array is subtracted and the index is deleted if zero is undershot
	void removeTime(const double deltaTime); // Checks if in the displayTime array is still some text left to display and removes the entries if necessary
	void wireframe(); // Displays wireframe text on screen
	void showCamCoords(Camera* camera); // Displays the camera coords on the screen
	void drawBulletDebug(); // Displays the showing debug context text on screen
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
	// Buffers
	char fpsBuffer[30]; // FPS character buffer
	const glm::vec3 DEFAULT_COLOR = glm::vec3(1.0f, 1.0f, 1.0f); // Default color of text
	glm::vec3 color = DEFAULT_COLOR; // Color of text to be drawn next
	// Display timed text
	double timeThreashold = 0; // Is a threshold that the fps are updated only each second when rendered on screen
	std::vector<int> displayTime; // Contains the text display on screen time constant and its remaining time on screen

	Shader* textShader;
	unsigned int characterTextureHandle = 0;

	Text(void) {}; // Private constructor to allow only one instance
	Text(Text const&); // Private constructor to prevent copies
	void operator=(Text const&); // Private constructor to prevent assignments

	unsigned int copyInBuffer(char buffer[], unsigned int i, const unsigned char* toCopy, const bool linebreak);// Copies a char* into an char []
	void writeVertices(std::vector<float>* vertices); // Writes the actual caracters denoted by the vector onto the screen
	void gameOver(); // Displays the game over text on screen for some seconds
	void screeny(); // Displays the screen shoot screen for some seconds
};