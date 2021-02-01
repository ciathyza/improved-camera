#pragma once

#include "skse64/ScaleformTypes.h"
#include "skse64_common/Utilities.h"

class GFxMovieRoot;

// 10
class GFxValue
{
public:
	GFxValue()
		:objectInterface(NULL), type(kType_Undefined) { }
	~GFxValue();

	enum
	{
		kType_Undefined = 0,
		kType_Null,
		kType_Bool,
		kType_Number,
		kType_String,
		kType_WideString,
		kType_Object,
		kType_Array,
		kType_DisplayObject,

		kTypeFlag_Managed = 1 << 6,

		kMask_Type = 0x8F,	// not sure why it checks the top bit
	};

	union Data
	{
		double			number;
		bool			boolean;
		const char		* string;
		const char		** managedString;
		const wchar_t	* wideString;
		const wchar_t	** managedWideString;
		void			* obj;
	};

	//#pragma pack (push, 8)
	class DisplayInfo
	{
	public:
		DisplayInfo() : _varsSet(0) {}
		enum
		{
			kChange_x = (1 << 0),
			kChange_y = (1 << 1),
			kChange_rotation = (1 << 2),
			kChange_xscale = (1 << 3),
			kChange_yscale = (1 << 4),
			kChange_alpha = (1 << 5),
			kChange_visible = (1 << 6),
			kChange_z = (1 << 7),
			kChange_xrotation = (1 << 8),
			kChange_yrotation = (1 << 9),
			kChange_zscale = (1 << 10),
			kChange_FOV = (1 << 11),
			kChange_projMatrix3D = (1 << 12),
			kChange_viewMatrix3D = (1 << 13)
		};

		double		_x;
		double		_y;
		double		_rotation;
		double		_xScale;
		double		_yScale;
		double		_alpha;
		bool		_visible;
		double		_z;
		double		_xRotation;
		double		_yRotation;
		double		_zScale;
		double		_perspFOV;
		GMatrix3D	_viewMatrix3D;
		GMatrix3D	_perspectiveMatrix3D;
		UInt16		_varsSet;

		void SetX(double x) { SetFlags(kChange_x); _x = x; }
		void SetY(double y) { SetFlags(kChange_y); _y = y; }
		void SetRotation(double degrees) { SetFlags(kChange_rotation); _rotation = degrees; }
		void SetXScale(double xscale) { SetFlags(kChange_xscale); _xScale = xscale; }
		void SetYScale(double yscale) { SetFlags(kChange_yscale); _yScale = yscale; }
		void SetAlpha(double alpha) { SetFlags(kChange_alpha); _alpha = alpha; }
		void SetVisible(bool visible) { SetFlags(kChange_visible); _visible = visible; }
		void SetZ(double z) { SetFlags(kChange_z); _z = z; }
		void SetXRotation(double degrees) { SetFlags(kChange_xrotation); _xRotation = degrees; }
		void SetYRotation(double degrees) { SetFlags(kChange_yrotation); _yRotation = degrees; }
		void SetZScale(double zscale) { SetFlags(kChange_zscale); _zScale = zscale; }
		void SetFOV(double fov) { SetFlags(kChange_FOV); _perspFOV = fov; }
		void SetProjectionMatrix3D(const GMatrix3D *pmat)
		{
			if (pmat) {
				SetFlags(kChange_projMatrix3D);
				_perspectiveMatrix3D = *pmat;
			}
			else
				ClearFlags(kChange_projMatrix3D);
		}
		void SetViewMatrix3D(const GMatrix3D *pmat)
		{
			if (pmat) {
				SetFlags(kChange_viewMatrix3D);
				_viewMatrix3D = *pmat;
			}
			else
				ClearFlags(kChange_viewMatrix3D);
		}

		// Convenience functions
		void SetPosition(double x, double y) { SetFlags(kChange_x | kChange_y); _x = x; _y = y; }
		void SetScale(double xscale, double yscale) { SetFlags(kChange_xscale | kChange_yscale); _xScale = xscale; _yScale = yscale; }

		void SetFlags(UInt32 flags) { _varsSet |= flags; }
		void ClearFlags(UInt32 flags) { _varsSet &= ~flags; }
	};
	//#pragma pack (pop)

	// 4
	class ObjectInterface
	{
	public:
		GFxMovieRoot	* root;

