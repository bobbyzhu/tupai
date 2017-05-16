//
//	file : isr.s
//
// 	This file is part of Tupai.
//
//	Tupai is free software: you can redistribute it and/or modify
// 	it under the terms of the GNU General Public License as published by
// 	the Free Software Foundation, either version 3 of the License, or
// 	(at your option) any later version.
//
// 	Tupai is distributed in the hope that it will be useful,
// 	but WITHOUT ANY WARRANTY; without even the implied warranty of
// 	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// 	GNU General Public License for more details.
//
// 	You should have received a copy of the GNU General Public License
// 	along with Tupai.  If not, see <http://www.gnu.org/licenses/>.
//

// Globally define the ISRs
.global isr_pit
.global isr_syscall

.section .text

	.align 4
	isr_pit: // PIT ISR (irq 0)
		pushal
		cld

		push %esp // Pass the current stack pointer
		call pit_isr_main
		pop %esp // Restore the thread stack pointer

		popal
		iret

	.align 4
	isr_syscall: // SYSCALL ISR (irq 0x80)
		pushal
		cld

		push %esp // Pass the current stack pointer
		call syscall_isr_main
		pop %esp // Restore the thread stack pointer

		popal
		iret
