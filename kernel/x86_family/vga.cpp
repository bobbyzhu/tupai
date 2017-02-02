/*
* 	file : vga.cpp
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
#include <tupai/x86_family/vga.hpp>
#include <tupai/x86_family/multiboot.hpp>
#include <tupai/i686/i686.hpp>
#include <tupai/i686/port.hpp>

#include <tupai/util/mem.hpp>
#include <tupai/kdebug.hpp>

namespace tupai
{
	namespace x86_family
	{
		extern "C" byte _binary_screenfont_psfu_start;

		static void vga_textmode_place_cursor(uint16 col, uint16 row);
		static void vga_textmode_place_entry(uint32 c, uint16 col, uint16 row, byte fg_color, byte bg_color);

		static inline uint32 color_blend(uint32 lower, uint32 higher);
		static void blit_character(uint32 c, uint16 x, uint16 y, uint32 fg_color, uint32 bg_color);

		struct psf2_header
		{
			// Magic heaader data
			static const byte MAGIC0 = 0x72;
			static const byte MAGIC1 = 0xb5;
			static const byte MAGIC2 = 0x4a;
			static const byte MAGIC3 = 0x86;

			// Bitflags
			static const byte HAS_UNICODE_TABLE = 0x01;

			// Supported versions
			static const byte MAX_VERSION = 0x00;

			// UTF-8 separators
			static const byte SEPARATOR = 0xFF;
			static const byte STARTSEQ  = 0xFE;

			uint8  magic[4];
			uint32 version;
			uint32 header_size;
			uint32 flags;
			uint32 glyph_num;
			uint32 glyph_size;
			uint32 height;
			uint32 width;
		};

		struct vga_config
		{
			enum class framebuffer_type
			{
				INDEXED = 0,
				RGB = 1,
				EGA_TEXT = 2,
			};

			uint32*           fb_addr;
			uint32            fb_pitch;
			uint32            fb_width;
			uint32            fb_height;
			uint8             fb_bpp;
			framebuffer_type  fb_type;
		};

		bool vga_initiated = false;
		vga_config config;

		uint32* rgb_buffered_framebuffer;
		uint16* textmode_buffered_framebuffer;
		static void vga_switch_buffers();

		virtualtty vga_virtualtty;
		uint32 vga_virtualtty_change_counter = 0;
		static void vga_virtualtty_changed();

		static const uint32 vga_virtualtty_palette[] =
		{
			0xFF000000, // Black
			0xFF800000, // Red
			0xFF008000, // Green
			0xFF808000, // Yellow
			0xFF800080, // Blue
			0xFF000080, // Magenta
			0xFF008080, // Cyan
			0xFFC0C0C0, // Light Grey

			0xFF808080, // Dark Grey
			0xFFFF0000, // Light Red
			0xFF00FF00, // Light Green
			0xFFFFFF00, // Light Yellow
			0xFF0000FF, // Light Blue
			0xFFFF00FF, // Light Magenta
			0xFF00FFFF, // Light Cyan
			0xFFFFFFFF, // White
		};

		bool vga_init()
		{
			multiboot_header::framebuffer fb = multiboot_get_framebuffer();

			// Read configuration
			config.fb_addr = (uint32*)fb.address;
			config.fb_pitch = fb.pitch;
			config.fb_width = fb.width;
			config.fb_height = fb.height;
			config.fb_bpp = fb.bpp;
			config.fb_type = (vga_config::framebuffer_type)fb.type;

			// If the video mode is textmode, we need to adjust the framebuffer address since we're in the higher half
			if (config.fb_type == vga_config::framebuffer_type::RGB)
			{
				// Set up a framebuffer for double-buffering
				rgb_buffered_framebuffer = util::alloc<uint32>(config.fb_width * config.fb_height).val();

				// Set up a virtual TTY for the screen
				vga_virtualtty = virtualtty_create(config.fb_width / 8, config.fb_height / 16);
				vga_virtualtty.change_signal_func = vga_virtualtty_changed;
			}
			else if (config.fb_type == vga_config::framebuffer_type::EGA_TEXT)
			{
				// Modify the framebuffer address for higher-half kernel
				config.fb_addr = (uint32*)((umem)config.fb_addr + KERNEL_VIRTUAL_OFFSET);

				// Set up a framebuffer for double-buffering
				textmode_buffered_framebuffer = util::alloc<uint16>(config.fb_width * config.fb_height).val();

				// Set up a virtual TTY for the screen
				vga_virtualtty = virtualtty_create(config.fb_width, config.fb_height);
				vga_virtualtty.change_signal_func = vga_virtualtty_changed;
			}

			// Everything is finished initiating
			vga_initiated = true;
			klog_init("Initiated VGA", vga_initiated);

			// Clear the screen
			vga_textmode_clear();

			return vga_initiated;
		}

		safeptr<virtualtty> vga_get_virtualtty()
		{
			if (!vga_initiated)
				return safeptr<virtualtty>(nullptr, false);

			return safeptr<virtualtty>(&vga_virtualtty, true);
		}

		static void vga_virtualtty_changed()
		{
			if (!vga_initiated)
				return;

			for (uint16 i = 0; i < vga_virtualtty.cols; i ++)
			{
				for (uint16 j = 0; j < vga_virtualtty.rows; j ++)
				{
					uint32 pos = j * vga_virtualtty.cols + i;
					ttyentry entry = vga_virtualtty.buffer[pos];

					if (entry.change_stamp < vga_virtualtty_change_counter)
						continue;

					byte fg_color = vga_virtualtty.default_fg_color;
					byte bg_color = vga_virtualtty.default_bg_color;

					if (entry.fg_color <= 0xF)
						fg_color = entry.fg_color;
					if (entry.bg_color <= 0xF)
						bg_color = entry.bg_color;

					if (config.fb_type == vga_config::framebuffer_type::RGB) // RGB linear framebuffer mode
					{
						blit_character(entry.c, i, j, vga_virtualtty_palette[fg_color], vga_virtualtty_palette[bg_color]);
					}
					else if (config.fb_type == vga_config::framebuffer_type::EGA_TEXT) // EGA text framebuffer mode
					{
						const byte textmode_color_table[] = {0x0, 0x4, 0x2, 0x6, 0x1, 0x5, 0x3, 0x7, 0x8, 0xC, 0xA, 0xE, 0x9, 0xD, 0xB, 0xF};

						// Translate the colour from ANSI to VGA textmode
						fg_color = textmode_color_table[fg_color];
						bg_color = textmode_color_table[bg_color];

						vga_textmode_place_entry(entry.c, i, j, fg_color, bg_color);
					}
				}
			}

			// Display cursor
			uint16 cursor_col = vga_virtualtty.cursor % vga_virtualtty.cols;
			uint16 cursor_row = vga_virtualtty.cursor / vga_virtualtty.cols;

			if (config.fb_type == vga_config::framebuffer_type::RGB) // RGB linear framebuffer mode
			{
				blit_character(' ', cursor_col, cursor_row, 0xFFFFFFFF, 0xFFFFFFFF);
			}
			else if (config.fb_type == vga_config::framebuffer_type::EGA_TEXT) // EGA text framebuffer mode
			{
				vga_textmode_place_cursor(cursor_col, cursor_row);
			}

			// Update the screen
			//if (config.fb_type == vga_config::framebuffer_type::RGB) // RGB linear framebuffer mode
				//vga_switch_buffers();

			vga_virtualtty_change_counter = vga_virtualtty.change_counter;
		}

		static void vga_textmode_place_cursor(uint16 col, uint16 row)
		{
			uint32 index = row * config.fb_width + col;

			// Cursor low bits
			port_out8(0x3D4, 0x0F);
			port_out8(0x3D5, (byte)(index & 0xFF));
			// Cursor high bits
			port_out8(0x3D4, 0x0E);
			port_out8(0x3D5, (byte)((index >> 8) & 0xFF));
		}

		static void vga_textmode_place_entry(uint32 c, uint16 col, uint16 row, byte fg_color, byte bg_color)
		{
			asm volatile ("xchg %bx, %bx");
			uint16 color = ((bg_color & 0x0F) << 4) | (fg_color & 0x0F);
			uint32 index = row * config.fb_width + col;
			((uint16*)config.fb_addr)[index] = (color << 8) | (uint8)c;
		}

		static inline uint32 color_blend(uint32 lower, uint32 upper)
		{
			byte a_upper = (upper >> 24) & 0xFF;

			byte r_lower = (lower >> 16) & 0xFF;
			byte g_lower = (lower >> 8)  & 0xFF;
			byte b_lower = (lower >> 0)  & 0xFF;

			byte r_upper = (upper >> 16) & 0xFF;
			byte g_upper = (upper >> 8)  & 0xFF;
			byte b_upper = (upper >> 0)  & 0xFF;

			byte r_final = (r_upper * a_upper + r_lower * (255 - a_upper)) >> 8;
			byte g_final = (g_upper * a_upper + g_lower * (255 - a_upper)) >> 8;
			byte b_final = (b_upper * a_upper + b_lower * (255 - a_upper)) >> 8;

			return (0xFF << 24) | (r_final << 16) | (g_final << 8) | b_final << 0;
		}

		static void blit_character(uint32 c, uint16 x, uint16 y, uint32 fg_color, uint32 bg_color)
		{
			if (config.fb_type != vga_config::framebuffer_type::RGB)
				return;

			psf2_header* header = (psf2_header*)&_binary_screenfont_psfu_start;

			uint8* glyph = (uint8*)((umem)&_binary_screenfont_psfu_start + header->header_size + ((c > 0 && c < header->glyph_num) ? c : 0) * header->glyph_size);

			bg_color = (bg_color & 0x00FFFFFF) | 0xA0000000;

			int offx = x * header->width;
			int offy = y * header->height;
			int w = config.fb_width;
			int h = config.fb_height;

			uint32* buff = config.fb_addr;
			//uint32* buff = rgb_buffered_framebuffer;
			uint32 skip = config.fb_pitch >> 2;
			//uint32 skip = config.fb_width;

			for (uint16 j = 0; j < header->height; j ++)
			{
				for (uint16 i = 0; i < header->width; i ++)
				{
					if (c == ' ') // TODO : Remove this optimisation hack
					{
						buff[(skip) * (offy + j) + (offx + i)] = bg_color;
						continue;
					}

					if (((*glyph >> (header->width - i)) & 0x01) > 0)
						buff[(skip) * (offy + j) + (offx + i)] = fg_color;
					else
						buff[(skip) * (offy + j) + (offx + i)] = bg_color;//color_blend(((((offx + i) * 256) / w) & 0xFF) + (((((offy + j) * 256) / h) & 0xFF) << 16), bg_color);
				}

				glyph += (header->width + 7) / 8;
			}
		}

		void vga_textmode_put_char(char c, uint16 col, uint16 row)
		{
			if (!vga_initiated)
				return;

			blit_character(c, col, row, vga_virtualtty.default_fg_color, vga_virtualtty.default_bg_color);
		}

		void vga_textmode_clear()
		{
			for (int i = 0; i < 80; i ++)
			{
				for (int j = 0; j < 25; j ++)
				{
					vga_textmode_put_char(' ', i, j);
				}
			}
		}

		static void vga_switch_buffers()
		{
			if (!vga_initiated)
				return;

			for (uint32 i = 0; i < config.fb_width; i ++)
			{
				for (uint32 j = 0; j < config.fb_height; j ++)
				{
					config.fb_addr[j * (config.fb_pitch / 4) + i] = rgb_buffered_framebuffer[j * config.fb_width + i];
				}
			}
		}
	}
}