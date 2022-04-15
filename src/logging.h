#pragma once

#ifdef _DEBUG

#include <stdio.h>

#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)

#define LOG(message, ...) wprintf(message L"\n", __VA_ARGS__)
#define ERR(message, ...) LOG(L"Error: " message, __VA_ARGS__); \
		LOG(L"\tLogged at %S:%d [%S(...)]", __FILENAME__, __LINE__-1, __FUNCTION__)
#define LOGARRAY(array, size, asMessage) wprintf(L"["); \
						for(int __i = 0; __i < size; __i++) wprintf(asMessage L"", *(array+__i)); \
						wprintf(L"]\n")
#else //Release
#define LOG(...) ;
#define ERR(...) ;
#define LOGARRAY(...) ;
#endif 


