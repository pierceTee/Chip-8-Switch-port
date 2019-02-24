#pragma once
//#include <Retro_Computer_ttf.h>
#include <iostream>
#include <string>
//#include <sstream>
#include <switch.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h> 
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_events.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>
#include "chip.h"
#undef main


class Window
{
public:
	Window(const char *title, int width, int height, int scale);
	~Window();

	void pollEvents(chip &chip8);
	void draw();
	void pixelCopy(uint8_t *gfx);

	bool isClosed() { return closed; }
	bool closed;
	void playSound();
	int emulationSpeed;				//How long to sleep the thread after each emulation cycle in ms.
	void bindKeys();
	bool isPaused = false;
	bool needsReset = false;
	bool needsNewRom = false;
	void displayPause();
private:
	bool init();

private:
	const char *title;
	int width;
	int height;
	int scale;
	
	const char *speedString = nullptr;		//String used to print out current emulation speed.
	int drawCount = 0;
	double emulationRatio = 1;
	uint32_t pixel_buffer[64 * 32] = { 0 };	//Initialize buffer to black for black background
	Uint32 primaryColor = 0x0022E8A7;



public:
	SDL_Window *window = nullptr;

	SDL_Renderer *renderer = nullptr;

	SDL_Texture *texture = nullptr;
	SDL_Texture *menuTexture = nullptr;
	SDL_Texture *speedTexture = nullptr;

	SDL_Surface *textSurface = nullptr;

	//TTF_Font *font = nullptr;
	SDL_Color white = { 255, 255, 255, 255 };

	SDL_Rect menuRect;
	SDL_Rect speedRect;

	SDL_AudioSpec wavSpec;
	SDL_AudioSpec desired;
	SDL_AudioDeviceID deviceId;
	SDL_Event event;

	Uint32 wavLength;
	Uint8 *wavBuffer;
	uint8_t keymap[16] = {
		SDLK_x, //0
		SDLK_1, //1
		SDLK_2, //2
		SDLK_3, //3
		SDLK_q, //4
		SDLK_w, //5 
		SDLK_e, //6
		SDLK_a, //7
		SDLK_s, //8
		SDLK_d, //9
		SDLK_z, //10
		SDLK_c, //11
		SDLK_4, //12
		SDLK_r, //13
		SDLK_f, //14
		SDLK_v, //15
	};
};
#pragma once