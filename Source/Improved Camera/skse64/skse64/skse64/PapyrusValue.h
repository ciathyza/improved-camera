#pragma once

#include "skse64_common/Utilities.h"
#include "skse64_common/RelocationEx.h"
#include "skse64/GameTypes.h"

class VMClassInfo;

// ??
class VMIdentifier
{
public:
	enum
	{
		kLockBit = 0x80000000,
		kFastSpinThreshold = 10000
	};

	SInt32			m_refCount;	// 00
	UInt32			unk04;		// 04
	VMClassInfo		* m_type;	// 08
	void			* unk08;	// 10
	void			* unk0C;	// 18
	volatile UInt64	m_handle;	// 20
	volatile SInt32	m_lock;		// 28

	UInt64	GetHandle(void);

	SInt32	Lock(void);
	void	Unlock(SInt32 oldLock);

	// lock and refcount?
	void	IncrementLock(void);
	SInt32	DecrementLock(void);

	void	Destroy(void);

	MEMBER_FN_PREFIX(VMIdentifier);
	DEFINE_MEMBER_FN_EX(Destroy_Internal, void, 0x0124AA40, "40 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 40 - 48 89 74 24 48 - 48 8B F9 - F6 01 01");
};

// 10
class VMValue
{
public:
	VMValue() :type(kType_None) { data.p = 0; }
	~VMValue() { CALL_MEMBER_FN(this, Destroy)(); }

	VMValue(const VMValue & other)
	{
		if (&other != this)
		{
			type = kType_None;
			data.p = nullptr;
			CALL_MEMBER_FN(this, Set)(&other);
		}
	}
	VMValue& operator=(const VMValue& other)
	{
		if (&other == this)
			return *this;

		CALL_MEMBER_FN(this, Set)(&other);
		return *this;
	}

	enum
	{
		kType_None = 0,
		kType_Identifier = 1,
		kType_String = 2,
		kType_Int = 3,
		kType_Float = 4,
		kType_Bool = 5,

		kType_Unk0B = 0x0B,
		kType_ArraysStart = 11,
		kType_StringArray = 12,
		kType_IntArray = 13,
		kType_FloatArray = 14,
		kType_BoolArray = 15,
		kType_ArraysEnd = 16,

		kNumLiteralArrays = 4
	};

	// 18+
	struct ArrayData
	{
		volatile SInt32	refCount;	// 00
		UInt32			unk04;		// 04
		UInt64			unk08;		// 08
		UInt32			len;		// 10
		UInt32			unk14;		// 14
		UInt64			unk18;		// 18
		//VMValue			data[0];	// 20

		VMValue	*	GetData(void) { return (VMValue *)(this + 1); }

		MEMBER_FN_PREFIX(ArrayData);
		DEFINE_MEMBER_FN_EX(Destroy, void, 0x01259210, "40 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 40 - 48 8B F9 - 33 DB - 39 59 10");
	};

	UInt64	type;	// 00

	union
	{
		SInt32			i;
		UInt32			u;
		float			f;
		bool			b;
		void			* p;
		ArrayData		* arr;
		VMIdentifier	* id;
		const char		* str;	// BSFixedString

		BSFixedString *	GetStr(void) { return (BSFixedString *)(&str); }
	} data;			// 04

	MEMBER_FN_PREFIX(VMValue);
	DEFINE_MEMBER_FN_EX(Set, void, 0x0124E220, "48 89 4C 24 08 - 53 56 57 41 56 - 48 83 EC 38 - 48 C7 44 24 20 FE FF FF FF - 48 8B FA 48 8B F1", const VMValue * src);
	DEFINE_MEMBER_FN_EX(Destroy, void, 0x0124E0E0, "40 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 48 - 48 89 74 24 50 - 48 8B F9 - 8B 15 XXXXXXXX - 65 48 8B 04 25 58 00 00 00 - B9 68 07 00 00 - 48 8B 34 D0 - 48 03 F1 - 8B 06 - 89 44 24 40 - C7 06 14 00 00 00");
	DEFINE_MEMBER_FN_EX(SetArray, void, 0x0124D330, "48 89 4C 24 08 - 56 57 41 56 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 58 - 4C 8B F2 - 48 8B F9 - 8B 09 - 8D 41 F5 - 83 F8 04", ArrayData * data);

	bool	IsIdentifierArray()
	{
		return (type >= kType_ArraysEnd && type & kType_Identifier);
	}

	bool	IsLiteralArray()
	{
		return type - kType_ArraysStart <= kNumLiteralArrays;
	}

	bool	IsArray()
	{
		return IsLiteralArray() || IsIdentifierArray();
	}

	void	SetNone(void)
	{
		CALL_MEMBER_FN(this, Destroy)();

		type = kType_None;
		data.u = 0;
	}

	void	SetInt(SInt32 i)
	{
		CALL_MEMBER_FN(this, Destroy)();

		type = kType_Int;
		data.i = i;
	}

	void	SetFloat(float f)
	{
		CALL_MEMBER_FN(this, Destroy)();

		type = kType_Float;
		data.f = f;
	}

	void	SetBool(bool b)
	{
		CALL_MEMBER_FN(this, Destroy)();

		type = kType_Bool;
		data.b = b;
	}

	void	SetIdentifier(VMClassInfo * classInfo)
	{
		CALL_MEMBER_FN(this, Destroy)();

		type = (UInt64)classInfo;
		data.id = NULL;
	}

	void	SetIdentifier(VMIdentifier ** identifier)
	{
		if (GetUnmangledType() == kType_Identifier)
		{
			CALL_MEMBER_FN(this, Destroy)();

			if (*identifier)
				(*identifier)->IncrementLock();

			data.id = *identifier;
		}
	}

	void	SetString(const char * str)
	{
		CALL_MEMBER_FN(this, Destroy)();

		type = kType_String;
		CALL_MEMBER_FN(data.GetStr(), Set)(str);
	}

	// 00-0F are untouched
	// 10+ alternate between 0x01 and 0x0B
	UInt32	GetUnmangledType(void);

	bool	IsIdentifier(void) { return GetUnmangledType() == kType_Identifier; }
};
