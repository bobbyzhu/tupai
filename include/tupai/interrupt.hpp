//
// file : interrupt.hpp
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

#ifndef TUPAI_INTERRUPT_HPP
#define TUPAI_INTERRUPT_HPP

// Standard
#include <stddef.h>
#include <stdint.h>

namespace tupai
{
	void interrupt_enable(bool enable = true);
	bool interrupt_enabled();
	void interrupt_bind(uint8_t irq, void* address, bool hardware = false);
	void interrupt_ack(uint8_t irq);
	void interrupt_mask(uint8_t irq, bool enable);
}

#endif
