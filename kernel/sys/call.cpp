//
// file : call.cpp
//
// This file is part of Tupai.
//
// Tupai is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tupai is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY// without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tupai.  If not, see <http://www.gnu.org/licenses/>.
//

// Tupai
#include <tupai/sys/call.hpp>
#include <tupai/interrupt.hpp>
#include <tupai/util/out.hpp>

namespace tupai
{
	namespace sys
	{
		extern "C" void isr_syscall();
		extern "C" void* syscall_isr_main(void* stack_ptr);

		void call_init()
		{
			// Bind the interrupt
			interrupt_bind(CALL_IRQ, (void*)isr_syscall);
		}

		void* syscall_isr_main(void* stack_ptr)
		{
			util::println("Syscall occured!");

			return stack_ptr;
		}
	}
}
