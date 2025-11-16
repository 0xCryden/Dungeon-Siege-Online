#include "Packet.h"
#include "../helper/Log.h"
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <intrin.h>
#else
#include <arpa/inet.h>
#endif

// --- 64-bit host/network helpers ---
/*inline uint64_t htonll(uint64_t value)
{
#ifdef _WIN32
	return _byteswap_uint64(value);
#else
	uint32_t hi = htonl(static_cast<uint32_t>(value >> 32));
	uint32_t lo = htonl(static_cast<uint32_t>(value & 0xFFFFFFFFULL));
	return (static_cast<uint64_t>(lo) << 32) | hi;
#endif
}*/

/*inline uint64_t ntohll(uint64_t value)
{
#ifdef _WIN32
	return _byteswap_uint64(value);
#else
	uint32_t hi = ntohl(static_cast<uint32_t>(value >> 32));
	uint32_t lo = ntohl(static_cast<uint32_t>(value & 0xFFFFFFFFULL));
	return (static_cast<uint64_t>(lo) << 32) | hi;
#endif
}*/
Packet::Packet() : m_size(2), m_position(2), m_data(NULL)
{
	m_data = (char*)malloc(2);
}

Packet::Packet(const char* buffer) : m_size(0), m_position(2), m_data(NULL)
{
	uint16_t size;
	memcpy((char*)(&size), buffer, sizeof(uint16_t));
	m_size = ntohs(size);

	m_data = (char*)malloc(m_size);
	memcpy(m_data, buffer, m_size);
}

Packet :: ~Packet()
{
	m_size = 0;
	m_position = 0;

	if (m_data)
	{
		free(m_data);
		m_data = NULL;
	}
}


void Packet::WriteUInt64(uint64_t data)
{
	uint64_t _data = htonl(data);
	Append((const char*)(&_data), sizeof(uint64_t));
}

void Packet::WriteUInt32(uint32_t data)
{
	uint32_t _data = htonl(data);
	Append((const char*)(&_data), sizeof(uint32_t));
}

void Packet::WriteInt32(int32_t data)
{
	int32_t _data = htonl(data);
	Append((const char*)(&_data), sizeof(int32_t));
}

void Packet::WriteUInt16(uint16_t data)
{
	uint16_t _data = htons(data);
	Append((const char*)(&_data), sizeof(uint16_t));
}

void Packet::WriteInt16(int16_t data)
{
	int16_t _data = htons(data);
	Append((const char*)(&_data), sizeof(int16_t));
}

void Packet::WriteUInt8(uint8_t data)
{
	Append((const char*)(&data), sizeof(uint8_t));
}

void Packet::WriteInt8(int8_t data)
{
	Append((const char*)(&data), sizeof(int8_t));
}

void Packet::WriteFloat(float data)
{
	Append((const char*)(&data), sizeof(float));
}

void Packet::WriteString(const string& buffer)
{
	uint16_t size = buffer.size() + 1;
	m_data = (char*)realloc(m_data, m_size + size);
	memcpy((m_data + m_size), buffer.c_str(), size);

	m_size += size;
	m_data[m_size - 1] = '\0';
}

uint32_t Packet::ReadUInt32()
{
	uint16_t size = sizeof(uint32_t);

	uint32_t data;
	memcpy(((char*)(&data)), (m_data + m_position), size);
	m_position += size;

	return ntohl(data);
}

int32_t Packet::ReadInt32()
{
	uint16_t size = sizeof(int32_t);

	int32_t data;
	memcpy(((char*)(&data)), (m_data + m_position), size);
	m_position += size;

	return ntohl(data);
}

uint16_t Packet::ReadUInt16()
{
	uint16_t size = sizeof(uint16_t);

	uint16_t data;
	memcpy(((char*)(&data)), (m_data + m_position), size);
	m_position += size;

	return ntohs(data);
}

int16_t Packet::ReadInt16()
{
	uint16_t size = sizeof(int16_t);

	int16_t data;
	memcpy(((char*)(&data)), (m_data + m_position), size);
	m_position += size;

	return ntohs(data);
}

uint8_t Packet::ReadUInt8()
{
	return (uint8_t)(*(m_data + m_position++));
}

int8_t Packet::ReadInt8()
{
	return (int8_t)(*(m_data + m_position++));
}

float Packet::ReadFloat()
{
	uint16_t size = sizeof(float);

	float data;
	memcpy(((char*)(&data)), (m_data + m_position), size);
	m_position += size;

	return data;
}

string Packet::ReadString()
{
	string buffer;

	try
	{
		while (m_position < m_size)
		{
			if (m_data[m_position++] == '\0') break;

			buffer += m_data[m_position - 1];
		}
	}
	catch (...)
	{
		return "";
	}

	return buffer;
}

uint16_t Packet::Size()
{
	return m_size;
}

const char* Packet::Data()
{
	uint16_t size = htons(m_size);
	memcpy(m_data, (char*)(&size), sizeof(uint16_t));

	return m_data;
}

