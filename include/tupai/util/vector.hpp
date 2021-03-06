//
// file : vector.hpp
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

#ifndef TUPAI_UTIL_VECTOR_HPP
#define TUPAI_UTIL_VECTOR_HPP

// Tupai
#include <tupai/util/math.hpp>
#include <tupai/util/mem.hpp>
#include <tupai/util/cpp.hpp>
#include <tupai/panic.hpp>

// Standard
#include <stddef.h>
#include <stdint.h>

namespace tupai
{
	namespace util
	{
		template <typename T>
		struct vector_t
		{
			T* arr;
			size_t ccapacity;
			size_t csize;

			void __copy(const vector_t<T>& other)
			{
				if (other.csize > 0)
				{
					this->csize     = other.csize;
					this->ccapacity = other.ccapacity;
					this->arr       = (T*)(new uint8_t[this->ccapacity * sizeof(T)]);
				}
				else
				{
					this->arr       = nullptr;
					this->ccapacity = 0;
					this->csize     = 0;
				}

				for (size_t i = 0; i < this->csize; i ++)
					this->arr[i] = other.arr[i];
			}

			// Default constructor
			vector_t()
			{
				this->arr       = nullptr;
				this->ccapacity = 0;
				this->csize     = 0;
			}

			// Copy constructor
			vector_t(const vector_t<T>& other)
			{
				this->__copy(other);
			}

			// Move constructor
			vector_t(vector_t<T>&& other)
			{
				this->arr       = other.arr;
				this->ccapacity = other.ccapacity;
				this->csize     = other.csize;

				other.arr       = nullptr;
				other.ccapacity = 0;
				other.csize     = 0;
			}

			// Destructor
			~vector_t() noexcept
			{
				for (size_t i = 0; i < this->csize; i ++)
					this->arr[i].~T();

				if (arr != nullptr)
					delete (uint8_t*)this->arr;
			}

			// Copy assignment operator
			vector_t<T>& operator=(const vector_t<T>& other)
			{
				this->__copy(other);
				return *this;
			}

			// Move assignment operator
			vector_t<T>& operator=(vector_t<T>&& other) noexcept
			{
				delete (uint8_t*)this->arr;

				this->arr       = other.arr;
				this->ccapacity = other.ccapacity;
				this->csize     = other.csize;

				other.arr       = nullptr;
				other.ccapacity = 0;
				other.csize     = 0;

				return *this;
			}

			size_t size() const
			{
				return this->csize;
			}

			size_t capacity() const
			{
				return this->ccapacity;
			}

			void resize(size_t size)
			{
				T* old = this->arr;
				this->ccapacity = size;
				this->arr = (T*)(new uint8_t[this->ccapacity * sizeof(T)]);

				for (size_t i = 0; i < this->csize; i ++)
					this->arr[i] = old[i];

				if (old != nullptr)
					delete old;
			}

			T& get(size_t i)
			{
				return this->arr[i];
			}

			T& operator[](size_t i)
			{
				return this->arr[i];
			}

			void push(const T& item)
			{
				if (this->csize >= this->ccapacity)
					this->resize(util::max(this->ccapacity * 2, (size_t)1));

				this->arr[this->csize] = item;
				this->csize ++;
			}

			T pop()
			{
				if (this->csize <= 0)
					panic("Attempted to pop from empty vector");

				T item = this->arr[this->csize - 1];
				this->csize --;

				if (this->csize <= this->ccapacity / 2)
					this->resize(this->ccapacity / 2);

				return item;
			}
		};
	}
}

#endif
