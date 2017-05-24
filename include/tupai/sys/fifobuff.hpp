//
// file : fifobuff.hpp
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

#ifndef TUPAI_SYS_FIFOBUFF_HPP
#define TUPAI_SYS_FIFOBUFF_HPP

// Tupai
#include <tupai/util/mutex.hpp>

// Standard
#include <stddef.h>
#include <stdint.h>

namespace tupai
{
	namespace sys
	{
		template <size_t SIZE>
		struct fifobuff_t;

		template <size_t SIZE> void    __fifo_write(volatile fifobuff_t<SIZE>& buff, uint8_t c);
		template <size_t SIZE> uint8_t __fifo_read (volatile fifobuff_t<SIZE>& buff);
		template <size_t SIZE> size_t  __fifo_len  (volatile fifobuff_t<SIZE>& buff);

		template <size_t SIZE>
		struct fifobuff_t
		{
		public:
			volatile uint8_t arr[SIZE] = { 0, };
			volatile size_t  head   = 0;
			volatile size_t  tail   = 0;
			volatile size_t  length = 0;

			volatile util::mutex mutex;

		public:
			void    write(uint8_t c) volatile { return __fifo_write(*this, c); }
			uint8_t read()           volatile { return __fifo_read(*this); }
			size_t  len()            volatile { return __fifo_len(*this); }
		};

		template <size_t SIZE>
		void __fifo_write(volatile fifobuff_t<SIZE>& buff, uint8_t c)
		{
			buff.mutex.lock(); // Begin critical section

			if (buff.length != 0)
			{
					buff.head = (buff.head + 1) % SIZE;
					if (buff.head == buff.tail)
						buff.tail = (buff.tail + 1) % SIZE;
			}

			buff.arr[buff.head] = c;

			if (buff.length < SIZE)
				buff.length ++;

			buff.mutex.unlock(); // End critical section
		}

		template <size_t SIZE>
		uint8_t __fifo_read (volatile fifobuff_t<SIZE>& buff)
		{
			// Wait loop
			while (true)
			{
				// Wait until data appears
				while (__fifo_len(buff) <= 0);

				// Lock the buffer while we check
				buff.mutex.lock(); // Begin critical section

				// Double-check to make sure there's STILL data
				if (buff.length > 0)
					break; // Exit to start reading data
				else
					buff.mutex.unlock(); // Somebody jumped the queue. Damn!
			}

			buff.length --;

			uint8_t val = buff.arr[buff.tail];

			if (buff.length != 0)
					buff.tail = (buff.tail + 1) % SIZE;

			buff.mutex.unlock(); // End critical section

			return val;
		}

		template <size_t SIZE>
		size_t __fifo_len (volatile fifobuff_t<SIZE>& buff)
		{
			buff.mutex.lock(); // Begin critical section

			size_t len = buff.length;

			buff.mutex.unlock(); // End critical section

			return len;
		}
	}
}

#endif