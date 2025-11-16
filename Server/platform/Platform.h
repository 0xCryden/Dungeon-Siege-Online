/*
*  This file is part of dsmmorpg.
*
*  dsmmorpg is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  dsmmorpg is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with dsmmorpg.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PLATFORM_HPP_
#define PLATFORM_HPP_

#include <cstdint>  // Standard fixed-width integer types
#include <cstring>  // For memset
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
#endif

#pragma comment(lib, "libxml2.lib")

// -------------------------
// Standardized integer aliases (cross-platform)
// -------------------------
/*using uint8_t = std::uint8_t;
using int8_t = std::int8_t;
using uint16_t = std::uint16_t;
using int16_t = std::int16_t;
using uint32_t = std::uint32_t;
using int32_t = std::int32_t;
using uint64_t = std::uint64_t;
using int64_t = std::int64_t;

// Boolean type alias (optional)
using boolean = unsigned char;

// Compatibility macro for bzero (zero memory)
#define bzero(b,len) (std::memset((b), '\0', (len)), (void)0)*/

#endif /* PLATFORM_HPP_ */
