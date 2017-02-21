/*
* 	file : prompt.cpp
*
* 	This file is part of Tupai.
*
* 	Tupai is free software: you can redistribute it and/or modify
* 	it under the terms of the GNU General Public License as published by
* 	the Free Software Foundation, either version 3 of the License, or
* 	(at your option) any later version.
*
* 	Tupai is distributed in the hope that it will be useful,
* 	but WITHOUT ANY WARRANTY; without even the implied warranty of
* 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* 	GNU General Public License for more details.
*
* 	You should have received a copy of the GNU General Public License
* 	along with Tupai.  If not, see <http://www.gnu.org/licenses/>.
*/

// Tupai
#include <tupai/prompt.hpp>
#include <tupai/tty.hpp>
#include <tupai/util/conv.hpp>

#include <tupai/prog/snake.hpp>
#include <tupai/prog/adventure.hpp>
#include <tupai/prog/sys.hpp>
#include <tupai/prog/timer.hpp>

// Libk
#include <libk/stdlib.hpp>
#include <libk/stdio.hpp>
#include <libk/string.hpp>

namespace tupai
{
	void readline(char* buffer, umem buffer_size)
	{
		umem buffer_pos = 0;
		while (true)
		{
			char input_char = libk::getchar();

			if (input_char != '\0')
			{
				if (input_char == '\n' || buffer_pos == buffer_size - 1) // Newline or end of buffer
				{
					buffer[buffer_pos] = '\0';
					return;
				}
				else if (input_char == '\b') // Backspace
				{
					if (buffer_pos > 0)
					{
						buffer_pos --;
						libk::putchar('\b');
					}
				}
				else
				{
					tty_write(input_char);

					buffer[buffer_pos] = input_char;
					buffer_pos ++;
				}
			}
		}
	}

	struct command_pair
	{
		int (*main_func)(int argc, char* argv[]);
		const char* cmd_name;

		command_pair(int (*main_func)(int argc, char* argv[]), const char* cmd_name)
		{
			this->main_func = main_func;
			this->cmd_name = cmd_name;
		}
	};

	int prompt()
	{
		command_pair commands[] =
		{
			command_pair(prog::sys_main, "sys"),
			command_pair(prog::snake_main, "snake"),
			command_pair(prog::adventure_main, "adventure"),
			command_pair(prog::timer_main, "timer"),
		};

		tty_write_str("Type 'help' for more information\n");

		while (true)
		{
			tty_set_fg_color(tty_color::CYAN);
			tty_write_str("kernel");
			tty_reset();
			tty_write('@');
			tty_set_fg_color(tty_color::RED);
			tty_write_str("tupai");
			tty_reset();
			tty_write_str("> ");

			const umem INPUT_BUFFER_SIZE = 1024;
			char buffer[INPUT_BUFFER_SIZE];
			readline(buffer, INPUT_BUFFER_SIZE);

			tty_write('\n');

			if (libk::strcmp(buffer, "help") == 0)
			{
				tty_write_str("--- Help ---\n");
				tty_write_str("help      - Show this help screen\n");
				tty_write_str("sys       - Show system information\n");
				tty_write_str("snake     - Play a demo snake game\n");
				tty_write_str("adventure - Play an adventure game\n");
				tty_write_str("timer     - Timer test program\n");
				tty_write_str("chars     - Display printable characters\n");
				tty_write_str("color     - Display printable colors\n");
				tty_write_str("exit      - Close the prompt session\n");
				tty_write_str("clear     - Clear the screen\n");
				tty_write_str("abort     - Abort the system\n");
			}
			else if (libk::strcmp(buffer, "exit") == 0)
			{
				return 0;
			}
			else if (libk::strcmp(buffer, "abort") == 0)
			{
				tty_write_str("Aborting...\n");
				libk::abort();
			}
			else if (libk::strcmp(buffer, "clear") == 0)
			{
				tty_clear();
			}
			else if (libk::strcmp(buffer, "chars") == 0)
			{
				for (int i = 32; i < 256; i ++)
				{
					tty_write(i);
					tty_write(' ');
					if ((i + 1) % 32 == 0)
						tty_write('\n');
				}
			}
			else if (libk::strcmp(buffer, "color") == 0)
			{
				const char* names[] = {"  Black  ", "   Red   ", "  Green  ", " Yellow  ", "  Blue   ", " Magenta ", "  Cyan   ", "  White  "};

				for (uint i = 0; i < (sizeof(names) / sizeof(char*)); i ++)
				{
					// Normal
					tty_set_bg_color(i);
					tty_set_fg_color(tty_color::WHITE);
					tty_write(' ');
					tty_write_str(util::compose((uint32)i, 16).val().raw());
					tty_write_str(names[i]);
					tty_reset();

					// Bright
					tty_set_bg_color(i + 8);
					tty_set_fg_color(tty_color::BLACK);
					tty_write(' ');
					tty_write_str(util::compose((uint32)i + 8, 16).val().raw());
					tty_write_str(names[i]);
					tty_reset();

					tty_write('\n');
				}
			}
			else
			{
				// Search for a command
				bool found = false;
				for (umem i = 0; i < sizeof(commands) / sizeof(command_pair); i ++)
				{
					if (libk::strcmp(commands[i].cmd_name, buffer) == 0)
					{
						commands[i].main_func(0, nullptr);
						found = true;
					}
				}

				// It's not been found
				if (!found && libk::strlen(buffer) > 0)
				{
					tty_write_str(buffer);
					tty_write_str(": command not found\n");
				}
			}
		}

		return 1;
	}
}
