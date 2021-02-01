#include "ScaleformAPI.h"

class ScaleformHeap
{
public:
	virtual void	Fn_00(void);
	virtual void	Fn_01(void);
	virtual void	Fn_02(void);
	virtual void	Fn_03(void);
	virtual void	Fn_04(void);
	virtual void	Fn_05(void);
	virtual void	Fn_06(void);
	virtual void	Fn_07(void);
	virtual void	Fn_08(void);
	virtual void	Fn_09(void);
	virtual void *	Allocate(size_t size, UInt32 unk = 0);	// unk is probably align, maybe flags (haven't traced)
	virtual void	Fn_0B(void);
	virtual void	Free(void * ptr);
};

// C70DB2D0DA8EB136C1BB8E87A7E39C173A7E4C0B+21
RelocPtrEx<ScaleformHeap *> g_scaleformHeap(0x852914, "83 FB 01 - 75 XX - 48 8B 0D XXXXXXXX - 48 8B 01", 8, true); // 3059C50 Note: More than 80 matches for this signature, but all of them resolve to scaleform heap.

void * ScaleformHeap_Allocate(UInt32 size)
{
	return (*(g_scaleformHeap))->Allocate(size);
}

void ScaleformHeap_Free(void * ptr)
{
	(*(g_scaleformHeap))->Free(ptr);
}

RelocAddrEx<_InvokeFunction> InvokeFunction(0x00ED6800, "40 57 - 48 83 EC 40 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 50 - 48 89 74 24 60 - 48 8B DA");
RelocAddrEx<_GFxAllocateHeap> GFxAllocateHeap(0x00F48100, "40 53 - 48 83 EC 20 - 48 83 3D XXXXXXXX 00 - 48 8B DA");
