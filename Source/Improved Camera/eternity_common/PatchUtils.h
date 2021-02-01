#ifndef __PATCHUTILS_H__
#define __PATCHUTILS_H__

#include "Utils.h"

typedef void (* MemoryBreakpointHandler)(void *pc, void *addr);

namespace PatchUtils
{
	void *GetPtr(size_t rel_address, const char *mod=nullptr);
	ptrdiff_t RelAddress(void *ptr, const char *mod=nullptr);
	
	uint8_t Read8(size_t rel_address, const char *mod=nullptr);
	uint16_t Read16(size_t rel_address, const char *mod=nullptr);
	uint32_t Read32(size_t rel_address, const char *mod=nullptr);
	uint64_t Read64(size_t rel_address, const char *mod=nullptr);
	
	void Write8(void *address, uint8_t data);
	bool Write8(size_t rel_address, uint8_t data, const char *mod=nullptr);
	void Write16(void *address, uint16_t data);
	bool Write16(size_t rel_address, uint16_t data, const char *mod=nullptr);
	void Write32(void *address, uint32_t data);
	bool Write32(size_t rel_address, uint32_t data, const char *mod=nullptr);
	void Write64(void *address, uint64_t data);
	bool Write64(size_t rel_address, uint64_t data, const char *mod=nullptr);
	
	void Copy(void *dst, void *src, size_t size);
	bool Copy(size_t dst, void *src, size_t size, const char *mod=nullptr);
	bool Copy(void *dst, size_t src, size_t size, const char *mod=nullptr);
	
	void Fill(void *dst, uint8_t value, size_t size);
	bool Fill(size_t dst, uint8_t value, size_t size, const char *mod=nullptr);
	
	inline void Zero(void *dst, size_t size) { Fill(dst, 0, size); }
	inline bool Zero(size_t dst, size_t size, const char *mod=nullptr) { return Fill(dst, 0, size, mod); }
	
#if defined(CPU_X86) || defined(CPU_X86_64)
	inline void Nop(void *dst, size_t size) { Fill(dst, 0x90, size); } 
	inline bool Nop(size_t dst, size_t size, const char *mod=nullptr) { return Fill(dst, 0x90, size, mod); }
#endif

    bool Hook(void *address, void **orig, void *new_func);
	bool Hook(size_t rel_address, void **orig, void *new_func, const char *mod=nullptr);
    bool Hook(const char *mod, const char *func, void **orig, void *new_func);

    void *FindImport(const char *import_mod, const char *import_func, const char *mod=nullptr);
    bool HookImport(const char *import_mod, const char *import_func, void *new_func, const char *mod=nullptr);

    bool Unhook(void *address);
    bool Unhook(size_t rel_address, const char *mod=nullptr);
    bool Unhook(const char *mod, const char *func);

    bool HookCall(void *call_addr, void **orig, void *new_addr);

#ifdef CPU_X86_64

    void *AllocateIn32BitsArea(void *ref_addr, size_t size, bool executable=false);

#endif

    bool SetMemoryBreakpoint(void *addr, size_t len, MemoryBreakpointHandler handler);

}

#endif
