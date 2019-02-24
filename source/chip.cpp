// Chip8.cpp : Defines the entry point for the console application.

#include "chip.h"
#include "window.h"
#include <iostream>
//#include <time.h>
//#include <random>
#include <algorithm>

#define FONT_SIZE 80

unsigned char chip8_fontset[FONT_SIZE] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};


chip::chip() {}
chip::~chip() {}

void chip::init()
{
	printf("chip 8 init BEGIN\n");
	I = 0x0;							//Initializes all variables to their default values.
	opcode = 0;
	pc = 0x200;
	delay_timer = 0;
	sound_timer = 0;
	stackPointer = 0;
	needsRedraw = false;
	needSound = false;

	std::fill_n(gfx, 64 * 32, 0);		//clears the display.
	std::fill_n(stack, 16, 0);			//clears the stack.
	std::fill_n(V, 16, 0);				//clears the registers V0 -> VF
	std::fill_n(memory, 4096, 0);		//clears the memory
	std::fill_n(key, 16, 0);			//clears the input register

	for (int i = 0; i < FONT_SIZE; i++)	//Initializes font set into correct memory locations. 
		memory[i] = chip8_fontset[i];
	printf("chip 8 init END\n");
}


bool chip::load(const char *file_path)
{
	printf("chip 8 load BEGIN\n");
	if (file_path == NULL) { return false; }
	//initialize chip(8)
	init();
	printf("Loading: %s\n" ,  file_path);

	//Open ROM file
	FILE* rom = fopen(file_path, "rb");
	if (rom == NULL)
	{
		printf("Could not open specified ROM\n");
		return false;
	}

	//Get File size
	fseek(rom, 0, SEEK_END);
	long rom_size = ftell(rom);
	rewind(rom);

	//Allocate memory to store ROM
	char* rom_buffer = (char*)malloc(sizeof(char) *rom_size);
	if (rom_buffer == NULL)
	{
		printf("Could not allocate memory for specified ROM file\n");
		return false;
	}

	//Copy ROM into buffer
	size_t result = fread(rom_buffer, sizeof(char), (size_t)rom_size, rom);
	if (result != (size_t)rom_size)
	{
		printf("Failed to read ROM\n");
		return false;
	}

	//Copy Buffer into memory
	if ((rom_size < 3584))							  //If the rom is smaller than allocated space in memory 4096 -512
	{
		for (int i = 0; i < rom_size; i++)
			memory[i + 512] = (uint8_t)rom_buffer[i]; //Load the rom buffer into memory starting at location 512 as specified by documentation.
	}
	else
	{
		printf("Specified ROM is too large to fit in memory\n");
		return false;
	}

	//clean up
	fclose(rom);
	free(rom_buffer);

	printf("chip 8 load END\n");
	return true;
}


