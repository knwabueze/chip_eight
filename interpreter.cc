#include "interpreter.hh"

using namespace Interpreter;

Chip8::Chip8()
	: PC(0), SP(0), delay_timer(0), sound_timer(0), I(0)
{
	memset(GFX, 0, WIDTH * HEIGHT); // Reset GFX
	memset(RAM, 0, 0x1000); // Reset RAM
	memset(Stack, 0, 0x10); // Reset Stack
	memset(V, 0, 0x10); // Reset general purpose registers
	memset(Keypad, 0, 0x10); // Reset keyboard

	// Load fonts into RAM
	static std::vector<unsigned char> fonts = {
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

	fonts.shrink_to_fit(); // throw away that rubbish mentality

	for (size_t i = 0; i < fonts.size(); ++i)
		RAM[i] = fonts[i];

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init SDL

	window = SDL_CreateWindow("Chip-8 Interpreter", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH * SCALE, HEIGHT * SCALE, 0);
	renderer = SDL_CreateRenderer(window, -1, 0);
}

Chip8::~Chip8()
{
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}

void Chip8::reset()
{
	PC = 0; SP = 0; delay_timer = 0; sound_timer = 0; I = 0; // Reset registers
	memset(GFX, 0, WIDTH * HEIGHT); // Reset GFX
	memset(RAM, 0, 0x1000); // Reset RAM
	memset(Stack, 0, 0x10); // Reset Stack
	memset(V, 0, 0x10); // Reset general purpose registers
	memset(Keypad, 0, 0x10); // Reset keyboard

	// Load fonts into RAM
	static std::vector<unsigned char> fonts = {
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

	fonts.shrink_to_fit(); // throw away that rubbish mentality

	for (size_t i = 0; i < fonts.size(); ++i)
		RAM[i] = fonts[i];
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
	unsigned char instr = (opcode & 0xF000);
	unsigned char nibble = (opcode & 0x000F);
	unsigned char byte = (opcode & 0x00FF);
	unsigned short addr = (opcode & 0x0FFF);
	VF = 0; // Reset flag register to 0
	
	std::printf("Current Execution Address >> 0x%X | Current Instruction >> 0x%04X\n", PC, opcode);

	switch (instr)
	{
	case 0x0000: // CLS or RET
		if (opcode == 0x00E0) // CLS
		{
			memset(GFX, 0, sizeof(GFX[0][0]) * WIDTH * HEIGHT);
			std::printf("CLS\n");
			redraw_flag = true;
		}
		else if (opcode == 0x00EE) // RET
		{
			PC = Stack[SP--];
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
		std::printf("LD Vx, 0x%02X\n", x, byte);
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
			if ((unsigned short)(V[x] + V[y]) > 255) VF = 1; // If integral overflow then set carry register

			V[x] += V[y] & 0xFF;
			PC += 2;
			std::printf("ADD V%i, V%i\n", x, y);
			break;
		}
		case 0x0005: // SUB Vx, Vy
		{
			if (V[x] > V[y]) VF = 1; // If Vx > Vy set NOT borrow register

			V[x] += V[y] & 0xFF;
			PC += 2;
			std::printf("ADD V%i, V%i\n", x, y);
			break;
		}
		}
		break;
	}
}

void Chip8::render()
{
}

void Chip8::keystates()
{
}
