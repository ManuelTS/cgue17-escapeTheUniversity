#include "Initialization.hpp"
#include "Debug/Debugger.hpp"
#include <sstream>
#include <fstream>

Initialization::Initialization(){
	readIni();
}

Initialization::~Initialization(){
	delete [] windowTitle;
}

/*Writes the content from ref into value and clears the stream afterwards. Prints an error message if the cast goes wrong.*/
template<typename T> void writeMember(stringstream* ref, T* value, T standard){

	if (typeid(T) == typeid(int) || typeid(T) == typeid(float) || typeid(T) == typeid(double)){
		if (!(*ref >> *value)) // Width
		{ // Failed, set to normal and error message
			*value = standard; 
			Debugger::getInstance()->pauseExit("Convertation of " + ref->str() +" failed, width is set to standard value.");
		}
	}
	else if (typeid(T) == typeid(bool)){
		*ref >> boolalpha >> *value;
	}

	ref->clear();
}


/* Reads the .ini file of this project, sets and casts its values in this class for further processing.*/
void Initialization::readIni(){
	ifstream infile(inifile);
	stringstream iniStream;
	string line;
	string key;
	string value;
	int equalIndex = string::npos;

	if (infile.is_open())
	{
		while (getline(infile, line)) // Go through file line by line
		{
			if ((equalIndex = line.find_first_of("=", 0)) != string::npos) // Cut at "="
			{
				key = line.substr(0, equalIndex);
				value = line.substr(++equalIndex, line.size() - equalIndex);
				// Read out the single keys
				if (key.compare("resolution") == 0){ // Resolution in 800x600 format 
					equalIndex = value.find_first_of("x");

					iniStream.str(value.substr(0, equalIndex));// Width
					writeMember<int>(&iniStream, &width, 800);

					iniStream.str(value.substr(++equalIndex, line.size() - equalIndex));// Height
					writeMember<int>(&iniStream, &height, 600);
				}
				else if (key.compare("maxResolution") == 0){ // Resolution in 800x600 format 
					equalIndex = value.find_first_of("x");

					iniStream.str(value.substr(0, equalIndex));// Width
					writeMember<int>(&iniStream, &maxWidth, 1920);

					iniStream.str(value.substr(++equalIndex, line.size() - equalIndex));// Height
					writeMember<int>(&iniStream, &maxHeight, 1080);
				}
				else if (key.compare("fps") == 0){
					iniStream.str(value);
					writeMember<int>(&iniStream, &fps, 60); // FPS
				}
				else if (key.compare("fullscreen") == 0){
					iniStream.str(value);
					writeMember<bool>(&iniStream, &fullscreen, false); // fullscreen
				}
				else if (key.compare("windowTitle") == 0)
				{
					windowTitle = new char[value.length() + 1];
					strcpy(windowTitle, value.c_str());
				}
				else if (key.compare("mouseSensitivity") == 0){
					iniStream.str(value);
					writeMember<double>(&iniStream, &mouseSensitivity, 0.3); // mouseSensitivity
				}
				else if (key.compare("movingSpeed") == 0){
					iniStream.str(value);
					writeMember<double>(&iniStream, &movingSpeed, 3.0); // movingSpeed
				}
				else if (key.compare("scrollSpeed") == 0){
					iniStream.str(value);
					writeMember<double>(&iniStream, &scrollSpeed, 0.1); // scrollSpeed
				}
				else if (key.compare("zoom") == 0){
					iniStream.str(value);
					writeMember<double>(&iniStream, &zoom, 45.0); // zoom
				}
				else
					Debugger::getInstance()->pauseExit("Unkown key " + key + " in ini " + inifile + " file.");
			}

			equalIndex = string::npos;
		}
		infile.close();
	}
	else
		Debugger::getInstance()->pauseExit("Could not open ini " +inifile +" file.");
}
