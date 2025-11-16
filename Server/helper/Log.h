/*#pragma once
#include "../Common.h"
#include <fstream>
	
class Log
{
	public:
		Log ();
		~Log ();
			
		static void Write(const char * write, bool outputConsole = false);
		static void Write(string write, bool outputConsole);
		static void WriteF(const char * write, ...);
			
	private:
		static ofstream m_file;
};*/

#pragma once
#include <fstream>
#include <string>
#include <mutex>
#include <chrono>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <cstdarg>

class Log
{
public:
    enum class Level { INFO, WARNING, ERR };

    // --- Static logging functions ---
    static void Init(const std::string& filename = "log.txt");
    static void Write(Level level, const std::string& message, bool outputConsole = true);
    static void WriteF(Level level, const char* fmt, ...);
    static void Flush();

private:
    static std::ofstream m_file;
    static std::mutex m_mutex;
    static bool m_initialized;

    // Disable instantiation
    Log() = delete;
    ~Log() = delete;
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    static std::string LevelPrefix(Level level);
    static std::string GetTimestamp();
};
