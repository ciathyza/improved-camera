/*
Note: this file is not part of SKSE64. This file was added by the Universal Dll Project.
The purpose of this file is to provide alternatives to RelocAddr and RelocPtr, which can work based on signatures instead of addresses.
*/

#pragma once

#include "Relocation.h"
#include "Utilities.h"

// Uncomment me to get all addresses printed to RelocationEx.log
//#define RELOCATION_DEBUG

#define WILDCARD_CHAR	0x100
#define SEARCH_SIZE		0x100000
#define MAX_SIGNATURE	256

void DMESSAGE(const char * fmt, ...);
void FMESSAGE(const char * fmt, ...);

bool CompileSignature(const char *signature, UInt16 *parsed_signature, size_t *signature_size);
uintptr_t FindSignature(uintptr_t search_start, UInt16 *signature, size_t size, size_t search_size=SEARCH_SIZE, UInt16 *signature2=nullptr, size_t size2=0, int jump2_offset=0);

static inline void *GetAddrFromRel(void *addr, uint32_t offset = 0)
{
	uint8_t *addr_8 = (uint8_t *)addr;
	return (void *)(addr_8 + 4 + offset + *(int32_t *)addr);
}

// use this for addresses that represent pointers to a type T
template <typename T>
class RelocPtrEx
{
public:
	RelocPtrEx(uintptr_t search_start, const char *signature, int displacement=0, bool resolve_var=false, int var_offset=0)
	{
		UInt16 *compiled_signature = new UInt16[MAX_SIGNATURE];
		size_t size = MAX_SIGNATURE;

		if (!CompileSignature(signature, compiled_signature, &size))
			exit(-1);

		m_offset = FindSignature(search_start, compiled_signature, size);
		if (!m_offset)
			exit(-1);

		m_offset += displacement;

		if (resolve_var)
		{
			m_offset = (uintptr_t)GetAddrFromRel((void *)m_offset);
			m_offset += var_offset;
			DMESSAGE("Var resolved at %p (relative: %p)", (void *)m_offset, (void *)(m_offset - RelocationManager::s_baseAddr));
		}
		
		delete[] compiled_signature;
	}
	
	operator T *() const
	{
		return GetPtr();
	}

	T * operator->() const
	{
		return GetPtr();
	}

	T * GetPtr() const
	{
		return reinterpret_cast <T *>(m_offset);
	}

	const T * GetConst() const
	{
		return reinterpret_cast <T *>(m_offset);
	}

	uintptr_t GetUIntPtr() const
	{
		return m_offset;
	}

private:
	uintptr_t	m_offset;

	// hide
	RelocPtrEx();
	RelocPtrEx(RelocPtrEx & rhs);
	RelocPtrEx & operator=(RelocPtrEx & rhs);
};

// use this for direct addresses to types T. needed to avoid ambiguity
template <typename T>
class RelocAddrEx
{
public:
	RelocAddrEx(uintptr_t search_start, const char *signature, int displacement=0, bool resolve_var = false, int var_offset = 0)
	{
		UInt16 *compiled_signature = new UInt16[MAX_SIGNATURE];
		size_t size = MAX_SIGNATURE;

		if (!CompileSignature(signature, compiled_signature, &size))
			exit(-1);

		uintptr_t offset = FindSignature(search_start, compiled_signature, size);
		if (!offset)
			exit(-1);

		offset += displacement;

		if (resolve_var)
		{
			offset = (uintptr_t)GetAddrFromRel((void *)offset);
			offset += var_offset;
			DMESSAGE("Var resolved at %p (relative: %p)", (void *)offset, (void *)(offset - RelocationManager::s_baseAddr));
		}
		
		delete[] compiled_signature;

		m_offset = reinterpret_cast <BlockConversionType *>(offset);
	}

	RelocAddrEx(uintptr_t search_start, const char *signature, int jump_offset, const char *signature2)
	{
		UInt16 *compiled_signature = new UInt16[MAX_SIGNATURE];
		UInt16 *compiled_signature2 = new UInt16[MAX_SIGNATURE];
		size_t size = MAX_SIGNATURE;
		size_t size2 = MAX_SIGNATURE;

		if (!CompileSignature(signature, compiled_signature, &size))
			exit(-1);

		if (!CompileSignature(signature2, compiled_signature2, &size2))
			exit(-1);

		uintptr_t offset = FindSignature(search_start, compiled_signature, size, SEARCH_SIZE, compiled_signature2, size2, jump_offset);
		if (!offset)
			exit(-1);	

		delete[] compiled_signature;
		delete[] compiled_signature2;

		m_offset = reinterpret_cast <BlockConversionType *>(offset);
	}

	operator T()
	{
		return reinterpret_cast <T>(m_offset);
	}

	uintptr_t GetUIntPtr() const
	{
		return reinterpret_cast <uintptr_t>(m_offset);
	}

private:
	// apparently you can't reinterpret_cast from a type to the same type
	// that's kind of stupid and makes it impossible to use this for uintptr_ts if I use the same type
	// so we make a new type by storing the data in a pointer to this useless struct
	// at least this is hidden by a wrapper
	struct BlockConversionType { };
	BlockConversionType * m_offset;

	// hide
	RelocAddrEx();
	RelocAddrEx(RelocAddrEx & rhs);
	RelocAddrEx & operator=(RelocAddrEx & rhs);
};

// extend Utilities macros

#define DEFINE_MEMBER_FN_LONG_EX(className, functionName, retnType, address, sig, ...)		\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);			\
																					\
	inline _##functionName##_type * _##functionName##_GetPtr(void)					\
	{																				\
		static RelocPtrEx<void *> pex(address, sig);								\
		static uintptr_t _address = pex.GetUIntPtr();								\
		return (_##functionName##_type *)&_address;									\
	}

#define DEFINE_MEMBER_FN_EX(functionName, retnType, address, sig, ...)	\
	DEFINE_MEMBER_FN_LONG_EX(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, sig, __VA_ARGS__)

#define DEFINE_MEMBER_FN_LONG_EX_OFFSET(className, functionName, retnType, address, sig, offset, ...)		\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);			\
																					\
	inline _##functionName##_type * _##functionName##_GetPtr(void)					\
	{																				\
		static RelocPtrEx<void *> pex(address, sig, offset);								\
		static uintptr_t _address = pex.GetUIntPtr();								\
		return (_##functionName##_type *)&_address;									\
	}

#define DEFINE_MEMBER_FN_EX_OFFSET(functionName, retnType, address, sig, offset, ...)	\
	DEFINE_MEMBER_FN_LONG_EX_OFFSET(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, sig, offset, __VA_ARGS__)

#define DEFINE_MEMBER_FN_LONG_EX_JUMP(className, functionName, retnType, address, sig, jump_offset, sig2, ...)		\
	typedef retnType (className::* _##functionName##_type)(__VA_ARGS__);			\
																					\
	inline _##functionName##_type * _##functionName##_GetPtr(void)					\
	{																				\
		static RelocAddrEx<void *> pex(address, sig, jump_offset, sig2);								\
		static uintptr_t _address = pex.GetUIntPtr();								\
		return (_##functionName##_type *)&_address;									\
	}

#define DEFINE_MEMBER_FN_EX_JUMP(functionName, retnType, address, sig, jump_offset, sig2, ...)	\
	DEFINE_MEMBER_FN_LONG_EX_JUMP(_MEMBER_FN_BASE_TYPE, functionName, retnType, address, sig, jump_offset, sig2, __VA_ARGS__)