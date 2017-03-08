//Source taken from www.dreamincode.net/forums/topic/185301-playing-audio-files-with-irrklang/
#pragma once

#include "irrKlang/irrKlang.h"
#include <string>

using namespace irrklang;
using namespace std;

/* All play functions STOP execution of the program except
* playSoundThreaded(). Note: It isn't really multi-threaded.*/
class SoundManager
{
private:
	ISoundEngine* soundEngine;
	ISound* currentSound;
	std::string fileName;
	ik_f32 volume;
	int position;

	const std::string SOUND_DIR = ".\\Sound\\";

	SoundManager(void){};// Private constructor to allow only one instance
	SoundManager(SoundManager const&);// Private constructor to prevent copies
	void operator=(SoundManager const&);// Private constructor to prevent assignments
public:
	/*Returns the pointer to the unique instance of this class.*/
	static SoundManager* SoundManager::getInstance()
	{
		static SoundManager instance;// lazy singleton, instantiated on first use
		return &instance;
	}
	~SoundManager();

	void init();
	void initFileName(string soundFile); // Doesn't play sound, only initializes fileName
	void playSound(); // Plays entire sound file
	void playSound(int milliseconds); // Plays for X number of milliseconds
	void playSound(string soundFile); // Play entire specified sound file
	void playSound(string soundFile, int milliseconds);
	void playSoundThreaded(); // Starts playing sound but program continues

	// Pause/Resume/Stop, works like any other media player
	void pause();
	void resume();
	void stopAll();

	// Set/get sound file to be played/playing
	void setFileName(string soundFile);
	string getFileName();

	// Volume controls
	void setVolume(ik_f32 newVolume);
	void increaseVolume();
	void increaseVolume(ik_f32 increment);
	void decreaseVolume();
	void decreaseVolume(ik_f32 increment);
	ik_f32 getCurrentVolume();
};


