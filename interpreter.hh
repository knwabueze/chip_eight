#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <SDL.h>
#include <cstdio>
#include <chrono>
#include <random>
#include <unordered_map>

namespace Interpreter
{
	const int WIDTH = 64, HEIGHT = 32, SCALE = 10; // WIDTH, HEIGHT, and SCALE constants

	// Map QWERTY Keyboard to hexadecimal keypad on Chip-8
	const std::unordered_map<int, int> keymap {
		{SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
		{SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
		{SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
		{SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF}
	};

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
		unsigned short Stack[0x10];

		// Graphics RAM, stores state of screen
		bool GFX[HEIGHT][WIDTH];

		// Keypad for input
		bool Keypad[0x10];

		// Flag for redrwaing
		bool redraw_flag;

		// Flag for pausing
		unsigned char pause_flag;

		// SDL Window and Rendererer objects (used for drawing)
		SDL_Window* window;
		SDL_Renderer* renderer;

		// SDL Event is used for keyboard input
		SDL_Event ev;

		Chip8();
		~Chip8();

		void load_rom(const std::string& path);
		void emulate_cycle();
		void emulate_hardware();
		void render();
		void keystates();
	};
}