void chip::emulateCycle()
{
	//Fetch Opcode
	opcode = memory[pc] << 8 | memory[pc + 1]; // combines 2 8-bit values from memory into one readable 16-bit opcode.
	cycle_count++;							   //Update current cycle count to keep track of timer updates

	//Decode Opcode
	switch (opcode & 0xF000)					// Clears all bits except first 8
	{
		//00EN
	case 0x0000:
		switch (opcode & 0x000F)
		{
			// 00E0: clears the screen
		case 0x0000:
			std::fill_n(gfx, 64 * 32, 0);		//clears the display.
			needsRedraw = true;
			pc += 2;							// progresses Program counter by 2 since all 16 bits have been read. 
			break;
			// 00EE: Returns from a subroutine
		case 0x000E:
			--stackPointer;
			pc = stack[stackPointer];
			pc += 2;
			break;

		default:
			printf("Error: Undefined OpCode -> %X\n", opcode);
			//exit(EXIT_FAILURE);
		}
		break;

		// 1NNN: Jumps to aaddress NNN.
	case 0x1000:
		pc = opcode & 0x0FFF;

		break;

		//2NNN: Calls subroutine at NNN
	case 0x2000:
		stack[stackPointer] = pc;
		stackPointer++;
		pc = opcode & 0x0FFF;
		break;

		//3XNN: Skips the next instruction if VX == NN
	case 0x3000:
		if (V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

		//4XNN: Skips the next instruction if VX != NN
	case 0x4000:
		if (V[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;
		//5XY0: Skips the  next instruction if VX == VY
	case 0x5000:
		if (V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

		//6XNN: Sets VX to NN.
	case 0x6000:
		V[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF);
		pc += 2;
		break;

		//7XNN: adds NN to VX.
	case 0x7000:
		V[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF);
		pc += 2;
		break;

		//8XYN
	case 0x8000:
		switch (opcode & 0x000F)
		{
			//8XY0: sets VX to the value of VY
		case 0x0000:
			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			//8XY1: sets VX to (VX or VY)
		case 0x0001:
			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;

			//8XY2: sets VX to (VX and VY)
		case 0x0002:
			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;

			//8XY3: sets VX to (VX XOR VY)
		case 0x0003:
			V[(opcode & 0x0F00) >> 8] = (V[(opcode & 0x0F00) >> 8] ^ V[(opcode & 0x00F0) >> 4]);
			pc += 2;
			break;

			//8XY4: adds VY to VX. VF is set to 1 when there's a carry and 0 when there isn't
		case 0x0004:
			V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4];

			// VX + VY is too large to be stored in a register
			if (V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode & 0x0F00) >> 8]))
				V[0xF] = 1;		 //A carry has taken place.	
			else
				V[0xF] = 0;

			pc += 2;
			break;

			//8XY5: VY is subracted from VX. VF is set to - when there's a borrow, and 1 when there isn't.
		case 0x0005:

			if (V[(opcode & 0x00F0) >> 4] > V[(opcode & 0x0F00) >> 8])
				V[0x0F] = 0;	//A borrow has taken place.
			else
				V[0xF] = 1;

			V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

			//8XY6: Stores the least significant bit of VX in VF and then shifts VX to the right by 1.
		case 0x0006:
			V[0x0F] = V[(opcode & 0x0F00) >> 8] & 0x1;
			V[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

			//8XY7: Sets VX to (VY - VX). VF is set to 0 when there's a borrow, and 1 when there isn't.
		case 0x0007:

			if (V[(opcode & 0x0F00) >> 8] > V[(opcode & 0x00F0) >> 4])
				V[0xF] = 0;		//A borrow has taken place.
			else
				V[0xF] = 1;

			V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			//8XYE: Stores the most significant bit of VX in VF and then shifts VX to the left by 1.
		case 0x000E:
			V[0x0F] = V[(opcode & 0x0F00) >> 8] >> 7;
			V[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			printf( "unknown opcode : %X\n", opcode);
			//exit(EXIT_FAILURE);
		}
		break;

		//9XY0 Skips the next instruction if VX != VY. 
	case 0x9000:
		if (V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;

		break;

		//ANNN: Sets I to the address NNN
	case 0xA000:
		I = (opcode & 0x0FFF);
		pc += 2;
		break;

		//BNNN: Jumps to the address NNN plus V0
	case 0xB000:
		pc = (opcode & 0x0FFF) + V[0];
		break;

		//CXNN: Sets VX to the result of a bitwise and operation on a random number(Typically: 0 to 255) and NN
	case 0xC000:
		V[(opcode & 0x0F00 >> 8)] = (rand() % 256) & (opcode & 0x00FF);
		pc += 2;
		break;

		/*DXYN
			Draws a sprite at coordinate (VX, VY) that has a width of 8 pixels and a height of N pixels.
			Each row of 8 pixels is read as bit-coded starting from memory location I;
			I value doesn’t change after the execution of this instruction.
			VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn,
			and to 0 if that doesn’t happen.
		*/
	case 0xD000:
	{
		unsigned short x = V[(opcode & 0x0F00) >> 8];
		unsigned short y = V[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		V[0xF] = 0;

		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						V[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
		}
		needsRedraw = true;
		pc += 2;
	}
	break;

	//EXNN
	case 0xE000:

		switch (opcode & 0x00FF)
		{
			//EX9E: Skips the next instruction if the key stores in VX is pressed.
		case 0x009E:
			if (key[V[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;

			break;

			//EXA1: Skips the next instruction if the key stored in VX isn't pressed.
		case 0x00A1:
			if (key[V[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;

			break;

		default:
			printf("Unknown opcode:%X\n ", opcode);
			//exit(EXIT_FAILURE);
		}
		break;

		//FXNN
	case 0xF000:

		switch (opcode & 0x00FF)
		{
			//FX07: Sets VX to the value of the delay timer.
		case 0x0007:
			V[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

			//FX0A: A key press is awaited, then stored in VX. 
			//Blocks Operation. All instruction halted until next key event.
		case 0x000A:
		{
			bool key_pressed = false;

			for (int i = 0; i < 16; i++)
				if (key[i] != 0)
				{
					V[(opcode & 0x0F00) >> 8] = i;
					key_pressed = true;
				}

			//If no key is pressed, retrun and try again.
			if (!key_pressed)
				return;

			pc += 2;
		}
		break;

		//FX15: Sets the delay timer to VX
		case 0x0015:
			delay_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			//FX18: Sets the sound timer to VX
		case 0x0018:
			sound_timer = V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			//FX1E: Adds VX to I
		case 0x001E:
			//VF is set to 1 when range overflow (I +VX > 0xFFF), andf 0 otherwise

			if (I + V[(opcode & 0x0F00) >> 8] > 0xFFF)
				V[0xF] = 1;
			else
				V[0xF] = 0;

			I += V[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

			//FX29: Sets I to the location of the sprite for the character in VX
			//Characters 0-F (hex) are represented by a 4x5 font.
		case 0x0029:
			I = V[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

			//FX33: Stores the binary-coded decimal representation of VX 
			//with the most significant of three digits at the address in I, 
			//the middle digit at I + 1
			//and the least signigicant digit at I + 2
		case 0x0033:
			memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (V[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

			//FX55: Stores V0 to VX (including VX) in memory 
			//starting at address I. The offset from I is increased
			//by 1 for each value written, but I itself is unmodified.
		case 0x0055:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = V[i];

			pc += 2;
			break;

			//FX65: Fills V0 to VX (including VX) with values from memory starting at address I. 
			//The offset from I is increased by 1 for each value written, but I itself is left unmodified.
		case 0x0065:
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				V[i] = memory[I + i];

			pc += 2;
			break;

		default:
			printf("Unknown opcode:%X\n ", opcode);

		}
		break;
	default:
		printf("Unknown opcode:%X\n ", opcode);
		//exit(EXIT_FAILURE);


	}
	//Update Timers every 8 emulation cycles.
	if (cycle_count % 8 == 0)
	{
		if (delay_timer > 0)
			--delay_timer;
		if (sound_timer > 0)
		{
			if (sound_timer == 1)
				needSound = true;

			--sound_timer;
		}

	}
}
