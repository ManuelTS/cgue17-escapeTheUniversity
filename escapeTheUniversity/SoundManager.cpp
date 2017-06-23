#include "SoundManager.hpp"
#include <cstdlib>
#include <iostream>
#include <Windows.h>

using namespace std;

// Source taken from  www.dreamincode.net/forums/topic/185301-playing-audio-files-with-irrklang/
/*******************************
CONSTRUCTORS
*******************************/
void SoundManager::init()
{
	initFileName("\0");
}

void SoundManager::initFileName(string soundFile)
{
	soundEngine = createIrrKlangDevice();
	currentSound = 0;

	if (!soundEngine)
	{
		cout << "Error: Could not create sound engine" << endl;
		exit(0);
	}

	fileName = SOUND_DIR + soundFile;
	volume = 50;
	position = 0;
	soundEngine->setSoundVolume(1);
}

/*****************************
DESTRUCTOR
*****************************/
SoundManager::~SoundManager()
{
	if (currentSound) //should stop playing sounds
	{
		currentSound->stop();
		currentSound->drop();
		soundEngine->removeAllSoundSources();
	}
		soundEngine->stopAllSounds();
		soundEngine->drop();
}


/***************************
PLAY FUNCTIONS
***************************/
void SoundManager::playSound()
{
	if (fileName == "\0")
	{
		cerr << "Error: No sound file selected" << endl;
		return;
	}

	currentSound = soundEngine->play2D(fileName.c_str(), false, false, true);

	if (!currentSound)
	{
		cerr << "Error: Could not play file" << endl;
		exit(0);
	}

	position = currentSound->getPlayPosition();
}

void SoundManager::playSound(string soundFile)
{
	setFileName(soundFile);

	currentSound = soundEngine->play2D(fileName.c_str(), false, false, true);

	if (!currentSound)
	{
		cout << "Error: Could not play file" << endl;
		exit(0);
	}

	position = currentSound->getPlayPosition();
}

void SoundManager::playSoundThreaded()
{
	currentSound = soundEngine->play2D(fileName.c_str(), false, false, true);

	if (!currentSound)
	{
		cout << "Error: Could not play file" << endl;
		exit(0);
	}
}

/************************
PAUSE/RESUME/STOP
************************/
void SoundManager::pause()
{
	position = currentSound->getPlayPosition();
	soundEngine->setAllSoundsPaused();
}

void SoundManager::resume()
{
	currentSound = soundEngine->play2D(fileName.c_str(), false, false, true);

	if (position != -1)
		currentSound->setPlayPosition(position);
}

// Stops all sounds with a small fade
void SoundManager::stopAll()
{
	if(soundEngine)
		soundEngine->stopAllSounds();
}

/************************
GET/SET FILENAME
************************/
void SoundManager::setFileName(string soundFile)
{
	if (soundFile != "")
		fileName = SOUND_DIR + soundFile;
}

string SoundManager::getFileName()
{
	return fileName;
}


/***************************
VOLUME CONTROL
***************************/
void SoundManager::checkVolume(const float newVolume)
{
	if (newVolume < 0.0f)
		volume = 0.0f;
	else if (newVolume > 100.0f)
		volume = 100.0f;
	else
		volume = newVolume;
}

void SoundManager::setVolume(ik_f32 newVolume)
{
	checkVolume(newVolume);
	currentSound->setVolume(volume / 100.0f);
}

void SoundManager::increaseVolume()
{
	checkVolume(volume += 10.0f);
	currentSound->setVolume(volume / 100.0f);
}

void SoundManager::increaseVolume(ik_f32 increment)
{
	checkVolume(volume += increment);
	currentSound->setVolume(volume / 100.0f);
}

void SoundManager::decreaseVolume()
{
	checkVolume(volume -= 10.0f);
	currentSound->setVolume(volume / 100.0f);
}

void SoundManager::decreaseVolume(ik_f32 decrement)
{
	checkVolume(volume -= decrement);
	currentSound->setVolume(volume / 100.0f);
}

ik_f32 SoundManager::getCurrentVolume()
{
	return volume;
}