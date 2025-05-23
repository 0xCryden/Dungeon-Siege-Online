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

#include "Log.hpp"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>

Log logger;

Log :: Log ()
{
	m_file.open ("log.txt", ios::app);
}

Log :: ~Log ()
{
	if (m_file.is_open())
	{
		m_file.close();
	}
}

void Log :: Write (const char * write, bool outputConsole)
{
	if (m_file.is_open())
	{
		// Get current time
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);

		// Write timestamp and log message
		m_file << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] ";
		m_file << write << "\n";
		if (outputConsole == true) {
			std::cout << "[" << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "] " << write <<  std::endl;
		}
		m_file.flush();
	}
}

void Log :: WriteF (const char * write, ...)
{
	if (m_file.is_open())
	{
		char buffer[1024];
		
		va_list list;
		va_start (list, write);
		vsprintf (buffer, write, list);
		va_end (list);
		
		Write (buffer);
	}
}