void Packet::Append(const char* buffer, uint16_t size)
{
	m_data = (char*)realloc(m_data, m_size + size);
	memcpy((m_data + m_size), buffer, size);

	m_size += size;
}

// --- Constructors ---
/*Packet::Packet() : m_position(2)
{
	m_data.resize(2); // reserve first 2 bytes for size
}

Packet::Packet(const char* buffer)
{
	uint16_t size;
	memcpy(&size, buffer, sizeof(uint16_t));
	m_data.resize(ntohs(size));
	memcpy(m_data.data(), buffer, m_data.size());
	m_position = 2;
}

// --- Destructor ---
Packet::~Packet() = default;

// --- Append helper ---
void Packet::Append(const char* buffer, uint16_t size)
{
	m_data.insert(m_data.end(), buffer, buffer + size);
}

// --- Write functions ---
void Packet::WriteUInt8(uint8_t data) { Append(reinterpret_cast<const char*>(&data), sizeof(data)); }
void Packet::WriteInt8(int8_t data) { Append(reinterpret_cast<const char*>(&data), sizeof(data)); }
void Packet::WriteUInt16(uint16_t data) { uint16_t net = htons(data); Append(reinterpret_cast<const char*>(&net), sizeof(net)); }
void Packet::WriteInt16(int16_t data) { int16_t net = htons(data); Append(reinterpret_cast<const char*>(&net), sizeof(net)); }
void Packet::WriteUInt32(uint32_t data) { uint32_t net = htonl(data); Append(reinterpret_cast<const char*>(&net), sizeof(net)); }
void Packet::WriteInt32(int32_t data) { int32_t net = htonl(data); Append(reinterpret_cast<const char*>(&net), sizeof(net)); }
void Packet::WriteUInt64(uint64_t data) { uint64_t net = htonll(data); Append(reinterpret_cast<const char*>(&net), sizeof(net)); }
void Packet::WriteInt64(int64_t data) { int64_t net = htonll(static_cast<uint64_t>(data)); Append(reinterpret_cast<const char*>(&net), sizeof(net)); }

void Packet::WriteFloat(float data)
{
	uint32_t asInt;
	static_assert(sizeof(float) == sizeof(uint32_t), "float must be 32-bit");
	memcpy(&asInt, &data, sizeof(float));   // reinterpret float bits
	asInt = htonl(asInt);                   // convert to network order
	Append(reinterpret_cast<const char*>(&asInt), sizeof(asInt));
}

void Packet::WriteDouble(double data)
{
	uint64_t asInt;
	static_assert(sizeof(double) == sizeof(uint64_t), "double must be 64-bit");
	memcpy(&asInt, &data, sizeof(double));
	asInt = htonll(asInt);
	Append(reinterpret_cast<const char*>(&asInt), sizeof(asInt));
}

void Packet::WriteString(const string& str)
{
	uint16_t size = static_cast<uint16_t>(str.size() + 1);
	Append(str.c_str(), size);
	m_data[m_data.size() - 1] = '\0';
}

// --- Read functions ---
uint8_t  Packet::ReadUInt8() { return static_cast<uint8_t>(m_data[m_position++]); }
int8_t   Packet::ReadInt8() { return static_cast<int8_t>(m_data[m_position++]); }

uint16_t Packet::ReadUInt16() { uint16_t data; memcpy(&data, m_data.data() + m_position, sizeof(data)); m_position += sizeof(data); return ntohs(data); }
int16_t  Packet::ReadInt16() { int16_t data; memcpy(&data, m_data.data() + m_position, sizeof(data)); m_position += sizeof(data); return ntohs(data); }
uint32_t Packet::ReadUInt32() { uint32_t data; memcpy(&data, m_data.data() + m_position, sizeof(data)); m_position += sizeof(data); return ntohl(data); }
int32_t  Packet::ReadInt32() { int32_t data; memcpy(&data, m_data.data() + m_position, sizeof(data)); m_position += sizeof(data); return ntohl(data); }
uint64_t Packet::ReadUInt64() { uint64_t data; memcpy(&data, m_data.data() + m_position, sizeof(data)); m_position += sizeof(data); return ntohll(data); }
int64_t  Packet::ReadInt64() { int64_t data; memcpy(&data, m_data.data() + m_position, sizeof(data)); m_position += sizeof(data); return static_cast<int64_t>(ntohll(static_cast<uint64_t>(data))); }

float Packet::ReadFloat()
{
	uint32_t asInt;
	memcpy(&asInt, m_data.data() + m_position, sizeof(asInt));
	m_position += sizeof(asInt);
	asInt = ntohl(asInt);                   // convert back from network order
	float result;
	memcpy(&result, &asInt, sizeof(float));
	return result;
}

double Packet::ReadDouble()
{
	uint64_t asInt;
	memcpy(&asInt, m_data.data() + m_position, sizeof(asInt));
	m_position += sizeof(asInt);
	asInt = ntohll(asInt);
	double result;
	memcpy(&result, &asInt, sizeof(double));
	return result;
}

string Packet::ReadString()
{
	string buffer;
	while (m_position < m_data.size())
	{
		char c = m_data[m_position++];
		if (c == '\0') break;
		buffer += c;
	}
	return buffer;
}

// --- Misc ---
uint16_t Packet::Size() { return static_cast<uint16_t>(m_data.size()); }

const char* Packet::Data()
{
	uint16_t size = htons(static_cast<uint16_t>(m_data.size()));
	memcpy(m_data.data(), &size, sizeof(size));
	return m_data.data();
}*/

