/*
* 	file : time.cpp
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

// Libk
#include <libk/time.hpp>
#include <tupai/i686/pit.hpp>

namespace libk
{
	/* Time functions */

	void sleep(useconds_t sec)
	{
		counter_t ctime = tupai::pit_counter;
		counter_t elapsed = sec * tupai::pit_rate;

		while (tupai::pit_counter - ctime < elapsed);
	}

	void usleep(useconds_t usec)
	{
		counter_t ctime = tupai::pit_counter;
		counter_t elapsed = (usec * tupai::pit_rate) / 1000;

		while (tupai::pit_counter - ctime < elapsed);
	}
}