		MEMBER_FN_PREFIX(ObjectInterface);
		DEFINE_MEMBER_FN_EX(HasMember, bool, 0x00ECA360, "48 89 5C 24 08 - 48 89 74 24 18 - 57 - 48 83 EC 60 - 49 8B D8 - 48 8B FA - 48 8B F1", void * obj, const char * name, bool isDisplayObj);
		DEFINE_MEMBER_FN_EX(SetMember, bool, 0x00ECC8E0, "48 89 6C 24 18 - 56 57 41 56 - 48 83 EC 50 - 80 BC 24 90 00 00 00 00", void * obj, const char * name, GFxValue * value, bool isDisplayObj);
		DEFINE_MEMBER_FN_EX(DeleteMember, bool, 0x00EC91D0, "48 89 5C 24 08 - 48 89 74 24 18 - 57 - 48 83 EC 20 - 49 8B D8 - 48 8B FA - 48 8B F1", void * obj, const char * name, bool isDisplayObj);
		DEFINE_MEMBER_FN_EX(GetMember, bool, 0x00EC9E90, "48 89 5C 24 18 - 56 57 41 56 - 48 83 EC 40 - 80 BC 24 80 00 00 00 00", void * obj, const char * name, GFxValue * value, bool isDisplayObj);
		DEFINE_MEMBER_FN_EX(Invoke, bool, 0x00ECA5A0, "40 53 41 54 41 56 41 57 - 48 81 EC A8 00 00 00 - 80 BC 24 00 01 00 00 00", void * obj, GFxValue * result, const char * name, GFxValue * args, UInt32 numArgs, bool isDisplayObj);
		DEFINE_MEMBER_FN_EX(AttachMovie, bool, 0x00EC80E0, "40 55 53 57 41 54 41 56 41 57 - 48 8D 6C 24 A8 - 48 81 EC 58 01 00 00", void * obj, GFxValue * value, const char * symbolName, const char * instanceName, SInt32 depth, void * initArgs);
		DEFINE_MEMBER_FN_EX(PushBack, bool, 0x00ECB040, "40 53 - 48 83 EC 40 - 49 8B C0 - 48 85 D2 - 74 06", void * obj, GFxValue * value);
		DEFINE_MEMBER_FN_EX(SetText, bool, 0x00ECCA30, "48 89 5C 24 08 - 48 89 6C 24 10 - 48 89 74 24 18 - 48 89 7C 24 20 - 41 56 - 48 83 EC 50 - 48 8B EA - 4C 8B F1 - 48 8B 11 - 41 0F B6 F9 - 48 8B CD - 49 8B F0 - E8 XXXXXXXX - 48 8B D8 - 48 85 C0 - 74 68 - 48 8B 00 - 48 8B CB - FF 90 60 01 00 00 - 83 F8 04 - 74 42 - 40 84 FF - 48 C7 44 24 30 00 00 00 00 - 48 8D 05 XXXXXXXX - C7 44 24 38 04 00 00 00", void * obj, const char * text, bool html);
		//DEFINE_MEMBER_FN(PopBack, bool, 0x00000000, void * obj, GFxValue * value);
		DEFINE_MEMBER_FN_EX(GetArraySize, UInt32, 0x00EC9910, "48 85 D2 - 74 04 - 8B 42 58 C3 - 33 C0 - 8B 40 78 - C3", void * obj);
		//DEFINE_MEMBER_FN(SetArraySize, bool, 0x00000000, void * obj, UInt32 size);
		DEFINE_MEMBER_FN_EX(GetElement, bool, 0x00EC9C70, "48 89 5C 24 08 - 48 89 74 24 10 - 48 89 7C 24 18 - 41 56 - 48 83 EC 20 - 49 63 F0", void * obj, UInt32 index, GFxValue * value);
		//DEFINE_MEMBER_FN(SetElement, bool, 0x00000000, void * obj, UInt32 index, GFxValue * value);
		DEFINE_MEMBER_FN_EX(GotoLabeledFrame, bool, 0x00ECA2B0, "48 89 5C 24 10 - 48 89 74 24 18 - 57 - 48 83 EC 20 - 48 8B C2 - 41 0F B6 F1", void * obj, const char * frameLabel, bool stop);
		//DEFINE_MEMBER_FN(GotoFrame, bool, 0x00000000, void * obj, UInt32 frameNumber, bool stop);
		DEFINE_MEMBER_FN_EX(GetDisplayInfo, bool, 0x00EC9960, "48 89 5C 24 20 - 57 - 48 81 EC F0 00 00 00 - 48 8B 05 XXXXXXXX - 48 33 C4", void * obj, DisplayInfo * displayInfo);
		DEFINE_MEMBER_FN_EX(SetDisplayInfo, bool, 0x00ECBAF0, "40 55 56 57 - 48 8D 6C 24 B0 - 48 81 EC 50 01 00 00", void * obj, DisplayInfo * displayInfo);

		DEFINE_MEMBER_FN_EX(AddManaged_Internal, void, 0x00ECADC0, "48 83 EC 28 - 8B 42 08 - 25 8F 00 00 00 - 83 F8 04", GFxValue * value, void * obj);
		DEFINE_MEMBER_FN_EX(ReleaseManaged_Internal, void, 0x00ECAE20, "40 53 - 48 83 EC 20 - 8B 42 08 - 49 8B D8 - 25 8F 00 00 00", GFxValue * value, void * obj);
	};

	ObjectInterface	* objectInterface;	// 00
	UInt32			type;				// 08
	Data			data;				// 10

	UInt32	GetType(void) const { return type & kMask_Type; }
	bool	IsManaged(void) const { return (type & kTypeFlag_Managed) != 0; }
	void	CleanManaged(void);
	void	AddManaged(void);

	bool	IsObject(void) const { return GetType() == kType_Object; }
	bool	IsDisplayObject(void) const { return GetType() == kType_DisplayObject; }

	bool			GetBool(void) const;
	const char *	GetString(void) const;
	const wchar_t *	GetWideString(void) const;
	double			GetNumber(void) const;

	void	SetUndefined(void);
	void	SetNull(void);
	void	SetBool(bool value);
	void	SetNumber(double value);
	void	SetString(const char * value);
	void	SetWideString(const wchar_t * value);

	UInt32	GetArraySize();
	bool	GetElement(UInt32 index, GFxValue * value);
	bool	HasMember(const char * name);
	bool	SetMember(const char * name, GFxValue * value);
	bool	GetMember(const char * name, GFxValue * value);
	bool	DeleteMember(const char * name);
	bool	Invoke(const char * name, GFxValue * result, GFxValue * args, UInt32 numArgs);
	bool	PushBack(GFxValue * value);
	bool	GetDisplayInfo(DisplayInfo * displayInfo);
	bool	SetDisplayInfo(DisplayInfo * displayInfo);
	bool	SetText(const char * text, bool html);
};
