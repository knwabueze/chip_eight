#include "interpreter.hh"

using namespace Interpreter;

using now = std::chrono::high_resolution_clock;

std::mt19937 rnd(now::now().time_since_epoch().count()); // seeded random generator

Chip8::Chip8()
	: PC(0),
	SP(-1),
	delay_timer(0),
	sound_timer(0),
	I(0),
	window(nullptr),
	renderer(nullptr),
	redraw_flag(false),
	pause_flag(0)
{
	memset(GFX, false, WIDTH * HEIGHT); // Reset GFX
	memset(RAM, 0, 0x1000); // Reset RAM
	memset(Stack, 0, 0x10); // Reset Stack
	memset(V, 0, 0x10); // Reset general purpose registers
	memset(Keypad, false, 0x10); // Reset keyboard

	// Load fonts into RAM
	const unsigned char fonts[80] = {
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
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80, // F
	};

	for (size_t i = 0; i < 80; ++i)
		RAM[i] = fonts[i];

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init SDL

	window = SDL_CreateWindow("Chip-8 Interpreter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * SCALE, HEIGHT * SCALE, 0);
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
}

Chip8::~Chip8()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void Chip8::load_rom(const std::string& path)
{
	std::ifstream file;
	unsigned file_size;

	file.open(path, std::ios::binary);

	if (file.is_open())
	{
		file.seekg(0, std::ios::end);
		file_size = static_cast<unsigned>(file.tellg());

		if (file_size <= 0x1000 - 0x200) // file size is correct size
		{
			char* prg_start = reinterpret_cast<char *>(&(RAM[0x200])); // Start at address 0x200

			file.seekg(0, std::ios::beg);
			file.read(prg_start, file_size);

			PC = 0x200;
		}

		file.close();
	}
}

void Chip8::emulate_cycle()
{
	unsigned short opcode = (RAM[PC] << 8) | RAM[PC + 1];
	unsigned char x = (opcode & 0x0F00) >> 8;
	unsigned char y = (opcode & 0x00F0) >> 4;
	unsigned short instr = (opcode & 0xF000);
	unsigned char nibble = (opcode & 0x000F);
	unsigned char byte = (opcode & 0x00FF);
	unsigned short addr = (opcode & 0x0FFF);

	std::printf("Current Execution Address >> 0x%X | Current Instruction >> 0x%04X | ", PC, opcode);

	switch (instr)
	{
	case 0x0000: // CLS or RET
		if (opcode == 0x00E0) // CLS
		{
			memset(GFX, 0, sizeof(GFX[0][0]) * WIDTH * HEIGHT);
			std::printf("CLS\n");
			redraw_flag = true;
			PC += 2;
		}
		else if (opcode == 0x00EE) // RET
		{
			PC = Stack[SP--];
			PC += 2;
			std::printf("RET\n");
		}
		break;
	case 0x1000: // JP addr
		PC = addr;
		std::printf("JP 0x%03X\n", addr);
		break;
	case 0x2000: // CALL addr
		Stack[++SP] = PC;
		PC = addr;
		std::printf("CALL 0x%03x\n", addr);
		break;
	case 0x3000: // SE Vx, byte
		if (V[x] == byte)
			PC += 2;
		PC += 2;
		std::printf("SE V%i, 0x%02X\n", x, byte);
		break;
	case 0x4000: // SNE Vx, byte
		if (V[x] != byte)
			PC += 2;
		PC += 2;
		std::printf("SNE V%i, 0x%02X\n", x, byte);
		break;
	case 0x5000: // SE Vx, Vy
		if (V[x] == V[y])
			PC += 2;
		PC += 2;
		std::printf("SE V%i, V%i\n", x, y);
		break;
	case 0x6000: // LD Vx, byte
		V[x] = byte;
		PC += 2;
		std::printf("LD V%i, 0x%02X\n", x, byte);
		break;
	case 0x7000: // ADD Vx, byte
		V[x] += byte;
		PC += 2;
		std::printf("ADD V%i, 0x%02X\n", x, byte);
		break;
	case 0x8000: // Arithmetic
		switch (nibble)
		{
		case 0x0000: // LD Vx, Vy
		{
			V[x] = V[y];
			PC += 2;
			std::printf("LD V%i, V%i\n", x, y);
			break;
		}
		case 0x0001: // OR Vx, Vy
		{
			V[x] |= V[y];
			PC += 2;
			std::printf("OR V%i, V%i\n", x, y);
			break;
		}
		case 0x0002: // AND Vx, Vy
		{
			V[x] &= V[y];
			PC += 2;
			std::printf("AND V%i, V%i\n", x, y);
			break;
		}
		case 0x0003: // XOR Vx, Vy
		{
			V[x] ^= V[y];
			PC += 2;
			std::printf("XOR V%i, V%i\n", x, y);
			break;
		}
		case 0x0004: // ADD Vx, Vy
		{
			VF = 0;
			if ((unsigned short)(V[x] + V[y]) > 255) VF = 1; // If integral overflow then set carry register

			V[x] += V[y] & 0xFF;
			PC += 2;
			std::printf("ADD V%i, V%i\n", x, y);
			break;
		}
		case 0x0005: // SUB Vx, Vy
		{
			VF = 0;
			if (V[x] > V[y]) VF = 1; // If Vx > Vy set NOT borrow register

			V[x] -= V[y];
			PC += 2;
			std::printf("SUB V%i, V%i\n", x, y);
			break;
		}
		case 0x0006: // SHR Vx {, Vy}
		{
			VF = 0;
			if ((V[x] & 1) == 1) VF = 1; // If least-significant bit of Vx is 1, then VF = 1
			V[x] /= 2; // Then Vx is divided by 2

			PC += 2;
			std::printf("SHR V%i {, V%i}\n", x, y);
			break;
		}
		case 0x0007: // SUBN Vx, Vy
		{
			VF = 0;
			if (V[y] > V[x]) VF = 1; // If Vy > Vx set NOT borrow register

			V[x] = V[y] - V[x];
			PC += 2;
			std::printf("SUBN V%i, V%i\n", x, y);
			break;
		}
		case 0x0008: // SHL Vx {, Vy}
		{
			VF = 0;
			if ((V[x] & 1 == 1)) VF = 1; // If least-significant bit of Vx is 1, then VF = 1
			V[x] *= 2; // Then Vx is divided by 2

			PC += 2;
			std::printf("SHL V%i {, V%i}\n", x, y);
			break;
		}
		}
		break;
	case 0x9000: // SNE Vx, Vy
		if (V[x] != V[y])
			PC += 2;
		PC += 2;
		std::printf("SNE V%i, V%i\n", x, y);
		break;
	case 0xA000: // LD I, addr
		I = addr;
		PC += 2;
		std::printf("LD I, 0x%03X\n", addr);
		break;
	case 0xB000: // JP V0, addr
		PC = V[0] + addr;
		std::printf("JP V0, 0x%03X\n", addr);
		break;
	case 0xC000: // RND Vx, byte
	{
		unsigned char random = std::uniform_int_distribution<>(0, 255)(rnd);

		V[x] = random & byte;
		PC += 2;
		std::printf("RND V%i, 0x%02X\n", x, byte);
		break;
	}
	case 0xD000: // DRW Vx, Vy, nibble
	{
		VF = 0;
		unsigned char& VX = V[x], VY = V[y]; // Set references to Vx and Vy

		for (int yline = 0; yline < nibble; ++yline)
		{
			unsigned char sprite = RAM[(I + yline) & 0xFFF];

			for (int xline = 0; xline < 8; ++xline)
			{
				if ((sprite & (0x80 >> xline)) != 0)
				{
					auto calc_y = (VY + yline) % HEIGHT; // Start at Vy then move according to y-axis (% HEIGHT to wrap around)
					auto calc_x = (VX + xline) % WIDTH; // Start at Vx then move according to x-axis (% WIDTH to wrap around)

					if (GFX[calc_y][calc_x] == 1)
						VF = 1; // Set collision flag if there is a written bit at y * x

					GFX[calc_y][calc_x] ^= 1;
				}
			}
		}

		redraw_flag = true;
		PC += 2;
		std::printf("DRW V%i, V%i, 0x%01X\n", x, y, nibble);
		break;
	}
	case 0xE000: // Skips
		switch (byte)
		{
		case 0x009E: // SKP Vx
			if (Keypad[V[x]])
				PC += 2;
			PC += 2;
			std::printf("SKP V%i\n", x);
			break;
		case 0x0A1: // SKNP Vx
			if (!Keypad[V[x]])
				PC += 2;
			PC += 2;
			std::printf("SKNP V%i\n", x);
			break;
		}
		break;
	case 0xF000: // Loads
		switch (byte)
		{
		case 0x0007: // LD Vx, DT
		{
			V[x] = delay_timer;
			std::printf("LD V%i, DT\n", x);
			PC += 2;
			break;
		}
		case 0x000A: // LD Vx, K
		{
			bool gotKeyEvent = false;

			while (!gotKeyEvent)
			{
				SDL_WaitEvent(&ev);

				if (ev.type == SDL_KEYDOWN)
				{
					gotKeyEvent = true;

					auto sym = keymap.find(ev.key.keysym.sym);

					if (ev.key.keysym.sym == SDLK_ESCAPE)
						exit(1);
					else if (sym == keymap.end())
						gotKeyEvent = false;
					else
						V[x] = sym->second;
				}
				else if (ev.type == SDL_QUIT)
					exit(1);
			}

			std::printf("LD V%i, K\n", x);
			PC += 2;
			break;
		}
		case 0x0015: // LD DT, Vx
		{
			delay_timer = V[x];
			std::printf("LD DT, V%i\n", x);
			PC += 2;
			break;
		}
		case 0x0018: // LD ST, Vx
		{
			sound_timer = V[x];
			std::printf("LD ST, V%i\n", x);
			PC += 2;
			break;
		}
		case 0x001E: // ADD I, Vx
		{
			VF = 0;
			if (I + V[x] > 255)
				VF = 1;

			I += V[x] & 0xF;
			std::printf("ADD I, V%i\n", x);
			PC += 2;
			break;
		}
		case 0x0029: // LD F, Vx
		{
			I = V[x] * 5;
			std::printf("LD F, V%i\n", x);
			PC += 2;
			break;
		}
		case 0x0033: // LD B, Vx
		{
			unsigned char VX = V[x];

			RAM[I + 2] = VX % 10;
			RAM[I + 1] = VX / 10 % 10;
			RAM[I] = VX / 100 % 10;

			std::printf("LD B, V%i\n", x);
			PC += 2;
			break;
		}
		case 0x0055: // LD [I], Vx
		{
			for (unsigned char i = 0; i < x; ++i)
				RAM[I + i] = V[i];
			PC += 2;
			std::printf("LD [I], V%i\n", x);
			break;
		}
		case 0x0065: // LD Vx, [I]
		{
			for (unsigned char i = 0; i < x; ++i)
				V[i] = RAM[I + i];
			PC += 2;
			std::printf("LD V%i, [I]\n", x);
			break;
		}
		}
		break;
	}
}

void Chip8::emulate_hardware()
{
	if (delay_timer > 0)
		--delay_timer;
	if (sound_timer > 0)
		--sound_timer;
}

void Chip8::render()
{
	SDL_Rect pxl = { 0, 0, 10, 10 };

	for (unsigned char y = 0; y < HEIGHT; ++y)
	{
		for (unsigned char x = 0; x < WIDTH; ++x)
		{
			auto pixel = GFX[y][x];
			int val = pixel == 1 ? 255 : 0;

			pxl.y = y * SCALE;
			pxl.x = x * SCALE;

			SDL_SetRenderDrawColor(renderer, val, val, val, 1);
			SDL_RenderFillRect(renderer, &pxl);
		}
	}

	SDL_RenderPresent(renderer);
	SDL_RenderClear(renderer);
	redraw_flag = false;
}

void Chip8::keystates()
{
	SDL_PollEvent(&ev);

	switch (ev.type)
	{
	case SDL_QUIT:
	{
		SDL_Quit();
		exit(1);
		break;
	}
	case SDL_KEYDOWN:
	{
		if (ev.key.keysym.sym == SDLK_ESCAPE)
		{
			SDL_Quit();
			exit(1);
		}

		else if (ev.key.keysym.sym == SDLK_SPACE && pause_flag == 0)
			pause_flag = 1;
		else if (ev.key.keysym.sym == SDLK_SPACE && pause_flag == 2)
			pause_flag = 0;

		auto sym = keymap.find(ev.key.keysym.sym);
		if (sym != keymap.end())
			Keypad[sym->second] = true;
		break;
	}
	case SDL_KEYUP:
	{
		auto sym = keymap.find(ev.key.keysym.sym);

		if (ev.key.keysym.sym == SDLK_SPACE && pause_flag == 1)
			pause_flag = 2;

		if (sym != keymap.end())
			Keypad[sym->second] = false;
		break;
	}
	}
}
