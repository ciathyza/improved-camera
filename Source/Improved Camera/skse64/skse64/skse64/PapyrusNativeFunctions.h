// UDP: partial TODO

#pragma once

#include "skse64/GameTypes.h"
#include "skse64/GameAPI.h"
#include "skse64/PapyrusArgs.h"
#include "skse64/PapyrusVM.h"

// native function bindings for papyrus

// all of this is originally in namespace BSScript

// use this for T_Base when there is no base
struct StaticFunctionTag
{
	enum { kTypeID = 0 };
};

// stack frame?
class VMState
{
public:
	VMState();
	~VMState();

	VMArgList	* argList;					// 00
	UInt64		pad08[(0x38 - 0x08) >> 3];	// 08
	UInt32		numArgs;					// 38
};

// 08
class IFunction
{
public:
	IFunction()				{ }
	virtual ~IFunction()	{ }

//	void	** _vtbl;	// 00
	BSIntrusiveRefCounted refCount; // 08

	virtual BSFixedString *		GetName(void) = 0;
	virtual BSFixedString *		GetClassName(void) = 0;
	virtual BSFixedString *		GetStr10(void) = 0;
	virtual UInt64 *			GetReturnType(UInt64 * dst) = 0;
	virtual UInt64				GetNumParams(void) = 0;
	virtual UInt64 *			GetParam(UInt32 idx, BSFixedString * nameOut, UInt64 * typeOut) = 0;
	virtual UInt64				GetNumParams2(void) = 0;
	virtual bool				IsNative(void) = 0;
	virtual UInt8				GetUnk40(void) = 0;
	virtual bool				Unk_0A(void) = 0;
	virtual UInt32				Unk_0B(void) = 0;
	virtual UInt32				GetUnk44(void) = 0;
	virtual BSFixedString *		GetStr48(void) = 0;
	virtual void				Unk_0E(UInt32 unk) = 0;
	virtual UInt32				Invoke(UInt64 unk0, UInt64 unk1, VMClassRegistry * registry, VMState * unk3) = 0;
	virtual BSFixedString *		Unk_10(void) = 0;
	virtual bool				Unk_11(UInt32 unk0, UInt32 * unk1) = 0;
	virtual bool				GetParamName(UInt32 idx, UInt64 out) = 0;	// param list stuff
	virtual UInt8				GetUnk41(void) = 0;
	virtual void				SetUnk41(UInt8 arg) = 0;
	virtual bool				HasCallback(void) = 0;
	virtual bool				Run(VMValue * baseValue, VMClassRegistry * registry, UInt32 arg2, VMValue * resultValue, VMState * state) = 0;	// unique to each type combination
};

// BSScript::NF_util::NativeFunctionBase
// 2C
class NativeFunctionBase : public IFunction
{
public:
	NativeFunctionBase()			{ }
	virtual ~NativeFunctionBase()	{ }

	// 08
	struct ParameterInfo
	{
		// 08
		struct Entry
		{
			StringCache::Ref	name;	// 00

			union
			{
				UInt64			type;		// 08 - shared with VMValue::type
				VMClassInfo		* typePtr;	// 08
			};
		};

		Entry	* data;		// 00 length = numParams + unk0A
		UInt16	numParams;	// 08
		UInt16	unk0A;		// 0A
	};

	virtual BSFixedString *		GetName(void) override				{ return &m_fnName; }
	virtual BSFixedString *		GetClassName(void) override { return &m_className; }
	virtual BSFixedString *		GetStr10(void) override { return &unk10; }
	virtual UInt64 *			GetReturnType(UInt64 * dst) override { *dst = m_retnType; return dst; }
	virtual UInt64				GetNumParams(void) override { return m_params.unk0A; }
	virtual UInt64 *			GetParam(UInt32 idx, BSFixedString * nameOut, UInt64 * typeOut)
															{ return CALL_MEMBER_FN(this, Impl_GetParam)(idx, nameOut, typeOut); }
	virtual UInt64				GetNumParams2(void) override { return m_params.unk0A; }
	virtual bool				IsNative(void) override { return true; }
	virtual UInt8				GetUnk40(void) override { return unk40; }
	virtual bool				Unk_0A(void) override { return false; }
	virtual UInt32				Unk_0B(void) override { return 0; }
	virtual UInt32				GetUnk44(void) override { return unk44; }
	virtual BSFixedString *		GetStr48(void) override { return &unk48; }
	virtual void				Unk_0E(UInt32 unk) override { }							// always nop?
	virtual UInt32				Invoke(UInt64 unk0, UInt64 unk1, VMClassRegistry * registry, VMState * unk3) override
															{ return CALL_MEMBER_FN(this, Impl_Invoke)(unk0, unk1, registry, unk3); }
	virtual BSFixedString *		Unk_10(void) override { return CALL_MEMBER_FN(this, Impl_Fn10)(); }
	virtual bool				Unk_11(UInt32 unk0, UInt32 * unk1) override
															{ *unk1 = 0; return false; }
	virtual bool				GetParamName(UInt32 idx, UInt64 out) override						// ### param list stuff
															{ return CALL_MEMBER_FN(this, Impl_Fn12)(idx, out); }
	virtual UInt8				GetUnk41(void) override { return unk41; }
	virtual void				SetUnk41(UInt8 arg) override { unk41 = arg; }
	virtual bool				HasCallback(void) = 0;
	virtual bool				Run(VMValue * baseValue, VMClassRegistry * registry, UInt32 arg2, VMValue * resultValue, VMState * state) = 0;

