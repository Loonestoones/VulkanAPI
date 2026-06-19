#pragma once
#include "KevDev_types.h"
#include <string>
#include <cstring>

using namespace std;


bool ReadFile(const char* fileName, string& outFile);
char* ReadBinaryFile(const char* pFileName, int& size);

void WriteBinaryFile(const char* pFilename, const void* pData, int size);

void OgldevError(const char* pFileName, uint line, const char* msg, ... );
void OgldevFileError(const char* pFileName, uint line, const char* pFileError);

// Use OGLDEV_ERROR0 for messages with NO extra variables
#define OGLDEV_ERROR0(msg) ::OgldevError(__FILE__, __LINE__, msg)

// Use OGLDEV_ERROR for messages WITH OR WITHOUT extra variables
// Note the :: prefix and the ## before __VA_ARGS__
#define OGLDEV_ERROR(msg, ...) ::OgldevError(__FILE__, __LINE__, msg, ##__VA_ARGS__)
#define OGLDEV_FILE_ERROR(FileError) OgldevFileError(__FILE__, __LINE__, FileError);


#define ZERO_MEM(a) memset(a, 0, sizeof(a))
#define ZERO_MEM_VAR(var) memset(&var, 0, sizeof(var))
#define ARRAY_SIZE_IN_ELEMENTS(a) (sizeof(a)/sizeof(a[0]))
#define ARRAY_SIZE_IN_BYTES(a) (sizeof(a[0]) * a.size())


#ifdef _WIN64
#define SNPRINTF _snprintf_s
#define VSNPRINTF vsnprintf_s
#define RANDOM rand
#define SRANDOM srand((unsigned)time(NULL))
#pragma warning (disable: 4566)
#else
#define SNPRINTF snprintf
#define VSNPRINTF vsnprintf
#define RANDOM random
#define SRANDOM srandom(getpid())
#endif
