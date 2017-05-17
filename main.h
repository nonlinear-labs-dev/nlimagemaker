/***
  Copyright (c) 2017 Nonlinear Labs GmbH

  Authors: Pascal Huerst <pascal.huerst@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, see <http://www.gnu.org/licenses/>.
***/

#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>


struct header {
	uint32_t kernel_size;
	uint32_t kernel_offset;
	uint32_t dts_size;
	uint32_t dts_offset;
	uint32_t rootfs_size;
	uint32_t rootfs_offset;
} __attribute((packed));

#endif // MAIN_H
