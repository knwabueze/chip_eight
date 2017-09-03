#include "interpreter.hh"

int main(int argc, char* argv[])
{
	Interpreter::Chip8 chip_eight;

	chip_eight.reset(); // Reset chip_eight registers and state

	chip_eight.load_rom("c:\\Users\\creat\\Documents\\chip_eight\\ROMs\\CONNECT4"); // Load CONNECT4 ROM, TODO: Load ROM mentioned in argv[1]

	for (;;) // while loop for intellectuals
	{
		// First emulate chip_eight cycle
		chip_eight.emulate_cycle();

		// Then renderer to the screen
		chip_eight.render();

		// Then check keystates
		chip_eight.keystates();

		// TODO: Calculate delta time stuff here (500hz target time)
		// TODO: Somehow decrement both sound and delay timers
	}
}