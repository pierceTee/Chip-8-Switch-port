#ifndef CHIP_H
#define CHIP_H
#include <switch.h>
#include <stdint.h>
#include <cstdio>
#include <stdio.h>
#include <errno.h>

class chip
{
private:
	uint8_t memory[4096];	//4k memory allocation, this is where all of the program contents are loaded.

							/*
							System Memory Map
							0x000 -> 0x1FF: Chip-8 interpreter ( contains font set in emulator).
							0x050 -> 0x0A0: Used for the built in 4x5 pixel font set (0-F).
							0x200 -> 0xFFF: Program ROM and work RAM.
							*/


	uint16_t opcode;		//store current 2 byte opcode.
	uint8_t V[16];			//16 8-bit general purpose registers (V0->VE).

	uint16_t I;				//index register (0x000 -> 0xFFF).
	uint16_t pc;			//pc counter (0x000 -> 0xFFF).

	uint8_t delay_timer;	//When set above zero, will count down to zero.
	uint8_t sound_timer;	//When set above zero, will count down to zero and system buzzer will sound.

	uint16_t stack[16];		//Used to remember current location before a jump is performed. 
	uint16_t stackPointer;	//Used to remember which level of the stack is used. 

	void init();		    //initialize all variables to their correct starting point. 


public:
	chip();
	~chip();

	uint8_t  gfx[64 * 32];  // Stores current "on"  state for each pixel.
	uint8_t key[16];		//Used to store current state of input on the Chip-8's HEX based keypad (0x0 - 0xF)
	bool needsRedraw;		//Used to determine if the screen has changed and needs to be redrawn. 
	bool needSound;			//Used to signal whether a sound should be played.
	void emulateCycle();
	bool load(const char *file_path);
	int cycle_count = 0;
};



#endif //CHIP_8
