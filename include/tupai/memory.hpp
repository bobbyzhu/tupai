/*
* 	file : memory.hpp
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

#ifndef TUPAI_MEMORY_HPP
#define TUPAI_MEMORY_HPP

// Tupai
#include <tupai/type.hpp>

namespace tupai
{
	struct memory_info
	{
		umem used;
		umem size;

		memory_info(umem used, umem size)
		{
			this->used = used;
			this->size = size;
		}
	};

	void memory_init();
	void memory_enforce(); // On most systems, this means 'enable paging'

	memory_info memory_get_info();
}

#endif