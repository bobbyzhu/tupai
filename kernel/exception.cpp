//
// file : exception.cpp
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

// Tupai
#include <tupai/exception.hpp>
#include <tupai/arch.hpp>
#include <tupai/sys/thread.hpp>
#include <tupai/sys/call.hpp>
#include <tupai/interrupt.hpp>
#include <tupai/panic.hpp>
#include <tupai/util/out.hpp>

#if defined(ARCH_FAMILY_x86)
	#include <tupai/x86/textmode.hpp>
#endif

namespace tupai
{
	const char* exceptions_msg[] =
	{
		"Division By Zero",            // 0
		"Debug",                       // 1
		"Non-Maskable Interrupt",      // 2
		"Breakpoint",                  // 3
		"Overflow",                    // 4
		"Out of Bounds",               // 5
		"Invalid Opcode",              // 6
		"No FPU Coprocessor",          // 7
		"Double Fault",                // 8
		"Coprocessor Segment Overrun", // 9
		"Bad TSS",                    // 10
		"Segment Not Present",        // 11
		"Stack Fault",                // 12
		"General Protection Fault",   // 13
		"Page Fault",                 // 14
		"Reserved Exception",         // 15
		"x87 Floating-Point",         // 16
		"Alignment Check",            // 17
		"Machine Check",              // 18
	};

	bool exception_panic[] =
	{
		false, // Division By Zero
		false, // Debug
		true,  // Non-Maskable Interrupt
		false, // Breakpoint
		true,  // Overflow
		false, // Out of Bounds
		false, // Invalid Opcode
		false, // No FPU Coprocessor
		true,  // Double Fault
		true,  // Coprocessor Segment Overrun
		true,  // Bad TSS
		false, // Segment not present
		false, // Stack fault
		false, // General Protection Fault
		false, // Page Fault
		true,  // Reserved Exception
		false, // x87 Floating-Point
		false, // Alignment Check
		true,  // Machine Check
	};

	size_t exception_handle(size_t stack, size_t code, size_t error)
	{
		sys::id_t thread_id = sys::thread_get_id();
		char thread_name[256];
		sys::thread_get_name(thread_id, thread_name);

		if (exception_panic[code])
			panic(exceptions_msg[code]);
		else
		{
			util::println("Exception in thread '", thread_name, "' (", thread_id, "): ", exceptions_msg[code], " [", util::fmt_int<size_t>(error, 16), ']');

			#if defined(DEBUG_ENABLED)
				arch_display_reg_state((arch_reg_state*)stack);
			#endif

			sys::thread_kill(thread_id);
		}

		if (sys::threading_enabled())
			stack = sys::thread_next_stack(stack);

		return stack;
	}
}
