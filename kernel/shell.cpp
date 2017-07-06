//
// file : shell.cpp
//
// This file is part of Tupai.
//
// Tupai is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tupai is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tupai.  If not, see <http://www.gnu.org/licenses/>.
//

// Tuapi
#include <tupai/shell.hpp>
#include <tupai/tupai.hpp>
#include <tupai/arch.hpp>
#include <tupai/panic.hpp>

#include <tupai/dev/clock.hpp>

#include <tupai/sys/kmem.hpp>
#include <tupai/sys/thread.hpp>
#include <tupai/sys/mmap.hpp>
#include <tupai/sys/call.hpp>

#include <tupai/util/in.hpp>
#include <tupai/util/out.hpp>
#include <tupai/util/str.hpp>
#include <tupai/util/vector.hpp>

#include <tupai/fs/vfs.hpp>

namespace tupai
{
	void shell_motd()
	{
		util::println(tupai_get_name_decorative(), " ",
			tupai_get_version(), " on ",
			arch_get_family(), "/", arch_get_target(), '\n',
			"Copyright 2017, ", P_MAINTAINER_NAME);
	}

	void shell_main(int argc, char* argv[])
	{
		(void)argc;
		(void)argv;

		// Display information
		shell_motd();
		util::println("Type 'help' for more info.");

		bool halted = false;
		while (!halted)
		{
			util::print("\n$ ");

			char buff[64];
			util::readline(buff);
			util::print('\n');

			if (util::str_equal(buff, "help"))
			{
				util::print(
					"Available commands:\n",
					"  help    -> Show this help text\n",
					"  threads -> Show running threads\n",
					"  fs      -> Show filesystem tree\n",
					"  pool    -> Show kernel memory pool\n",
					"  motd    -> Show the MOTD\n",
					"  div     -> Trigger a divided-by-zero exception\n",
					"  panic   -> Trigger a kernel panic\n",
					"  info    -> Show system info\n",
					"  time    -> Show the system time\n"
				);
			}
			else if (util::str_equal(buff, "threads"))
			{
				size_t n = sys::threads_count();
				for (size_t i = 0; i < n; i ++)
				{
					char name[64];
					sys::id_t id = sys::thread_get_id(i);
					sys::thread_get_name(id, name);
					util::println(i, ' ', id, ' ', name);
				}
			}
			else if (util::str_equal(buff, "fs"))
			{
				fs::vfs_display();
			}
			else if (util::str_equal(buff, "file"))
			{
				id_t id;
				const char* path = "/conf/init.cfg";
				sys::call(sys::CALL::OPEN, (size_t)&id, (size_t)path);
				util::println("FD id is ", id);
			}
			else if (util::str_equal(buff, "pool"))
			{
				sys::kmem_display();
			}
			else if (util::str_equal(buff, "motd"))
			{
				shell_motd();
			}
			else if (util::str_equal(buff, "div"))
			{
				int volatile a = 5 / 0; //
			}
			else if (util::str_equal(buff, "panic"))
			{
				panic("Panic triggered artificially");
			}
			else if (util::str_equal(buff, "info"))
			{
				util::print(
					"System Info:\n",
					"  address_size -> ", sizeof(void*) * 8, " bits\n",
					"  kernel_start -> ", (void*)arch_get_kernel_start(), '\n',
					"  kernel_end   -> ", (void*)arch_get_kernel_end(), '\n'
				);
				sys::kmem_info();
				sys::mmap_display();
			}
			else if (util::str_equal(buff, "time"))
			{
				datetime_t time = dev::clock_read();

				util::println(
					"The system time is ",
					util::fmt_int<short        >(time.year, 10, 4), '-', util::fmt_int<unsigned char>(time.month, 10, 2), '-', util::fmt_int<unsigned char>(time.day, 10, 2), ' ',
					util::fmt_int<unsigned char>(time.hour, 10, 2), ':', util::fmt_int<unsigned char>(time.min  , 10, 2), ':', util::fmt_int<unsigned char>(time.sec, 10, 2)
				);
			}
			else if (util::str_len(buff) > 0)
				util::println("Command '", buff, "' not found!");
		}
	}
}
