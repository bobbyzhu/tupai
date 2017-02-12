/*
* 	file : kmain.cpp
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
#include <tupai/kmain.hpp>

// --- MEMORY ---
#include <tupai/mempool.hpp>
#include <tupai/memory.hpp>

// --- MULTITASKING ---
#include <tupai/syscall.hpp>
#include <tupai/task.hpp>

// --- CONSOLE & I/O ---
#include <tupai/tty.hpp>
#include <tupai/console.hpp>
#include <tupai/prompt.hpp>

// --- DEBUGGING ---
#include <tupai/kdebug.hpp>

// --- EARLY ---
#include <tupai/startup.hpp>

#if defined(SYSTEM_ARCH_i686)
	// --- ESSENTIAL X86 THINGS ---
	#include <tupai/i686/gdt.hpp>
	#include <tupai/i686/idt.hpp>
	#include <tupai/i686/interrupt.hpp>
	#include <tupai/i686/pit.hpp>

	// --- X86 MEMORY ---
	#include <tupai/i686/paging.hpp>

	// --- X86 I/O AND DEVICES ---
	#include <tupai/i686/kbd.hpp>
	#include <tupai/i686/serial.hpp>
	#include <tupai/x86_family/vga.hpp>

	// --- X86 MULTIBOOT ---
	#include <tupai/x86_family/multiboot.hpp>
#endif

// --- TESTING ---

namespace tupai
{
	// Kernel early
	void kearly(ptr_t mb_header, uint32 mb_magic, uint32 stack)
	{
		#if defined(SYSTEM_ARCH_i686)
			// Multiboot
			x86_family::multiboot_init(mb_header, mb_magic);

			// Serial debug
			#if defined(CFG_ENABLE_SERIAL_DEBUG)
				serial_init();
				startup_print_unit_init("Serial");
				serial_open(1);
				startup_print_unit_init("COM1 debugging");
			#endif

			// Memory management unit
			memory_init();

			// Paging
			paging_init();
			startup_print_unit_init("Paging structure");
			paging_enable();
			startup_print_unit_init("Paging");

			// Dynamic memory pool
			mempool_init((ubyte*)mempool_begin, mempool_size, 1024); // Blocks of 1K
			startup_print_unit_init("Dynamic memory pool");

			// VGA
			x86_family::vga_init();
			startup_print_unit_init("VGA driver");

			// Console
			console_init_global();
			startup_print_unit_init("Kernel console");

			// TTY
			tty_init();
			startup_print_unit_init("Kernel TTY");

			// GDT
			gdt_init();
			startup_print_unit_init("GDT");

			// IDT
			idt_init();
			startup_print_unit_init("IDT");

			// PIT
			pit_init();
			startup_print_unit_init("PIT");

			// Keyboard
			kbd_init();
			startup_print_unit_init("Keyboard");

			// Serial
			serial_init();
			startup_print_unit_init("Serial");
		#endif

		syscall_init();
		startup_print_unit_init("Syscall routine");

		task_init();
		startup_print_unit_init("Task scheduler");

		tty_write_str("Boot procedure complete.\n");
	}

	// Kernel main
	void kmain()
	{
		interrupt_enable();
		pit_enable();
		kbd_enable();

		startup_welcome();

		while (prompt() == 0);
	}
}
