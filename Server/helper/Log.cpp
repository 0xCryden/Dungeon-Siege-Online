// Log.cpp
#include "Log.h"

std::ofstream Log::m_file;
std::mutex Log::m_mutex;
bool Log::m_initialized = false;

void Log::Init(const std::string& filename)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_initialized)
	{
		try
		{
			m_file.exceptions(std::ofstream::failbit | std::ofstream::badbit);
			m_file.open(filename, std::ios::app);
			m_initialized = true;
		}
		catch (const std::ofstream::failure& e)
		{
			std::cerr << "Failed to open log file '" << filename << "': " << e.what() << std::endl;
		}
	}
}

std::string Log::LevelPrefix(Level level)
{
	switch (level)
	{
	case Level::INFO:    return "[INFO] ";
	case Level::WARNING: return "[WARN] ";
	case Level::ERR:   return "[ERROR]";
	}
	return "[UNKNOWN]";
}

std::string Log::GetTimestamp()
{
	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);
	std::tm local_tm{};
#ifdef _WIN32
	localtime_s(&local_tm, &now_time);
#else
	localtime_r(&now_time, &local_tm);
#endif
	char buffer[32];
	std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &local_tm);
	return std::string(buffer);
}

void Log::Write(Level level, const std::string& message, bool outputConsole)
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (!m_file.is_open()) return;

	std::string line = "[" + GetTimestamp() + "] " + LevelPrefix(level) + " " + message;

	m_file << line << "\n";
	if (outputConsole)
		std::cout << line << std::endl;

	//m_file.flush(); // optional: could remove for performance
}

void Log::WriteF(Level level, const char* fmt, ...)
{
	char buffer[1024];

	va_list args;
	va_start(args, fmt);
#ifdef _WIN32
	vsnprintf_s(buffer, sizeof(buffer), _TRUNCATE, fmt, args);
#else
	vsnprintf(buffer, sizeof(buffer), fmt, args);
#endif
	va_end(args);

	Write(level, buffer);
}

void Log::Flush()
{
	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_file.is_open())
		m_file.flush();
}

/*#include "Log.h"
#include <iostream>

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

void Log::Write(const char* write, bool outputConsole)
{
	if (m_file.is_open())
	{
		// Get current time
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm{};

#ifdef _WIN32
		localtime_s(&local_tm, &now_time); // Windows
#else
		localtime_r(&now_time, &local_tm); // POSIX (Linux, macOS)
#endif
		// Write timestamp and log message
		m_file << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ";
		m_file << write << "\n";
		if (outputConsole == true) {
			std::cout << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] " << write << std::endl;
		}
		m_file.flush();
	}
}

void Log::Write(string write, bool outputConsole)
{
	if (m_file.is_open())
	{
		// Get current time
		auto now = std::chrono::system_clock::now();
		std::time_t now_time = std::chrono::system_clock::to_time_t(now);
		std::tm local_tm{};

#ifdef _WIN32
		localtime_s(&local_tm, &now_time); // Windows
#else
		localtime_r(&now_time, &local_tm); // POSIX (Linux, macOS)
#endif
		// Write timestamp and log message
		m_file << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] ";
		m_file << write << "\n";
		if (outputConsole == true) {
			std::cout << "[" << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "] " << write << std::endl;
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
		vsprintf_s(buffer, write, list);
		va_end (list);
		
		Write (buffer);
	}
}
*/