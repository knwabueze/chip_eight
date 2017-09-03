#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <SDL.h>
#include <cstdio>

namespace Interpreter
{
	const int WIDTH = 64, HEIGHT = 32, SCALE = 10; // WIDTH, HEIGHT, and SCALE constants

	struct Chip8
	{
		// RAM Initialization
		unsigned char RAM[0x1000];

		// General-purpose 8-bit registers initialization
		unsigned char V[0x10];
		unsigned char& VF = V[0xF];

		// 16-bit I register declaration
		unsigned short I;

		// Special-purpose 8-bit registers
		unsigned char sound_timer, delay_timer;

		// Psuedo-registers (cannot be directly manipulated)
		unsigned short PC;
		unsigned char SP;

		// Stack (16 16-bit)
		unsigned char Stack[0x10];

		// Graphics RAM, stores state of screen
		unsigned short GFX[HEIGHT][WIDTH];

		// Keypad for input
		unsigned short Keypad[0x10];

		// Flag for redrwaing
		bool redraw_flag;

		// SDL Window and Rendererer objects (used for drawing)
		SDL_Window *window;
		SDL_Renderer *renderer;

		Chip8();
		~Chip8();

		void reset();
		void load_rom(const std::string& path);
		void emulate_cycle();		
		void render();
		void keystates();
	};
}