	MEMBER_FN_PREFIX(NativeFunctionBase);
	DEFINE_MEMBER_FN_EX(Impl_dtor, void, 0x012682A0, "40 53 - 48 83 EC 20 - 48 8D 05 XXXXXXXX - 48 8B D9 - 48 89 01 - 48 83 C1 48 - E8 XXXXXXXX - 48 8D 4B 30");
	// ??_7?$NativeFunction1@VActiveEffect@@XPEAVTESForm@@@BSScript@@6B@ entry 6
	DEFINE_MEMBER_FN_EX(Impl_GetParam, UInt64 *, 0x01267A40, "48 89 5C 24 08 - 48 89 74 24 10 - 57 - 48 83 EC 20 - 0F B7 41 38", UInt32 idx, BSFixedString * nameOut, UInt64 * typeOut); // Note: thee matches, but the 3 funtions are 100% identical
	DEFINE_MEMBER_FN_EX(Impl_Invoke, UInt32, 0x01267BC0, "48 8B C4 - 4C 89 48 20 - 4C 89 40 18 - 55 56 57 41 54 41 55 41 56 41 57 - 48 8D A8 68 F8 FF FF", UInt64 unk0, UInt64 unk1, VMClassRegistry * registry, VMState * unk3);
	// ??_7?$NativeFunction1@VActiveEffect@@XPEAVTESForm@@@BSScript@@6B@ entry 0x10
	DEFINE_MEMBER_FN(Impl_Fn10, BSFixedString *, 0x01267B20); // UDP: TODO
	// ??_7?$NativeFunction1@VActiveEffect@@XPEAVTESForm@@@BSScript@@6B@ entry 0x12
	DEFINE_MEMBER_FN_EX(Impl_Fn12, bool, 0x01267B40, "48 89 5C 24 08 - 57 - 48 83 EC 20 - 0F B7 41 3A", UInt32 idx, UInt64 out); // Note: thee matches, but the 3 funtions are 100% identical

	DEFINE_STATIC_HEAP(Heap_Allocate, Heap_Free);

protected:
	BSFixedString		m_fnName;		// 10
	BSFixedString		m_className;	// 18
	BSFixedString		unk10;			// 20
	UInt64				m_retnType;		// 28 - return type
	ParameterInfo		m_params;		// 30
	UInt8				unk40;			// 40 - related to extra entries in m_params
	UInt8				unk41;			// 41
	UInt8				m_isLatent;		// 42
	UInt8				pad43;			// 43
	UInt32				unk44;			// 44
	BSFixedString		unk48;			// 48
};

// 30
// this should be fully functional for deriving
class NativeFunction : public NativeFunctionBase
{
public:
	NativeFunction(const char * fnName, const char * className, UInt8 isStatic, UInt32 numParams)
								{ CALL_MEMBER_FN(this, Impl_ctor)(fnName, className, isStatic, numParams); }
	// lower class destructors are invoked by this call
	virtual ~NativeFunction()	{ CALL_MEMBER_FN(this, Impl_dtor)(); }

	virtual bool				HasCallback(void)	{ return m_callback != 0; }
	virtual bool				Run(VMValue * baseValue, VMClassRegistry * registry, UInt32 arg2, VMValue * resultValue, VMState * state) = 0;

	MEMBER_FN_PREFIX(NativeFunction);
	DEFINE_MEMBER_FN_EX(Impl_ctor, NativeFunction *, 0x01268050, "48 89 4C 24 08 - 55 56 57 41 56 41 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 70 - 45 0F B6 F1 - 49 8B F0", const char * fnName, const char * className, UInt32 unk0, UInt32 numParams);
	DEFINE_MEMBER_FN_EX(Impl_dtor, void, 0x012682A0, "40 53 - 48 83 EC 20 - 48 8D 05 XXXXXXXX - 48 8B D9 - 48 89 01 - 48 83 C1 48 - E8 XXXXXXXX - 48 8D 4B 30");

	void	DebugRunHook(VMValue * baseValue, VMClassRegistry * registry, UInt32 arg2, VMValue * resultValue, VMState * state);

protected:
	void	* m_callback;	// 2C

	// hide
	NativeFunction();
};

#define NUM_PARAMS 0
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 1
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 2
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 3
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 4
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 5
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 6
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 7
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 8
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 9
#include "skse64/PapyrusNativeFunctionDef.inl"

#define NUM_PARAMS 10
#include "skse64/PapyrusNativeFunctionDef.inl"

// Less boilerplate for checking of input parameters in native function callbacks.

#define ERROR_AND_RETURN_IF(condition, message, registry, stackId) \
	if (condition)								\
	{											\
		registry->LogError(message, stackId);	\
		return;									\
	}

#define ERROR_AND_RETURN_0_IF(condition, message, registry, stackId) \
	if (condition)								\
	{											\
		registry->LogError(message, stackId);	\
		return 0;								\
	}
