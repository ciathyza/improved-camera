#pragma once

#include "skse64_common/RelocationEx.h"
#include "skse64_common/Utilities.h"

class VMIdentifier;

class IObjectHandlePolicy
{
public:
	IObjectHandlePolicy();
	virtual ~IObjectHandlePolicy();

	// this code heavily uses 64-bit values, so many of these arguments may be paired 64-bit (eax/edx)
	virtual bool	IsType(UInt32 typeID, UInt64 handle);
	virtual bool	Unk_02(UInt64 handle);
	virtual UInt64	GetInvalidHandle(void);
	virtual UInt64	Create(UInt32 typeID, void * srcData);
	virtual bool	IsREFR(UInt64 handle);	// return IsType(TESObjectREFR::kTypeID, handle);
	virtual UInt64	Unk_06(UInt32 unk0, UInt32 unk1);
	virtual UInt64	Unk_07(UInt32 unk0, UInt32 unk1);
	virtual void *	Resolve(UInt32 typeID, UInt64 handle);
	virtual void	AddRef(UInt64 handle);
	virtual void	Release(UInt64 handle);
	virtual void	GetName(UInt64 handle, void * outStr);

	//	void	** _vtbl;	// 00
};

extern RelocPtrEx<IObjectHandlePolicy	*> g_objectHandlePolicy;

class ObjectBindPolicy
{
public:
	ObjectBindPolicy();
	virtual ~ObjectBindPolicy();

	virtual void	Unk_01(UInt64 unk);
	virtual void	Unk_02(UInt64 unk);
	virtual void	Unk_03(UInt64 unk);
	virtual void	Unk_04(void);
	virtual UInt32	Unk_05(UInt32 unk0, UInt32 unk1);
	virtual void	Unk_06(UInt32 unk0, UInt32 unk1, UInt32 unk2, UInt32 unk3);
	virtual void	Unk_07(UInt32 unk0, UInt32 unk1, UInt32 unk2, UInt32 unk3, UInt32 unk4, UInt32 unk5);
	virtual bool	Unk_08(void);
	virtual void	Unk_09(UInt32 unk0, UInt32 unk1, UInt32 unk2, UInt32 unk3);
	virtual void	Unk_0A(UInt32 unk0, UInt32 unk1, UInt32 unk2, UInt32 unk3);
	virtual void	Unk_0B(UInt32 unk0, UInt32 unk1, UInt32 unk2, UInt32 unk3, UInt32 unk4);
	virtual void	Unk_0C(UInt32 unk0, UInt32 unk1, UInt32 unk2, UInt32 unk3, UInt32 unk4);

	//	void	** _vtbl;	// 00

	MEMBER_FN_PREFIX(ObjectBindPolicy);
	DEFINE_MEMBER_FN_EX(BindObject, void, 0x01244EA0, "40 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 48 - 44 8B 0D XXXXXXXX - 65 48 8B 04 25 58 00 00 00 - 41 BA 68 07 00 00 - 4A 8B 3C C8 - 49 03 FA - 8B 1F - 89 5C 24 40 - C7 07 18 00  00 00", VMIdentifier ** identifier, UInt64 handle);
};
