#include "interpreter.hh"

typedef Uint32 SDL_Time;

const int fps = 1000;
const float minframetime = 1000 / fps;

int main(int argc, char* argv[])
{
	Interpreter::Chip8 chip_eight;
	chip_eight.load_rom("c:\\Users\\creat\\Documents\\chip_eight\\ROMs\\VERS");

	// Delta time stuff
	SDL_Time start_time;
	SDL_Time last_tick = SDL_GetTicks();

	for (;;) // while loop for intellectuals
	{
		// Get current frame start time
		start_time = SDL_GetTicks();

		// Check keystates
		chip_eight.keystates();
		
		if (chip_eight.pause_flag != 2)
		{			
			// Emulate hardware (~62.5hz)
			if (SDL_GetTicks() - last_tick >= 16)
			{
				chip_eight.emulate_hardware();
				last_tick = SDL_GetTicks();
			}

			// Emulate chip cycle
			chip_eight.emulate_cycle();

			// Emulate render cycle
			if (chip_eight.redraw_flag)
				chip_eight.render();
		}		
			
		// Limit frame rate
		if (SDL_GetTicks() - start_time < minframetime)
			SDL_Delay(minframetime - (SDL_GetTicks() - start_time));
	}

	return 0;
}