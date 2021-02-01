#pragma once

#include "skse64_common/Utilities.h"
#include "skse64_common/RelocationEx.h"

class PlayerCharacter;

class Heap
{
public:
	MEMBER_FN_PREFIX(Heap);
	// UDP: Using the offset variants in these two, to avoid signature incompatibity that would happen if they were hooked by another plugin (like sse engine fixes with MemmoryManagement=true)
	DEFINE_MEMBER_FN_EX_OFFSET(Allocate, void *, 0x00C02450 + 0xD /* 0xC0245D */, "48 83 EC 30 - 65 48 8B 04 25 58 00 00 00 - 48 8B FA", -0xD, size_t size, size_t alignment, bool aligned);
	DEFINE_MEMBER_FN_EX_OFFSET(Free, void, 0x00C02750 + 9 /* 0xC02759 */, "48 89 5C 24 08 - 48 89 54 24 10 - 57 - 48 83 EC 20 - 80 39 00", -9, void * buf, bool aligned);
};

extern RelocPtrEx <Heap> g_mainHeap;

void * Heap_Allocate(size_t size);
void Heap_Free(void * ptr);

class ConsoleManager
{
public:
	MEMBER_FN_PREFIX(ConsoleManager);
	DEFINE_MEMBER_FN_EX(VPrint, void, 0x0085C4B0, "48 8B C4 - 57 41 54 41 55 41 56 41 57 - 48 83 EC 40 - 48 C7 40 C8 FE FF FF FF - 48 89 58 10 - 48 89 68 18 - 48 89 70 20 - 4D 8B F8", const char * fmt, va_list args);
//	DEFINE_MEMBER_FN(Print, void, 0x001D2050, const char * str);
};

extern RelocPtrEx <ConsoleManager *> g_console;
extern RelocPtrEx <UInt32> g_consoleHandle;
extern RelocPtrEx <PlayerCharacter*> g_thePlayer;

void Console_Print(const char * fmt, ...);
bool IsConsoleMode(void);
__int64 GetPerfCounter(void);
