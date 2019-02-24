
#if __INTELLISENSE__
typedef unsigned int __SIZE_TYPE__;
typedef unsigned long __PTRDIFF_TYPE__;
#define __attribute__(q)
#define __builtin_strcmp(a,b) 0
#define __builtin_strlen(a) 0
#define __builtin_memcpy(a,b) 0
#define __builtin_va_list void*
#define __builtin_va_start(a,b)
#define __extension__
#endif

#if defined(_MSC_VER)
#include <BaseTsd.h>
typedef SSIZE_T ssize_t;
#endif

#include "window.h"
#include "chip.h"


#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/errno.h>
#include <unistd.h>

//#include <SDL2/SDL_renderer.h>
#include <iostream>
#include <stdlib.h>
#include <dirent.h>


#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

int getInd(char* curFile, int curIndex) {
	DIR* dir;
	struct dirent* ent;

	if (curIndex < 0)
		curIndex = 0;

	dir = opendir("sdmc:/Roms/chip8");//Open current-working-directory.
	if (dir == NULL)
	{
		sprintf(curFile, "Failed to open dir!");
		return curIndex;
	}
	else
	{
		int i;
		for (i = 0; i <= curIndex; i++) {
			ent = readdir(dir);
		}
		if (ent)
			sprintf(curFile, "sdmc:/Roms/chip8/%s", ent->d_name);
		else
			curIndex--;
		closedir(dir);
	}

	return curIndex;
}

void getFile(char* curFile)
{
	//consoleInit(NULL);
	//gfxInitDefault();
	//consoleInit(NULL);

	printf("\x1b[16;10HSelect a file using the up and down keys.");
	printf("\x1b[17;10HPress start to run the rom.");

	sprintf(curFile, "Couldn't find any files in that folder!");
	int curIndex = 0;
	curIndex = getInd(curFile, curIndex);
	printf("\x1b[18;10H%s", curFile);

	while (true)
	{
		//Scan all the inputs. This should be done once for each frame
		hidScanInput();

		//hidKeysDown returns information about which buttons have been just pressed (and they weren't in the previous frame)
		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

		if (kDown & KEY_DOWN || kDown & KEY_DDOWN) {
			//consoleClear();
			printf("\x1b[16;10HSelect a file using the up and down keys.");
			printf("\x1b[17;10HPress start to run the rom.");
			curIndex++;
			curIndex = getInd(curFile, curIndex);
			printf("\x1b[18;10H%s", curFile);
		}

		if (kDown & KEY_UP || kDown & KEY_DUP) {
			//consoleClear();
			printf("\x1b[16;10HSelect a file using the up and down keys.");
			printf("\x1b[17;10HPress start to run the rom.");
			curIndex--;
			curIndex = getInd(curFile, curIndex);
			printf("\x1b[18;10H%s", curFile);
		}


		if (kDown & KEY_PLUS || kDown & KEY_A) {
			break;
		}
		//gfxFlushBuffers();
		//gfxSwapBuffers();
		//gfxWaitForVsync();
	}

	//consoleClear();
	//consoleExit(NULL);
	//gfxExit();
}


int main(int argc, char* argv[])
{
	socketInitializeDefault();
	nxlinkStdio();

	chip chip8;
	Window window("Chip-8 by pierceTee", 64, 32, 20);

	
	//char currentROM[255];
	//getFile(currentROM);


	int sleep_time = 400;


	//const char* finalROM = currentROM;


	//Create chip and window objects


	//hardcoded path to rom
	const char * currentROM = "romfs:/roms/BRIX.C8";



	//Enable File io
	Result rc = romfsInit();
	if (R_FAILED(rc))
		printf("romfsInit: %08X\n", rc);

	//Load the rom into memory. 
	if (!chip8.load(currentROM))
	{
		printf("Unable to load specified ROM file, please try again.\n");
		return -1;
	}


	// Main loop
	printf("In the game yo \n");
	while ( !window.closed && appletMainLoop())
	{
		//printf("In the loop yo \n");
		hidScanInput();

		u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

		if (kDown & KEY_MINUS) { chip8.load(currentROM); }
		window.pollEvents(chip8);

		//Run 9 cycles per frame, then render.
		for (int cycles = 0; cycles < 9; cycles++)
		{
			chip8.emulateCycle();
		}
		if (!window.isPaused)
		{
			chip8.emulateCycle();
			if (chip8.needsRedraw == true)
			{
				window.pixelCopy(chip8.gfx);
				window.draw();
				chip8.needsRedraw = false;
				for (int i = 0; i < sleep_time; i++)
				{
					//printf("sleeping untill %d\n", sleep_time);
				}
			}

			if (chip8.needSound)
			{
				window.playSound();
				chip8.needSound = false;
			}

			if (window.needsReset == true)
			{
				if (window.needsNewRom == true)
				{
					//TODO: FIX/REMOVE OPTION TO CHANGE ROM
				}
				else
				{
					if (!chip8.load(currentROM))
					{
						printf("Unable to load specified ROM file, please try again.\n");
						return -1;
					}
				}

				window.needsReset = false;
			}
		}
		//Sleep the thread to slow down emulation speed.
		//std::this_thread::sleep_for(std::chrono::microseconds(window.emulationSpeed));
	
	}
	//SDL_VideoQuit();
	//SDL_DestroyRenderer(renderer);
//	SDL_DestroyWindow(window);
	//SDL_Quit();
	// Deinitialize and clean up resources used by the console (important!)
	//consoleExit(NULL);
	return 0;
}