//old
/*#include "Packet.h"
#include "../helper/Log.h"

Packet :: Packet () : m_size (2), m_position (2), m_data (NULL)
{
	m_data = (char *) malloc (2);
}

Packet :: Packet (const char * buffer) : m_size (0), m_position (2), m_data (NULL)
{
	uint16_t size;
	memcpy ((char*)(&size), buffer, sizeof (uint16_t));
	m_size = ntohs (size);
	
	m_data = (char *) malloc (m_size);
	memcpy (m_data, buffer, m_size);
}

Packet :: ~Packet ()
{
	m_size = 0;
	m_position = 0;
	
	if (m_data)
	{
		free (m_data);
		m_data = NULL;
	}
}

void Packet :: WriteUInt32 (uint32_t data)
{
	uint32_t _data = htonl (data);
	Append ((const char*)(&_data), sizeof (uint32_t));
}

void Packet :: WriteInt32 (int32_t data)
{
	int32_t _data = htonl (data);
	Append ((const char*)(&_data), sizeof (int32_t));
}

void Packet :: WriteUInt16 (uint16_t data)
{
	uint16_t _data = htons (data);
	Append ((const char*)(&_data), sizeof (uint16_t));
}

void Packet :: WriteInt16 (int16_t data)
{
	int16_t _data = htons (data);
	Append ((const char*)(&_data), sizeof (int16_t));
}

void Packet :: WriteUInt8 (uint8_t data)
{
	if (data > 255)
	{
		Log::Write(Log::Level::WARNING, "[Warning] WriteUInt8 called with value " + to_string(data) + " > 255. Clamping to 255.", true);
		data = 255;  // or throw, depending on strictness
	}

	Append ((const char*)(&data), sizeof (uint8_t));
}

void Packet :: WriteInt8 (int8_t data)
{
	Append ((const char*)(&data), sizeof (int8_t));
}

void Packet :: WriteFloat (float data)
{
	Append ((const char*)(&data), sizeof (float));
}

void Packet :: WriteString (const string & buffer)
{
	uint16_t size = buffer.size() + 1;
	m_data = (char*) realloc (m_data, m_size + size);
	memcpy ((m_data + m_size), buffer.c_str(), size);
	
	m_size += size;
	m_data[m_size - 1] = '\0';
}

uint32_t Packet :: ReadUInt32 ()
{
	uint16_t size = sizeof (uint32_t);
	
	uint32_t data;
	memcpy (((char*)(&data)), (m_data + m_position), size);
	m_position += size;
	
	return ntohl (data);
}

int32_t Packet :: ReadInt32 ()
{
	uint16_t size = sizeof (int32_t);
	
	int32_t data;
	memcpy (((char*)(&data)), (m_data + m_position), size);
	m_position += size;
	
	return ntohl (data);
}

uint16_t Packet :: ReadUInt16 ()
{
	uint16_t size = sizeof (uint16_t);
	
	uint16_t data;
	memcpy (((char*)(&data)), (m_data + m_position), size);
	m_position += size;
	
	return ntohs (data);
}

int16_t Packet :: ReadInt16 ()
{
	uint16_t size = sizeof (int16_t);
	
	int16_t data;
	memcpy (((char*)(&data)), (m_data + m_position), size);
	m_position += size;
	
	return ntohs (data);
}

uint8_t Packet :: ReadUInt8 ()
{
	return (uint8_t)(*(m_data + m_position++));
}

int8_t Packet :: ReadInt8 ()
{
	return (int8_t)(*(m_data + m_position++));
}

float Packet :: ReadFloat ()
{
	uint16_t size = sizeof (float);
	
	float data;
	memcpy (((char*)(&data)), (m_data + m_position), size);
	m_position += size;
	
	return data;
}

string Packet :: ReadString ()
{
	string buffer;
	
	try
	{
		while (m_position < m_size)
		{
			if (m_data[m_position++] == '\0') break;
			
			buffer += m_data[m_position - 1];
		}
	}
	catch (...)
	{
		return "";
	}
	
	return buffer;
}

uint16_t Packet :: Size ()
{
	return m_size;
}

const char * Packet :: Data ()
{
	uint16_t size = htons (m_size);
	memcpy (m_data, (char*)(&size), sizeof (uint16_t));
	
	return m_data;
}

void Packet :: Append (const char * buffer, uint16_t size)
{
	m_data = (char*) realloc (m_data, m_size + size);
	memcpy ((m_data + m_size), buffer, size);
	
	m_size += size;
}*/
