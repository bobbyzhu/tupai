//
// file : kthread.hpp
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

#ifndef TUPAI_SYS_KTHREAD_HPP
#define TUPAI_SYS_KTHREAD_HPP

// Tupai
#include <tupai/type.hpp>

// Standard
#include <stddef.h>
#include <stdint.h>

namespace tupai
{
	namespace sys
	{
		struct kthread_t
		{
			enum class state
			{
				NEW,
				ACTIVE,
				WAITING,
				DEAD,
			};

			const static size_t NAME_MAX_LEN = 64;

			id_t  id = -1;
			char name[NAME_MAX_LEN + 1];
			state cstate = state::DEAD;
			bool native = true; // Are thread components such as the stack native to this system?
			int argc = 0;
			char** argv = nullptr;

			size_t entry = 0x0;
			size_t stack = 0x0;
			size_t stackpos = 0x0;
		};

		void threading_init();
		bool threading_enabled();

		id_t   thread_create(void(*entry)(int argc, char* argv[]), int argc = 0, char* argv[] = nullptr, const char* name = "null", bool create_stack = true);
		id_t   thread_get_id();
		void   thread_kill(id_t id);
		size_t threads_count();
		id_t   thread_get_id(size_t i);
		bool __thread_get_name(id_t id, char* buff, size_t n);
		template <size_t SIZE>
		bool thread_get_name(id_t id, char(&buff)[SIZE]) { return __thread_get_name(id, buff, SIZE); }
		size_t thread_next_stack(size_t ostack);
	}
}

#endif
