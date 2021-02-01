#include "skse64/GameAPI.h"

// 4371A3D33EB5D62C7C548EBD5F760AF15A9B0B96+41
RelocPtrEx <Heap> g_mainHeap(0x397886, "8B D3 - 48 8D 0D XXXXXXXX - 48 C1 E2 04 - 45 33 C9 - 45 33 C0", 5, true); // 1EE4280 Note: multiple matches for this signature, but all of them will resolve g_mainHeap

// 75643FD50A96D1F83B1AC4797EC84EF1C53039AC+68
RelocPtrEx <ConsoleManager *> g_console(0x322F51, "0F 5A D2 - 66 49 0F 7E D0 - 48 8D 15 XXXXXXXX - 48 8B 0D XXXXXXXX", 0x12, true); // 2F270F0

// E1E59B64FDA5B8A9085AE9314353ABEEA0DB2823+C4
RelocPtrEx <UInt32> g_consoleHandle(0x316E1A, "80 79 1A 2B - 0F 85 XXXX0000 - 8B 05 XXXXXXXX", 0xC, true); // 2F7331C

// 52DD97B7C619EA732D3CD95637F449FC7A23DD12+24
RelocPtrEx<UInt32> g_TlsIndexPtr(0x78F4, "8B 0D XXXXXXXX - 65 48 8B 04 25 58 00 00 00 - BA 68 07 00 00", 2, true); // 34BCA98 Note: multiple signatures match, but all will resolve the tls

// BC8BF08A45C960EB35F2BAFEC9432C80365A1473+14A
RelocPtrEx<PlayerCharacter*> g_thePlayer(0x1347AA, "48 3B 3D XXXXXXXX - 74 XX - 49 8B 7E 40 - 49 8B 06", 3, true); // 2F4DEF8

void * Heap_Allocate(size_t size)
{
	return CALL_MEMBER_FN(g_mainHeap, Allocate)(size, 0, false);
}

void Heap_Free(void * ptr)
{
	CALL_MEMBER_FN(g_mainHeap, Free)(ptr, false);
}

void Console_Print(const char * fmt, ...)
{
	ConsoleManager * mgr = *g_console;
	if(mgr)
	{
		va_list args;
		va_start(args, fmt);

		CALL_MEMBER_FN(mgr, VPrint)(fmt, args);

		va_end(args);
	}
}

struct TLSData
{
	// thread local storage

	UInt8	unk000[0x600];	// 000
	UInt8	consoleMode;	// 600
	UInt8	pad601[7];		// 601
};

static TLSData * GetTLSData()
{
	UInt32 TlsIndex = *g_TlsIndexPtr;
	TLSData ** data = (TLSData **)__readgsqword(0x58);

	return data[TlsIndex];
}


bool IsConsoleMode(void)
{
	return GetTLSData()->consoleMode != 0;
}

__int64 GetPerfCounter(void)
{
	LARGE_INTEGER li;
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}
