/*
	Note: this file is not part of SKSE64. This file was added by the Universal Dll Project.
	The purpose of this file is to provide alternatives to RelocAddr and RelocPtr, which can work based on signatures instead of addresses.
*/

#include <shlobj.h>	
#include "RelocationEx.h"

IDebugLog relocLog; 

void DMESSAGE(const char * fmt, ...)
{
#ifdef RELOCATION_DEBUG	
	
	static bool log_opened = false;
	if (!log_opened)
	{
		relocLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\RelocationEx.log");
		relocLog.SetPrintLevel(IDebugLog::kLevel_Error);
		relocLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		log_opened = true;
	}
	
	va_list args;

	va_start(args, fmt);
	relocLog.Log(IDebugLog::kLevel_Message, fmt, args);
	va_end(args);
#endif
}

static char *GetMySelfName()
{
	HMODULE hMod;
	static char path[MAX_PATH];

	if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)GetMySelfName, &hMod))
	{
		strcpy_s(path, sizeof(path), "Unknown");
		return path;
	}

	GetModuleFileNameA(hMod, path, sizeof(path));

	char *p = strrchr(path, '\\');
	if (p)
		return (p + 1);

	return path;
}

void FMESSAGE(const char * fmt, ...)
{
	va_list args;
	static char dbg[128], dbg2[256];

	va_start(args, fmt);
	vsnprintf(dbg, sizeof(dbg), fmt, args);
	va_end(args);

	snprintf(dbg2, sizeof(dbg2), "%s\n\nModule that caused this error: %s.\n\nPlease remove the mod with that DLL and report a capture of this error to the author.", dbg, GetMySelfName());
	MessageBox(NULL, dbg2, "Universal DLL Project", MB_ICONERROR);
	
	ExitProcess(-1);
}

bool CompileSignature(const char *signature, UInt16 *compiled_signature, size_t *signature_size)
{
	size_t len = strlen(signature);
	size_t n = 0;
	bool first_nibble = true;
	UInt16 current_byte = 0;
	bool in_wildcard = false;

	for (size_t i = 0; i < len; i++)
	{
		char ch = toupper(signature[i]);
		UInt8 digit;

		if (ch <= ' ')  // skip empty spaces
		{
			continue;
		}
		else if (ch >= '0' && ch <= '9')
		{
			digit = ch - '0';
		}
		else if (ch >= 'A' && ch <= 'F')
		{
			digit = ch - 'A' + 0xA;
		}
		else if (ch == 'X')
		{
			if (!first_nibble)
			{
				if (!in_wildcard)
				{
					FMESSAGE("X cannot be used in a single nibble, it must be applied per byte (in signature %s)", signature);
					return false;
				}

				if (n >= *signature_size)
				{
					FMESSAGE("Signature too long, limit is %Id.", *signature_size);
					return false;
				}
				
				compiled_signature[n++] = WILDCARD_CHAR;
			}

			in_wildcard = !in_wildcard;
			first_nibble = !first_nibble;
			continue;
		}
		else // Ignore symbols, etc
		{
			continue;
		}

		if (in_wildcard)
		{
			FMESSAGE("X cannot be used in a single nibble, it must be applied per byte (in signature %s)", signature);
			return false;
		}

		if (first_nibble)
		{
			current_byte = digit << 4;
		}
		else
		{
			current_byte |= digit;

			if (n >= *signature_size)
			{
				FMESSAGE("Signature too long, limit is %Id.", *signature_size);
				return false;
			}

			compiled_signature[n++] = current_byte;
		}

		first_nibble = !first_nibble;
	}

	*signature_size = n;
	return true;
}

static bool IsPattern(size_t address, UInt16 *signature, size_t length)
{
	uint8_t *module_base = (uint8_t *)RelocationManager::s_baseAddr;

	for (size_t i = 0; i < length; i++)
	{
		if (signature[i] >= WILDCARD_CHAR)
			continue;

		if (module_base[address + i] != signature[i])
			return false;
	}

	return true;
}

uintptr_t FindSignature(uintptr_t search_start, UInt16 *signature, size_t size, size_t search_size, UInt16 *signature2, size_t size2, int jump2_offset)
{
	size_t address_lowest, address_highest;
	size_t address_down, address_up;

	address_lowest = (search_start < search_size) ? 0 : search_start - search_size;
	address_highest = search_start + search_size;

	address_down = search_start;
	address_up = search_start + 1;

	bool search_down_end = (address_down == address_lowest);
	bool search_up_end = (address_up == address_highest);

	while (!search_down_end || !search_up_end)
	{
		if (!search_down_end)
		{
			if (IsPattern(address_down, signature, size))
			{
				uintptr_t result = address_down + RelocationManager::s_baseAddr;
				bool found = false;

				if (signature2)
				{
					uintptr_t search_start2 = (uintptr_t)GetAddrFromRel((void *)(result + jump2_offset)) - RelocationManager::s_baseAddr;
					//DMESSAGE("search_start = %p", (void *)search_start2);					

					if (FindSignature(search_start2, signature2, size2, 1))
					{
						found = true;
					}
				}
				else
				{
					found = true;
				}

				if (found)
				{
					if (search_size != 1)
						DMESSAGE("Signature for [%p] found at %p (relative: %p)", (void *)search_start, (void *)result, (void *)address_down);

					return result;
				}
			}

			address_down--;
			if (address_down == address_lowest)
				search_down_end = true;
		}

		if (!search_up_end)
		{
			if (IsPattern(address_up, signature, size))
			{
				uintptr_t result = address_up + RelocationManager::s_baseAddr;
				bool found = false;

				if (signature2)
				{
					uintptr_t search_start2 = (uintptr_t)GetAddrFromRel((void *)(result + jump2_offset)) - RelocationManager::s_baseAddr;
					//DMESSAGE("search_start = %p", (void *)search_start2);
					
					if (FindSignature(search_start2, signature2, size2, 1))
					{
						found = true;
					}
				}
				else
				{
					found = true;
				}

				if (found)
				{
					if (search_size != 1)
						DMESSAGE("Signature for [%p] found at %p (relative: %p)", (void *)search_start, (void *)result, (void *)address_up);

					return result;
				}
			}

			address_up++;
			if (address_up == address_highest)
				search_up_end = true;
		}
	}
	
	if (search_size != 1)
		FMESSAGE("Signature with search_start=%x not found!", search_start);

	return 0;
}

