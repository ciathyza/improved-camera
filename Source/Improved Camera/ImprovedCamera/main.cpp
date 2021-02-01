#include "skse64/PluginAPI.h"
#include "skse64/PapyrusArgs.h"
#include "skse64/PapyrusNativeFunctions.h"
#include "skse64/ScaleformCallbacks.h"
#include "skse64/ScaleformMovie.h"
#include "skse64_common/BranchTrampoline.h"
#include "skse64_common/SafeWrite.h"
#include "skse64/GameForms.h"
#include "skse64/GameReferences.h"
#include "skse64/GameCamera.h"
#include "skse64/GameMenus.h"
#include "skse64/GameThreads.h"
#include "skse64/NiNodes.h"
#include "skse64/NiObjects.h"
#include "xbyak/xbyak.h"
#include <shlobj.h>				// CSIDL_MYCODUMENTS
#include <stdlib.h>

#define _USE_MATH_DEFINES 1
#include <math.h>

#include "PatchUtils.h"

//#define RUNTIME_VERSION		0x01050270  // No More Runtime version, now it is universal dll

//#define LOGME	

#ifndef LOGME
#define _MESSAGE
#endif

// SKSE class with some types corrections
class ThirdPersonState_ : public TESCameraState
{
public:
	ThirdPersonState_();
	virtual ~ThirdPersonState_();
	virtual void Unk_09(void);
	virtual void Unk_0A(void);
	virtual void UpdateMode(bool weaponDrawn);

	PlayerInputHandler		inputHandler;				// 20
	NiNode					* cameraNode;				// 30
	NiNode					* controllerNode;			// 38
	float					unk40[4];					// 40
	UInt32					unk50[3];					// 50
	float					fOverShoulderPosX;			// 5C
	float					fOverShoulderCombatAddY;	// 60
	float					fOverShoulderPosZ;			// 64
	float					unk68[3];					// 68
	float					unk74[6];					// 74  // This would be 54 in Skyrim32
	float					unk8C;						// 8C - init'd 7F7FFFFF   This is 6C in Skyrim32
	UInt32					unk90[3];					// 90
	float					unk9C;						// 9C - init'd 7F7FFFFF
	UInt64					unkA0;						// A0
	UInt64					unkA8;						// A8
	float					unkB0;						// B0
	UInt32					unkB4[3];					// B4
	float					unkC0[6];					// C0     C0[5] = AC in Skyrim32
	float					unkD8;						// D8    B0 in Skyrim32
	UInt8					unkDC[7];					// DC 
	UInt8					padE3[5];					// E3
};
STATIC_ASSERT(sizeof(ThirdPersonState_) == 0xE8);
STATIC_ASSERT(offsetof(ThirdPersonState_, unk74) == 0x74);
STATIC_ASSERT(offsetof(ThirdPersonState_, unkC0) == 0xC0);

class FurnitureCameraState_ : public TESCameraState
{
public:
	FurnitureCameraState_();
	virtual ~FurnitureCameraState_();

	float	unk20;	// 20
	float	unk24;	// 24
	float	unk28;	// 28
	float	unk2C;	// 2C
	UInt32	unk30;	// 30
	UInt32	unk34;	// 34
	UInt32	unk38;	// 38
	UInt8	unk3C;	// 3C
	UInt8	unk3D;	// 3D
	UInt8	unk3E;	// 3E
	UInt8	pad3F;	// 3F
};
STATIC_ASSERT(offsetof(FurnitureCameraState_, unk20) == 0x20);

class MovementControllerNPC
{
public:

	virtual ~MovementControllerNPC();

	// void *vtbl  - 00
	UInt8 unk008[0x1C0 - 8]; 
	bool unk1C0; // E4 in Skyrim32

	// ... More shit here
};
STATIC_ASSERT(offsetof(MovementControllerNPC, unk1C0) == 0x1C0);

struct RefEntry
{
	UInt32 id;
	BSHandleRefObject *handleRef;
};
STATIC_ASSERT(sizeof(RefEntry) == 0x10);

enum Transformation
{
	TRANSFORM_NONE,
	TRANSFORM_WEREWOLF,
	TRANSFORM_VAMPIRELORD
};

IDebugLog	gLog;

BOOL bEnableFirstPersonBody;
BOOL bEnableFirstPersonBodyConsole;
BOOL bEnableHeadBobPosition;
BOOL bEnableHeadBobPositionSprint;
BOOL bEnableHeadBobPositionCombat;
BOOL bEnableHeadBobPositionBow;
BOOL bEnableHeadBobPositionBowAim;
BOOL bEnableHeadBobPositionCombatSprint;
BOOL bEnableHeadBobPositionSneak;
BOOL bEnableHeadBobPositionSneakCombat;
BOOL bEnableHeadBobPositionSneakBow;
BOOL bEnableHeadBobPositionSneakBowAim;
BOOL bEnableHeadBobPositionSneakRoll;
BOOL bEnableHeadBobPositionHorse;
BOOL bEnableHeadBobPositionHorseCombat;
BOOL bEnableHeadBobPositionDragon;
BOOL bEnableHeadBobPositionDragonCombat;
BOOL bEnableHeadBobPositionWerewolf;
BOOL bEnableHeadBobPositionVampireLord;
BOOL bEnableHeadBobPositionScripted;
BOOL bFirstPersonShadows;
BOOL bAdjustPlayerScale;
BOOL bSmoothAnimationTransitions;
BOOL bEquipWeaponAnimFix;
BOOL bHide2HWeaponFirstPerson;
BOOL bHideBowFirstPerson;
BOOL bHideQuiverFirstPerson;
BOOL bHideBodySitting;
BOOL bHideBodySleeping;
BOOL bHideBodyJumping;
BOOL bHideBodySwimming;
BOOL bHideBodySneakRoll;
BOOL bHideBodyAttack;
BOOL bHideBodyPowerAttack;
BOOL bHideBodyAttackBow;
BOOL bHideBodyKillmove;
BOOL bFixTorchWhenSitting;
BOOL bFixReadingElderScroll;
BOOL bFixSkyrimIntro;
BOOL bFixHorseMountCamera1st;
BOOL bFixHorseMountCamera3rd;
BOOL bCraftingResetCamera1st;
BOOL bCraftingResetCamera3rd;
BOOL bSwitchPOVMatchCameraRotX;
BOOL bSwitchPOVMatchCameraRotZ;
BOOL bFixCamKillmovePlayerDeath;
BOOL bScriptedIdleAnimMatchHeadRotation;
BOOL bFixWerewolfTransformation;
BOOL bWerewolfTransformDetectWeaponOut;
BOOL bWerewolfCameraRotateIdleAnim;
BOOL bWerewolfCameraRotateRestrained;
BOOL bFix360Animations;
BOOL bEnableFOVOverride;
BOOL bEnableNearDistanceOverride;
BOOL bFirstPersonSitting;
BOOL bFirstPersonCrafting;
BOOL bFirstPersonHorse;
BOOL bFirstPersonHorseTransition;
BOOL bFirstPersonDragon;
BOOL bFirstPersonDragonTransition;
BOOL bFirstPersonWerewolf;
BOOL bFirstPersonVampireLord;
BOOL bFirstPersonTransform;
BOOL bFirstPersonKillmove;
BOOL bFirstPersonKillmoveBow;
BOOL bFirstPersonKnockout;
BOOL bFirstPersonDeath;
BOOL bFirstPersonCannibal;
BOOL bFirstPersonVampireFeed;
BOOL bFirstPersonIdleAnim;
BOOL bFirstPersonPairedAnim;
BOOL bFirstPersonRestrained;
BOOL bFirstPersonDontMove;
BOOL bFirstPersonScripted;
BOOL bForceFirstPersonCamera;
BOOL bForceFirstPersonSitting;
BOOL bEnableHeadFirstPerson;
BOOL bEnableHeadFirstPersonHorse;
BOOL bEnableHeadFirstPersonDragon;
BOOL bEnableHeadFirstPersonWerewolf;
BOOL bEnableHeadFirstPersonVampireLord;
BOOL bEnableHeadFirstPersonScripted;
BOOL bUseThirdPersonArms;
BOOL bUseThirdPersonArmsBow;
BOOL bUseThirdPersonArmsBowAim;
BOOL bFixEnchantmentArt;

float fMountedSwitchPOVDetectDistance;
float fDetect360AnimDegrees;
float fFirstPersonWerewolfKillmoveChance;
float fFirstPersonVampireLordKillmoveChance;
float fFirstPersonFOV;
float fFirstPersonFOVCombat;
float fFirstPersonFOVSitting;
float fFirstPersonFOVCrafting;
float fFirstPersonFOVHorse;
float fFirstPersonFOVHorseCombat;
float fFirstPersonFOVHorseTransition;
float fFirstPersonFOVDragon;
float fFirstPersonFOVDragonCombat;
float fFirstPersonFOVDragonTransition;
float fFirstPersonFOVKnockout;
float fFirstPersonFOVDeath;
float fFirstPersonFOVWerewolf;
float fFirstPersonFOVVampireLord;
float fFirstPersonFOVCannibal;
float fFirstPersonFOVVampireFeed;
float fFirstPersonFOVScripted;
float fThirdPersonFOV;
float fNearDistanceFirstPerson;
float fNearDistanceThirdPerson;
float fNearDistanceSitting;
float fNearDistanceCrafting;
float fNearDistanceHorse;
float fNearDistanceHorseCombat;
float fNearDistanceHorseTransition;
float fNearDistanceDragon;
float fNearDistanceDragonCombat;
float fNearDistanceDragonTransition;
float fNearDistanceKillmove;
float fNearDistanceKillmoveBow;
float fNearDistanceKnockout;
float fNearDistanceDeath;
float fNearDistanceWerewolf;
float fNearDistanceWerewolfKillmove;
float fNearDistanceVampireLord;
float fNearDistanceVampireLordKillmove;
float fNearDistanceCannibal;
float fNearDistanceVampireFeed;
float fNearDistanceRestrained;
float fNearDistanceDontMove;
float fNearDistanceIdleAnim;
float fNearDistancePairedAnim;
float fNearDistanceScripted;
float fControllerBufferDepth1stOverride;
float fControllerBufferDepth3rdOverride;
float fSittingMaxLookingDownOverride;
float fMountedMaxLookingDownOverride;
float fFlyingMountedMaxLookingDownOverride;
float fMountedRestrictAngle;
float fFlyingMountedRestrictAngle;
float fWerewolfRestrictAngle;
float fScriptedRestrictAngle;
float fCameraPosX;
float fCameraPosY;
float fCameraPosZ;
float fHorseCameraPosX;
float fHorseCameraPosY;
float fHorseCameraPosZ;
float fHorseCombatCameraPosX;
float fHorseCombatCameraPosY;
float fHorseCombatCameraPosZ;
float fDragonCameraPosX;
float fDragonCameraPosY;
float fDragonCameraPosZ;
float fDragonCombatCameraPosX;
float fDragonCombatCameraPosY;
float fDragonCombatCameraPosZ;
float fWerewolfCameraPosX;
float fWerewolfCameraPosY;
float fWerewolfCameraPosZ;
float fVampireLordCameraPosX;
float fVampireLordCameraPosY;
float fVampireLordCameraPosZ;
float fScriptedCameraPosX;
float fScriptedCameraPosY;
float fScriptedCameraPosZ;
float fCameraHeightOffset;
float fHeadBobRotation;
float fHeadBobRotationSprint;
float fHeadBobRotationCombat;
float fHeadBobRotationBow;
float fHeadBobRotationBowAim;
float fHeadBobRotationCombatSprint;
float fHeadBobRotationSneak;
float fHeadBobRotationSneakCombat;
float fHeadBobRotationSneakBow;
float fHeadBobRotationSneakBowAim;
float fHeadBobRotationSneakRoll;
float fHeadBobRotationHorse;
float fHeadBobRotationHorseCombat;
float fHeadBobRotationDragon;
float fHeadBobRotationDragonCombat;
float fHeadBobRotationWerewolf;
float fHeadBobRotationVampireLord;
float fHeadBobRotationScripted;

float gf_var1;
float gf_var20;

bool g_inDialogueMenu;
int g_isThirdPerson; 
int gi_var4;  // crafting?
int g_transform;  
int g_inCanibal;
int g_restrained;
int g_inDontMove;
int gi_var9;
int gi_var10;
int gi_var11;
int gi_var12;
int gi_var13;
int gi_var14;
int gi_var15;
int g_inKillMoveTransform;
int g_inPairedAnim;
int g_inKillmovePlayerDeath;
int gi_var19;
int gi_var21;
int g_savedCameraState;
int g_previousCameraState;
int gi_var24;
int gi_var25;
NiPoint3 saved_pos_3rd;

typedef int (*_GetEquippedItemType)(void *, UInt32, Actor *, int);
typedef bool(*_GameFunc2)(BaseExtraList *, UInt32);
typedef bool(*_GameFunc5)(void *, UInt32 *);
typedef void(*_ForceFirstPerson)();
typedef void(*_ForceThirdPerson)();
typedef float(*_UtilityRandomFloat)(void *, void *, void *, float, float);
typedef bool(*_IsJumping)(Actor *actor);
typedef bool(*_IsAttacking)(Actor *actor);
typedef bool(*_IsPowerAttacking)(Actor *actor);
typedef void(*_UpdateCamera)(NiCamera *);
typedef UInt32 * (*_GetFurnitureHandle)(ActorProcessManager *, UInt32 *);
typedef bool(*_IsActivateControlsEnabled)();
typedef void (*_BSFixedString_Copy)(BSFixedString *, BSFixedString *);
typedef bool(*_TorchFunc)(ActorWeightData *, int);
typedef NiNode *(*_GetNiNode)(TESObjectREFR *);
typedef void(*Func15Type)(void *);
typedef bool(*Func16Type)(void *);
typedef bool(*Func17Type)(void *);
typedef bool(*Func23Type)();

//// Patches signatures (search start addresses are based on 1.5.39)
RelocAddrEx<uintptr_t> Patch1_Address(0x6A1F7F, "E8 XXXXXXXX - 48 8B 9C 24 90 00 00 00 - 45 84 E4 - 4C 8B A4 24 A0 00 00 00 - 74 08");
RelocAddrEx<uintptr_t> Patch2_Address(0x004F602F + 0x15, "48 8B 4F 28 - 48 85 C9 - 74 06 - 48 8B 01 - FF 50 10", -0x15); // .Net Script Framework workaround
//RelocAddrEx<uintptr_t> Patch2_Address(0x4F602F, "48 8D 54 24 40 - FF 50 18 - 83 CE FF - 48 8B 5C 24 40 - 48 85 DB");
RelocAddrEx<uintptr_t> Patch3_Address(0x6A5427, "E8 XXXXXXXX - 48 8B 03 - 48 8B CB - FF 90 A8 02 00 00 - 48 8B CB");
RelocAddrEx<uintptr_t> Patch4_Address(0x26F3DC, "FF 90 68 04 00 00 - 4C 8B E8 - 48 89 44 24 48 - 48 85 C0 - 0F 84 XXXXXXXX");
RelocAddrEx<uintptr_t> Patch5_Address(0x69F4F3, "FF 90 80 03 00 00 - 48 8B C8 - 41 B1 01 - 45 0F B6 C1 - 48 8B D3");
RelocAddrEx<uintptr_t> Patch5_Address2(0x69F506, "E8 XXXXXXXX - 48 8B 8F F0 00 00 00 - E8 XXXXXXXX - 48 85 C0 - 74 1A - 48 8B 8F F0 00 00 00");
RelocAddrEx<uintptr_t> Patch6_Address(0x627437, "E8 XXXXXXXX - 48 85 DB - 74 23 - 4D 8B CE - 4D 8B C7 - 41 0F 28 CA");
RelocAddrEx<uintptr_t> Patch7_Address(0x6FC21A, "FF 90 88 03 00 00 - 84 C0 - 0F 84 XXXXXXXX - F3 0F 10 35 XXXXXXXX");
RelocAddrEx<uintptr_t> Patch8_Address(0x6FCE32, "48 03 D3 - F3 0F 5C 42 08 - 0F 2F 05 XXXXXXXX - 76 70 - ", 8); // 6FCE3A
RelocAddrEx<uintptr_t> Patch9_Address(0x1C9499, "E8 XXXXXXXX - 48 8B 05 XXXXXXXX - B2 01 - 48 8B 88 F0 00 00 00 - E8 XXXXXXXX - 48 85 C0");
RelocAddrEx<uintptr_t> Patch10_Address(0x6A1F74, "E8 XXXXXXXX - 48 8B D3 - 48 8B CE - E8 XXXXXXXX - 48 8B 9C 24 90 00 00 00");
RelocAddrEx<uintptr_t> Patch11_Address(0xADBF80, "4C 8B DC - 49 89 73 18 - 4D 89 73 20 - 41 57 - 48 81 EC 80 00 00 00 - 4C 8B 0A");
//RelocAddr<uintptr_t> Patch12_Address(0x5B1DE3);
//RelocAddr<uintptr_t> Patch12_Address(0x5B1F02);
//RelocAddr<uintptr_t> Patch12_Address(0x5B1FC4);
//RelocAddr<uintptr_t> Patch12_Address(0x5B1FD6);


//RelocAddrEx<uintptr_t> Patch12_Address(0x12FA2F5, "45 33 C0 - 33 D2 - 48 8B 0D XXXXXXXX- E8 XXXXXXXX - E8 XXXXXXXX - 48 8B 0D XXXXXXXX - ", 0x0C); // 12FA301  
RelocAddrEx<uintptr_t> Patch12_Address(0x5B1FE7, "41 B8 01 00 00 00 - 48 8B D6 - 48 8D 0D XXXXXXXX - E8 XXXXXXXX - E8 XXXXXXXX", 0x10);

//RelocAddr<uintptr_t> Patch12_Address(0x5B1CDD);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA142);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA091);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA09D);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA855);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA919);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA766);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA740);
//RelocAddr<uintptr_t> Patch13_Address(0x12FA931);
//RelocAddr<uintptr_t> Patch13B_Address(0x12FA931);
//RelocAddr<uintptr_t> Patch13_Address(0x5B20AC);
//RelocAddr<uintptr_t> Patch13_Address(0x12FB2BC);
//RelocAddr<uintptr_t> Patch13_Address(0x12FB282);

RelocAddrEx<uintptr_t> Patch14_Address(0x5D432C, "8B 0F - 89 48 6C - 8B 4F 04 - 89 48 70");
RelocAddrEx<uintptr_t> Patch15_Address(0x55876F, "E8 XXXXXXXX - 40 84 FF - 74 09 - 83 8E D0 00 00 00 02");
RelocAddrEx<uintptr_t> Patch16_Address(0x5584B6, "E8 XXXXXXXX - 84 C0 - 75 07 - B0 01 - E9 XXXXXXXX");
RelocAddrEx<uintptr_t> Patch17_Address(0x55F601, "E8 XXXXXXXX - 84 C0 - 0F 84 XXXXXXXX - 41 0F 28 C2 - F3 0F 58 86 2C 01 00 00");
RelocAddrEx<uintptr_t> Patch18_Address(0x55F6AA, "FF 90 D8 01 00 00 - 8B 86 30 01 00 00 - C1 E8 05 - A8 01");
RelocAddrEx<uintptr_t> Patch19_Address(0x5D4C35, "E8 XXXXXXXX - 48 8B 5C 24 58 - 48 8B 74 24 60 - 0F 28 74 24 30 - 48 83 C4 40");
RelocAddrEx<uintptr_t> Patch20_Address(0x6B6B1C, "E8 XXXXXXXX - 48 8B 6C 24 70 - 48 8B 74 24 78 - 0F 28 74 24 40");
RelocAddrEx<uintptr_t> Patch21_Address(0x67D9AC, "F3 41 0F 10 5E 04 - F3 41 0F 10 16 - 48 8B D3 - 48 8B 0D XXXXXXXX - E8 XXXXXXXX", 0x15); // 67D9C1
RelocAddrEx<uintptr_t> Patch22_Address(0x67DCCD, "E8 XXXXXXXX - F3 41 0F 10 06 - F3 0F 10 15 XXXXXXXX - F3 0F 59 C2");
RelocAddrEx<uintptr_t> Patch23_Address(0x67D991, "E8 XXXXXXXX - 84 C0 - 74 31 - F3 0F 11 74 24 28 - F3 41 0F 10 46 08");
RelocAddrEx<uintptr_t> Patch24_Address(0x67D7E0, "41 B1 01 - 45 0F B6 C1 - 33 D2 - E8 XXXXXXXX - 81 A7 C0 00 00 00 FF FF FF F7");
RelocAddrEx<uintptr_t> Patch24_Address2(0x67D7E9, "E8 XXXXXXXX - 81 A7 C0 00 00 00 FF FF FF F7 - 81 8F C0 00 00 00 00 00 00 06");
RelocAddrEx<uintptr_t> Patch25_Address(0x6A1D62, "41 0F B6 87 F4 00 00 00 - 41 0F B6 8E F4 00 00 00 - 24 01 - 80 E1 01");
RelocAddrEx<uintptr_t> Patch25_Address2(0x6A1DF1, "74 XX - 41 83 8E F4 00 00 00 01 - EB XX - 41 83 A7 F4 00 00 00 FE");
RelocAddrEx<uintptr_t> Patch26_Address(0x542414, "40 0F B6 D6 - 48 8B D8 - 4C 8B 01 - 41 FF 90 78 03 00 00"); 
RelocAddrEx<uintptr_t> Patch27_Address(0x550651, "FF 90 B8 08 00 00 - 48 8B 4C 24 60 - 48 8B 81 F0 01 00 00 - 48 85 C0");
RelocAddrEx<uintptr_t> Patch28_Address(0x551476, "FF 90 B8 08 00 00 - 45 0F 57 D2 - 41 0F 2F F2 - 77 0B");

RelocAddrEx<_GetEquippedItemType> GetEquippedItemType(0x94B380, "48 83 EC 38 - 4C 8B D1 - 49 8B C0 - 49 8B 88 F0 00 00 00 - 48 85 C9");
RelocAddrEx<_GameFunc2> GameFunc2(0x1077D0, "4C 8B 49 08 - 44 8B C2 - 4D 85 C9 - 74 21 - 8B C2");
RelocAddrEx<_GameFunc5> GameFunc5(0x4EDD70, "40 55 56 57 41 56 41 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 68 - 4C 8B FA - 4C 8B F1");
RelocAddrEx<_ForceFirstPerson> ForceFirstPerson(0x979940, "48 8B 0D XXXXXXXX - E9 XXXXXXXX", 8, "40 53 - 48 83 EC 20 - 8B 05 XXXXXXXX - 48 8B D9 - 39 41 3C - 75 26");
RelocAddrEx<_ForceThirdPerson> ForceThirdPerson(0x979950, "48 8B 0D XXXXXXXX - E9 XXXXXXXX", 8, "40 57 - 48 83 EC 20 - 48 8B F9 - 48 8B 49 28 - 48 85 C9 - 74 06 - 83 79 18 09 - 73 29 - 48 89 5C 24 30");
RelocAddrEx<_UtilityRandomFloat> UtilityRandomFloat(0x9B8C20, "48 83 EC 48 - 0F 29 74 24 30 - F3 0F 10 74 24 70 - 0F 2F DE - 0F 29 7C 24 20");
RelocAddrEx<_IsJumping> IsJumping(0x5D1AD0, "40 53 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 32 DB - 48 C7 44 24 48 00 00 00 00");
RelocAddrEx<_IsAttacking> IsAttacking(0x627950, "F7 81 C0 00 00 00 00 00 00 F0 - 0F 95 C0 - C3");
RelocAddrEx<_IsPowerAttacking> IsPowerAttacking(0x6279B0, "48 83 EC 28 - F7 81 C0 00 00 00 00 00 00 F0 - 74 33 - 48 8B 89 F0 00 00 00");
RelocAddrEx<_UpdateCamera> UpdateCamera(0xC66770, "48 8B C4 - 48 81 EC B8 00 00 00 - 80 B9 68 01 00 00 00 - F3 0F 10 91 A0 00 00 00");
RelocAddrEx<_GetFurnitureHandle> GetFurnitureHandle(0x67F3A0, "48 8B 41 08 - 48 85 C0 - 74 0E - 48 05 08 02 00 00"); // Look in 1.5.62
RelocAddrEx<_IsActivateControlsEnabled> IsActivateControlsEnabled(0x979980, "48 8B 0D XXXXXXXX - E9 XXXXXXXX", 8, " 48 8B 91 A0 01 00 00 - 48 85 D2 - 74 09 - 33 C0 - 38 42 1B");
RelocAddrEx<_BSFixedString_Copy> BSFixedString_Copy(0xC28CE0, "4C 8B 02 - 4C 8B C9 - 4D 85 C0 - 74 2D - 41 8B 40 F0");  // Note: there are two functions in the game where this signature would land. Anyway, they are both completely identical from beginning to end, so not a problem.
RelocAddrEx<_TorchFunc> TorchFunc(0x1C9950, "40 57 - 48 83 EC 30 - 48 C7 44 24 20 FE FF FF FF - 48 89 5C 24 - 48 48 89 6C 24 50 - 48 89 74 24 58 - 0F B6 F2");
RelocAddrEx<_GetNiNode> GetNiNode(0x291A10, "8B 15 XXXXXXXX - 65 48 8B 04 25 58 00 00 00 - 48 8B 04 D0 BA F8 05 00 00");
RelocAddrEx<Func15Type> Patch15Orig(0x5593D0, "40 56 - 48 83 EC 20 - F6 81 D0 00 00 00 01 - 48 8B F1");
RelocAddrEx<Func16Type> Patch16Orig(0x559470, "40 53 55 56 57 41 55 41 56 41 57 - 48 83 EC 60 - 48 C7 44 24 40 FE FF FF FF - 0F 29 74 24 50");
RelocAddrEx<Func17Type> Patch17Orig(0x561130, "40 53 57 41 56 - 48 83 EC 20 - 8B 81 30 01 00 00");
RelocAddrEx<Func23Type> Patch23Orig(0x63FCC0, "40 53 - 48 83 EC 20 - BB 01 00 00 00 - 8B CB");

RelocPtrEx<float> fDefaultWorldFov(0x3177B9, "F3 0F 11 35 XXXXXXXX - 0F 28 B4 24 80 00 00 00 - F3 0F 11 3D XXXXXXXX", 4, true); // 1E2CF00
RelocPtrEx<float> fNearDistance(0x3B3612, "F3 0F 5C 0D XXXXXXXX - F3 0F 5C D0 - F3 0F 5E D1 - 0F 28 CC", 4, true); // 1E2CDE0
RelocPtrEx<float> fMinCurrentZoom(0x84FC60, "F3 0F 10 05 XXXXXXXX - F3 0F 11 41 78 - 84 D2 - 75 06", 4, true); // 1E19808
RelocPtrEx<UInt32> GameVar1(0x6EA785, "44 0F 45 05 XXXXXXXX -  44 89 84 24 F8 00 00 00 - 83 3D XXXXXXXX 02", 4, true); // 2F4DEF4
RelocPtrEx<void *> GameVar2(0x69D8E5, "C1 E8 03 - A8 01 - 0F 85 XXXXXXXX - 48 8B 1D XXXXXXXX", 0xE, true); // 2F26990  Note: Two addreses match this signature, but they both will resolve the same var address
RelocPtrEx<BGSKeyword> GameVar3(0x219D81, "BA 00 00 00 00 - 48 0F 45 15 XXXXXXXX - 48 8D 8F 90 00 00 00", 9, true, 0x10); // 1EE4CD8
RelocPtrEx<float> fMountedMaxLookingDown(0x5EE26B, "C1 E9 05 - 83 E1 07 - 83 E9 03 - 83 F9 02 - 77 10 - F3 0F 10 35 XXXXXXXX", 0x12, true); // 0x1DFFE20
RelocPtrEx<float> fFlyingMountedMaxLookingDown(0x5EE2A5, "74 1A - F3 0F 10 35 XXXXXXXX - F3 0F 10 3D XXXXXXXX - F3 0F 59 3D XXXXXXXX", 6, true); // 1DFFE38
RelocPtrEx<RefEntry> RefArray(0x136457, "48 C1 E0 04 - 48 8D 0D XXXXXXXX - 48 03 C1 8B 08 - 0F BA E1 1A", 7, true); // 1EEB7C0  Note: three addresses match this signature, but they will resolve the same var address
RelocPtrEx<bool> Journal_TabsDisabled(0x2FB6AF, "83 7D 27 00 - 0F 45 C7 - 88 05 XXXXXXXX - 40 0F B6 C7", 9, true); // 2F761C4
RelocPtrEx<const char *> NiNodePtr(0xD41948, "C7 81 F0 00 00 00 00 00 80 3F - FF 50 10 - 48 85 C0 - 74 XX - 48 8D 0D XXXXXXXX", 0x15, true); // 30393E0
RelocPtrEx<const char *> BSFadeNodePtr(0x1D3C74, "48 8B 4B 10 - 48 85 C9 - 74 XX - 48 8B 01 - FF 50 10 - 48 85 C0 - 74 XX - 48 8D 0D XXXXXXXX", 0x17, true); // 31F6418
RelocPtrEx<const char *> BSFlattenedBoneTreePtr(0xD41A53, "FF 50 10 - 48 8D 0D XXXXXXXX -  48 3B C1 - 75 XX - 41 B0 01 - 48 8B D7", 6, true); // 303A618
RelocPtrEx<float> ControllerVar1(0x153144, "F3 0F 10 05 XXXXXXXX - 48 8D 54 24 30 - 48 8B 89 98 00 00 00", 4, true); // 1EE5B54
RelocPtrEx<float> fSittingMaxLookingDown(0x5EE2C7, "C1 E8 0E - 83 E0 0F - FF C8 - 83 F8 03 - 77 XX - F3 0F 10 35 XXXXXXXX", 0x11, true); // 1DFFE50
////


#define GET_BOOL(var, def) var = GetPrivateProfileInt("Main", #var, def, ini_path);
#define GET_FLOAT(var, def) { GetPrivateProfileString("Main", #var, def, buf, sizeof(buf)-1, ini_path); var = atof(buf); }
#define GET_ANGLE(var, def) { GET_FLOAT(var, def); var = (var * M_PI) / 180.0; }

void ReadConfig()
{
	static char ini_path[MAX_PATH];
	static char buf[256]; 
	
	GetModuleFileName(nullptr, ini_path, sizeof(ini_path)); 
	char *slash = strrchr(ini_path, '\\'); 
	if (slash)
	{
		*slash = 0; 
		strcat_s(ini_path, sizeof(ini_path), "\\Data\\SKSE\\Plugins\\ImprovedCamera.ini");		
	}

	GET_BOOL(bEnableFirstPersonBody, TRUE);
	GET_BOOL(bEnableFirstPersonBodyConsole, FALSE);
	GET_BOOL(bEnableHeadBobPosition, TRUE);
	GET_BOOL(bEnableHeadBobPositionSprint, TRUE);
	GET_BOOL(bEnableHeadBobPositionCombat, TRUE);
	GET_BOOL(bEnableHeadBobPositionBow, TRUE);
	GET_BOOL(bEnableHeadBobPositionBowAim, TRUE);
	GET_BOOL(bEnableHeadBobPositionCombatSprint, TRUE);
	GET_BOOL(bEnableHeadBobPositionSneak, TRUE);
	GET_BOOL(bEnableHeadBobPositionSneakCombat, TRUE);
	GET_BOOL(bEnableHeadBobPositionSneakBow, TRUE);
	GET_BOOL(bEnableHeadBobPositionSneakBowAim, TRUE);
	GET_BOOL(bEnableHeadBobPositionSneakRoll, FALSE);
	GET_BOOL(bEnableHeadBobPositionHorse, TRUE);
	GET_BOOL(bEnableHeadBobPositionHorseCombat, TRUE);
	GET_BOOL(bEnableHeadBobPositionDragon, TRUE);
	GET_BOOL(bEnableHeadBobPositionDragonCombat, TRUE);
	GET_BOOL(bEnableHeadBobPositionWerewolf, TRUE);
	GET_BOOL(bEnableHeadBobPositionVampireLord, TRUE);
	GET_BOOL(bEnableHeadBobPositionScripted, TRUE);
	GET_BOOL(bFirstPersonShadows, TRUE);
	GET_BOOL(bAdjustPlayerScale, TRUE);
	GET_BOOL(bSmoothAnimationTransitions, TRUE);
	GET_BOOL(bEquipWeaponAnimFix, TRUE);
	GET_BOOL(bHide2HWeaponFirstPerson, FALSE);
	GET_BOOL(bHideBowFirstPerson, FALSE);
	GET_BOOL(bHideQuiverFirstPerson, FALSE);
	GET_BOOL(bHideBodySitting, FALSE);
	GET_BOOL(bHideBodySleeping, FALSE);
	GET_BOOL(bHideBodyJumping, FALSE);
	GET_BOOL(bHideBodySwimming, FALSE);
	GET_BOOL(bHideBodySneakRoll, TRUE);
	GET_BOOL(bHideBodyAttack, FALSE);
	GET_BOOL(bHideBodyPowerAttack, FALSE);
	GET_BOOL(bHideBodyAttackBow, FALSE);
	GET_BOOL(bHideBodyKillmove, FALSE);
	GET_BOOL(bFixTorchWhenSitting, TRUE);
	GET_BOOL(bFixReadingElderScroll, TRUE);
	GET_BOOL(bFixSkyrimIntro, TRUE);
	GET_BOOL(bFixHorseMountCamera1st, TRUE);
	GET_BOOL(bFixHorseMountCamera3rd, FALSE);
	GET_BOOL(bCraftingResetCamera1st, TRUE);
	GET_BOOL(bCraftingResetCamera3rd, FALSE);
	GET_BOOL(bSwitchPOVMatchCameraRotX, TRUE);
	GET_BOOL(bSwitchPOVMatchCameraRotZ, FALSE);
	GET_BOOL(bFixCamKillmovePlayerDeath, TRUE);
	GET_BOOL(bScriptedIdleAnimMatchHeadRotation, TRUE);
	GET_BOOL(bFixWerewolfTransformation, TRUE);
	GET_BOOL(bWerewolfTransformDetectWeaponOut, TRUE);
	GET_BOOL(bWerewolfCameraRotateIdleAnim, TRUE);
	GET_BOOL(bWerewolfCameraRotateRestrained, TRUE);
	GET_BOOL(bFix360Animations, FALSE);
	GET_BOOL(bEnableFOVOverride, TRUE);
	GET_BOOL(bEnableNearDistanceOverride, TRUE);
	GET_BOOL(bFirstPersonSitting, TRUE);
	GET_BOOL(bFirstPersonCrafting, TRUE);
	GET_BOOL(bFirstPersonHorse, TRUE);
	GET_BOOL(bFirstPersonHorseTransition, TRUE);
	GET_BOOL(bFirstPersonDragon, TRUE);
	GET_BOOL(bFirstPersonDragonTransition, TRUE);
	GET_BOOL(bFirstPersonWerewolf, TRUE);
	GET_BOOL(bFirstPersonVampireLord, TRUE);
	GET_BOOL(bFirstPersonTransform, TRUE);
	GET_BOOL(bFirstPersonKillmove, TRUE);
	GET_BOOL(bFirstPersonKillmoveBow, FALSE);
	GET_BOOL(bFirstPersonKnockout, TRUE);
	GET_BOOL(bFirstPersonDeath, TRUE);
	GET_BOOL(bFirstPersonCannibal, TRUE);
	GET_BOOL(bFirstPersonVampireFeed, TRUE);
	GET_BOOL(bFirstPersonIdleAnim, TRUE);
	GET_BOOL(bFirstPersonPairedAnim, TRUE);
	GET_BOOL(bFirstPersonRestrained, TRUE);
	GET_BOOL(bFirstPersonDontMove, TRUE);
	GET_BOOL(bFirstPersonScripted, TRUE);
	GET_BOOL(bForceFirstPersonCamera, FALSE);
	GET_BOOL(bForceFirstPersonSitting, FALSE);
	GET_BOOL(bEnableHeadFirstPerson, TRUE);
	GET_BOOL(bEnableHeadFirstPersonHorse, TRUE);
	GET_BOOL(bEnableHeadFirstPersonDragon, TRUE);
	GET_BOOL(bEnableHeadFirstPersonWerewolf, TRUE);
	GET_BOOL(bEnableHeadFirstPersonVampireLord, TRUE);
	GET_BOOL(bEnableHeadFirstPersonScripted, FALSE);
	GET_BOOL(bUseThirdPersonArms, TRUE);
	GET_BOOL(bUseThirdPersonArmsBow, TRUE);
	GET_BOOL(bUseThirdPersonArmsBowAim, FALSE);
	GET_BOOL(bFixEnchantmentArt, TRUE);

	GET_FLOAT(fMountedSwitchPOVDetectDistance, "0.03");
	GET_ANGLE(fDetect360AnimDegrees, "30.0");
	GET_FLOAT(fFirstPersonWerewolfKillmoveChance, "33.0");
	GET_FLOAT(fFirstPersonVampireLordKillmoveChance, "33.0");
	GET_FLOAT(fFirstPersonFOV, "90.0");
	GET_FLOAT(fFirstPersonFOVCombat, "90.0");
	GET_FLOAT(fFirstPersonFOVSitting, "90.0");
	GET_FLOAT(fFirstPersonFOVCrafting, "90.0");
	GET_FLOAT(fFirstPersonFOVHorse, "90.0");
	GET_FLOAT(fFirstPersonFOVHorseCombat, "90.0");
	GET_FLOAT(fFirstPersonFOVHorseTransition, "90.0");
	GET_FLOAT(fFirstPersonFOVDragon, "90.0");
	GET_FLOAT(fFirstPersonFOVDragonCombat, "90.0");
	GET_FLOAT(fFirstPersonFOVDragonTransition, "90.0");
	GET_FLOAT(fFirstPersonFOVKnockout, "90.0");
	GET_FLOAT(fFirstPersonFOVDeath, "90.0");
	GET_FLOAT(fFirstPersonFOVWerewolf, "90.0");
	GET_FLOAT(fFirstPersonFOVVampireLord, "90.0");
	GET_FLOAT(fFirstPersonFOVCannibal, "90.0");
	GET_FLOAT(fFirstPersonFOVVampireFeed, "90.0");
	GET_FLOAT(fFirstPersonFOVScripted, "90.0");
	GET_FLOAT(fThirdPersonFOV, "90.0");	
	GET_FLOAT(fNearDistanceFirstPerson, "10.0");
	GET_FLOAT(fNearDistanceThirdPerson, "15.0");
	GET_FLOAT(fNearDistanceSitting, "10.0");
	GET_FLOAT(fNearDistanceCrafting, "2.0");
	GET_FLOAT(fNearDistanceHorse, "10.0");
	GET_FLOAT(fNearDistanceHorseCombat, "10.0");
	GET_FLOAT(fNearDistanceHorseTransition, "4.0");
	GET_FLOAT(fNearDistanceDragon, "10.0");
	GET_FLOAT(fNearDistanceDragonCombat, "10.0");
	GET_FLOAT(fNearDistanceDragonTransition, "4.0");
	GET_FLOAT(fNearDistanceKillmove, "10.0");
	GET_FLOAT(fNearDistanceKillmoveBow, "10.0");
	GET_FLOAT(fNearDistanceKnockout, "2.0");
	GET_FLOAT(fNearDistanceDeath, "2.0");
	GET_FLOAT(fNearDistanceWerewolf, "10.0");
	GET_FLOAT(fNearDistanceWerewolfKillmove, "2.0");
	GET_FLOAT(fNearDistanceVampireLord, "10.0");
	GET_FLOAT(fNearDistanceVampireLordKillmove, "2.0");
	GET_FLOAT(fNearDistanceCannibal, "10.0");
	GET_FLOAT(fNearDistanceVampireFeed, "10.0");
	GET_FLOAT(fNearDistanceRestrained, "2.0");
	GET_FLOAT(fNearDistanceDontMove, "2.0");
	GET_FLOAT(fNearDistanceIdleAnim, "2.0");
	GET_FLOAT(fNearDistancePairedAnim, "2.0");
	GET_FLOAT(fNearDistanceScripted, "2.0");
	GET_FLOAT(fControllerBufferDepth1stOverride, "0.01");
	GET_FLOAT(fControllerBufferDepth3rdOverride, "0.14");
	GET_FLOAT(fSittingMaxLookingDownOverride, "70");
	GET_FLOAT(fMountedMaxLookingDownOverride, "70");
	GET_FLOAT(fFlyingMountedMaxLookingDownOverride, "70");
	GET_ANGLE(fMountedRestrictAngle, "70");
	GET_ANGLE(fFlyingMountedRestrictAngle, "70");
	GET_ANGLE(fWerewolfRestrictAngle, "70");
	GET_ANGLE(fScriptedRestrictAngle, "70");
	GET_FLOAT(fCameraPosX, "0.0");
	GET_FLOAT(fCameraPosY, "20.0");
	GET_FLOAT(fCameraPosZ, "4.0");
	GET_FLOAT(fHorseCameraPosX, "0.0");
	GET_FLOAT(fHorseCameraPosY, "15.0");
	GET_FLOAT(fHorseCameraPosZ, "7.0");
	GET_FLOAT(fHorseCombatCameraPosX, "0.0");
	GET_FLOAT(fHorseCombatCameraPosY, "15.0");
	GET_FLOAT(fHorseCombatCameraPosZ, "7.0");
	GET_FLOAT(fDragonCameraPosX, "0.0");
	GET_FLOAT(fDragonCameraPosY, "15.0");
	GET_FLOAT(fDragonCameraPosZ, "7.0");
	GET_FLOAT(fDragonCombatCameraPosX, "0.0");
	GET_FLOAT(fDragonCombatCameraPosY, "15.0");
	GET_FLOAT(fDragonCombatCameraPosZ, "7.0");
	GET_FLOAT(fWerewolfCameraPosX, "1.0");
	GET_FLOAT(fWerewolfCameraPosY, "26.0");
	GET_FLOAT(fWerewolfCameraPosZ, "1.0");
	GET_FLOAT(fVampireLordCameraPosX, "1.0");
	GET_FLOAT(fVampireLordCameraPosY, "18.0");
	GET_FLOAT(fVampireLordCameraPosZ, "1.0");
	GET_FLOAT(fScriptedCameraPosX, "0.0");
	GET_FLOAT(fScriptedCameraPosY, "8.0");
	GET_FLOAT(fScriptedCameraPosZ, "3.0");
	GET_FLOAT(fCameraHeightOffset, "0");
	GET_FLOAT(fHeadBobRotation, "0");
	GET_FLOAT(fHeadBobRotationSprint, "0");
	GET_FLOAT(fHeadBobRotationCombat, "0");
	GET_FLOAT(fHeadBobRotationBow, "0");
	GET_FLOAT(fHeadBobRotationBowAim, "0");
	GET_FLOAT(fHeadBobRotationCombatSprint, "0");
	GET_FLOAT(fHeadBobRotationSneak, "0");
	GET_FLOAT(fHeadBobRotationSneakCombat, "0");
	GET_FLOAT(fHeadBobRotationSneakBow, "0");
	GET_FLOAT(fHeadBobRotationSneakBowAim, "0");
	GET_FLOAT(fHeadBobRotationSneakRoll, "0");
	GET_FLOAT(fHeadBobRotationHorse, "0");
	GET_FLOAT(fHeadBobRotationHorseCombat, "0");
	GET_FLOAT(fHeadBobRotationDragon, "0");
	GET_FLOAT(fHeadBobRotationDragonCombat, "0");
	GET_FLOAT(fHeadBobRotationWerewolf, "0");
	GET_FLOAT(fHeadBobRotationVampireLord, "0");
	GET_FLOAT(fHeadBobRotationScripted, "0");

	if (bUseThirdPersonArms)
	{
		bHideBodyAttack = FALSE;
		bHideBodyPowerAttack = FALSE;
		bFixTorchWhenSitting = FALSE;
	}

	if (bUseThirdPersonArmsBow)
	{
		bHideBodyAttackBow = FALSE;
	}
}

typedef void (* Func1Type)(PlayerCharacter *, void *);
typedef void (* Func2Type)(void *, void *);
typedef void (* Func5Type)(NiNode *, void *, bool, bool);
typedef void (* Func6Type)(Actor *, float, void *);
typedef void (* Func9Type)(Actor *);
typedef void (* Func11Type)(void *, void *, void *);
typedef void * (* Func19Type)(BSTaskPool *, Actor *, float, void *, UInt8, UInt8);
typedef void * (* Func20Type)(Actor *, void *, float, UInt8, UInt8);
typedef void * (* Func21Type)(BSTaskPool *, Actor *, float, float, float, float);
typedef void (* Func24Type)(NiAVObject *, void *, UInt32, UInt32);
typedef void *(* Func12Type)(void *, void *, void *, void *);
typedef void *(*Func13Type)(void *, void *, void *, void *);

Func1Type Patch1Orig;
Func5Type Patch5Orig;
Func6Type Patch6Orig;
Func9Type Patch9Orig;
Func11Type Patch11Orig;
Func19Type Patch19Orig;
Func20Type Patch20Orig;
Func21Type Patch21Orig;
Func24Type Patch24Orig;
Func12Type Patch12Orig;
Func13Type Patch13Orig;
Func12Type Patch12OrigB;
Func13Type Patch13OrigB;

static inline void *GetVtblAddress(void *obj, UInt32 address)
{
	UInt64 *vtbl = (UInt64 *) *(UInt64 *)obj;
	return (void *)(vtbl[address / 8]);
}

/*static inline UInt64 GetField64(void *obj, UInt32 address)
{
	return ((UInt64 *)obj)[address / 8];
}*/

static inline int EquippedItemType(Actor *actor)
{
	return GetEquippedItemType(nullptr, 0, actor, 0);
}

static inline bool IsFighting(Actor *actor)
{
	return ((actor->actorState.flags08 & 0xE0) != 0);
}

static inline bool IsRiding(Actor *actor)
{
	return ((actor->actorState.flags04 & 0x3C000) == 0x0C000);
}

static inline bool IsUnknown3C000(Actor *actor)
{
	return ((actor->actorState.flags04 & 0x3C000) != 0);
}

static inline bool IsKnockout(Actor *actor)
{
	return ((actor->actorState.flags04 & 0x0E000000) != 0);
}

// May be "IsRestrained"
static inline bool IsUnknown1E00000_0C00000(Actor *actor)
{
	return ((actor->actorState.flags04 & 0x1E00000) == 0xC00000);
}

// May be is "IsDontMove"
static inline bool IsUnknown1E00000_1200000(Actor *actor)
{
	return ((actor->actorState.flags04 & 0x1E00000) == 0x1200000);
}

static inline bool IsUnknown3C000_1C000(Actor *actor)
{
	return ((actor->actorState.flags04 & 0x3C000) == 0x1C000);
}

static inline bool IsSitting(Actor *actor)
{
	UInt32 flags = (actor->actorState.flags04 >> 14) & 0xF;
	return !(flags >= 4 || flags == 0);
}

static inline bool IsSleeping(Actor *actor)
{
	UInt32 flags = (actor->actorState.flags04 >> 14) & 0xF;
	return !(flags <= 5 || flags >= 9);
}

static inline bool IsBowAiming(Actor *actor)
{
	UInt32 flags = actor->actorState.flags04 >> 28;
	return (flags >= 8 && flags <= 13);
}

static inline bool IsKillMove(Actor *actor)
{
	return (((actor->flags2 >> 14) & 1) != 0);
}

static inline bool IsUnknown138_1(Actor *actor)
{
	return (((actor->flags2 >> 1) & 1) != 0);
}

static inline bool IsMenuOpen(MenuManager *mm, UIStringHolder *uistr, BSFixedString *str)
{
	return (mm && uistr && mm->IsMenuOpen(str));
}

static inline float RandomFloat(float min, float max)
{
	return UtilityRandomFloat(nullptr, nullptr, nullptr, min, max);
}

static inline bool EvaluateProbability(float prob)
{
	return (prob < RandomFloat(0.0f, 100.0f));
}

static inline bool PlayerControlsFunc1()
{
	PlayerControls *pc = PlayerControls::GetSingleton();
	return (pc && (pc->unk04C >> 8));
}

static inline bool PlayerControlsFunc2()
{
	PlayerControls *pc = PlayerControls::GetSingleton();
	return (pc && (pc->unk04C & 0xFF));
}

static inline bool PlayerControlsFunc3()
{
	PlayerControls *pc = PlayerControls::GetSingleton();
	if (!pc)
		return false;

	UInt8 *unk = (UInt8 *)(pc->unk058.entries[12]);
	return (unk[0x18] != 0);  // 0x18 here = 0x0C of Skyrim32  (reference access in Skyrim32: 77172C)
}

bool IsFurniture(TESObjectREFR *ref)
{
	TESForm *base = ref->baseForm;
	if (!base)
		return false;

	return (base->formType == kFormType_Furniture);
}

inline bool DoGameFunc5()
{
	UInt32 val = *GameVar1;
	return GameFunc5(*GameVar2, &val);
}

bool UseThirdPersonArms()
{
	PlayerCharacter *player = (*g_thePlayer);
	
	int equippedType = EquippedItemType(player);
	bool bow = (equippedType == 7 || equippedType == 12); // Bow / Crossbow  cl
	bool aiming = IsBowAiming(player); // al

	if (bUseThirdPersonArms)
	{
		if (!bow)
			return true;
	}
	else
	{
		if (!bow)
			return false;
	}

	if (bUseThirdPersonArmsBow && !aiming)
		return true;

	else if (bUseThirdPersonArmsBowAim && aiming)
		return true;
	
	return false;
}

// May be something close to "IsRiding", but what is the difference?
static inline bool SFunc2_3(Actor *actor)
{
	return (((actor->flags2 >> 1) & 1) ? false : GameFunc2(&actor->extraData, 0xA9));
}

static inline bool IsIdlePlaying()
{
	PlayerCharacter *player = (*g_thePlayer);
	MovementControllerNPC *mc = (MovementControllerNPC *)player->unk148;
	return (!mc->unk1C0);
}

TESFurniture *GetPlayerFurniture()
{
	PlayerCharacter *player = (*g_thePlayer);
	ActorProcessManager	* processManager = player->processManager;

	if (!processManager)
		return nullptr;

	UInt32 handle;
	GetFurnitureHandle(processManager, &handle);

	if (!handle)
		return nullptr;

	RefEntry *arr = RefArray.GetPtr();
	UInt8 *handleRef = (UInt8 *)(arr[handle & 0xFFFFF].handleRef);
	if (!handleRef)
		return nullptr;

	TESObjectREFR *ref = (TESObjectREFR *)(handleRef - offsetof(TESObjectREFR, handleRefObject));

	if (!IsFurniture(ref))
		return nullptr;
	
	return (TESFurniture *)ref->baseForm;
}

bool HasEnchantingKeyword(BGSKeywordForm *kwf)
{
	if (kwf->numKeywords <= 0)
		return false;

	BGSKeyword **keywords = kwf->keywords;

	for (UInt32 i = 0; i < kwf->numKeywords; i++)
	{
		BGSKeyword *kw = *keywords;
		const char *kw_str = (kw->keyword.c_str()) ? kw->keyword.c_str() : "";

		if (strcmp(kw_str, "isEnchanting") == 0)
			return true;

		keywords++;
	}
	
	return false;
}

bool IsVampireLord(Actor *actor)
{
	const char *race = actor->race->editorId.c_str();	
	return (race && strcmp(race, "DLC1VampireBeastRace") == 0);
}

bool SFunc2_8()
{
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	if (camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
		return false;

	InputManager *input = InputManager::GetSingleton();
	UInt8 x = ~input->unk118;
	UInt8 y = ~(input->unk118 >> 1);

	if (!(x & 1))
		return false;

	if (!(y & 1))
		return false;

	return true;
}

bool ShouldFixSkyrimIntro()
{
	if (!bFixSkyrimIntro)
		return false;

	if (*Journal_TabsDisabled)
		return true;

	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	InputManager *input = InputManager::GetSingleton();

	if (CALL_MEMBER_FN(player, GetLevel)() != 1)
		return false;

	if (camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
		return false;

	UInt32 val = input->unk118; 
	UInt8 x = ~(val >> 7) & 1;  // var_2
	UInt8 y = ~(val >> 8) & 1; // var_3
	UInt8 z = ~(val >> 6) & 1; // bl
	bool u = PlayerControlsFunc2(); // var_1
	bool ac_enabled = IsActivateControlsEnabled(); 

	if (!z || !x || !y || !u || ac_enabled || !(*Journal_TabsDisabled))
		return false;
	
	return true;
}

static inline bool IsState(ActorState *state, UInt32 flags)
{
	return ((state->flags04 & flags & 0x3FFF) == flags);
}

bool IsSwimming(Actor *actor)
{
	return IsState(&actor->actorState, ActorState::kState_Swimming);
}

bool IsSneaking(Actor *actor)
{
	ActorState *state = &actor->actorState;		
	return (IsState(state, ActorState::kState_Sneaking) && !IsState(state, ActorState::kState_Swimming) && !SFunc2_3(actor));
}

bool IsSneakRolling(Actor *actor) 
{
	return (IsSneaking(actor) && IsState(&actor->actorState, ActorState::kState_Sprinting));
}

bool IsSprinting(Actor *actor)
{
	return (IsState(&actor->actorState, ActorState::kState_Sprinting));
}

NiNode *FindNode(NiNode *root, const char *name)
{
	if (!root)
		return nullptr;

	if (root->m_name && strcmp(root->m_name, name) == 0)
		return root;

	// L5
	typedef const char **(* _NiNodeV10)(NiNode *);
	_NiNodeV10 NiNodeV10 = (_NiNodeV10)GetVtblAddress(root, 0x10);

	const char **ret = NiNodeV10(root);
	if (ret != NiNodePtr.GetPtr() && ret != BSFadeNodePtr.GetPtr() && ret != BSFlattenedBoneTreePtr.GetPtr())
		return nullptr;

	UInt16 num = root->m_children.m_emptyRunStart;
	for (UInt16 i = 0; i < num; i++)
	{
		NiNode *ret = FindNode((NiNode *)root->m_children.m_data[i], name);
		if (ret)
			return ret;
	}
	
	return nullptr;
}

void MatrixToEuler(const NiMatrix33 *m, float *x, float *y, float *z) 
{
	if (m->data[0][2] < +1) {
		if (m->data[0][2] > -1) {
			*y = asin(m->data[0][2]);
			*x = atan2(-m->data[1][2], m->data[2][2]);
			*z = atan2(-m->data[0][1], m->data[0][0]);
		}
		else {
			*y = -M_PI / 2;
			*x = -atan2(m->data[1][0], m->data[1][1]);
			*z = 0;
		}
	}
	else {
		*y = +M_PI / 2;
		*x = atan2(m->data[1][0], m->data[1][1]);
		*z = 0;
	}
}

void EulerToMatrix(NiMatrix33 * m, float x, float y, float z) {
	m->data[0][0] = cos(y)*cos(z);
	m->data[0][1] = -cos(y)*sin(z);
	m->data[0][2] = sin(y);
	m->data[1][0] = cos(z)*sin(x)*sin(y) + cos(x)*sin(z);
	m->data[1][1] = cos(x)*cos(z) - sin(x)*sin(y)*sin(z);
	m->data[1][2] = -cos(y)*sin(x);
	m->data[2][0] = -cos(x)*cos(z)*sin(y) + sin(x)*sin(z);
	m->data[2][1] = cos(z)*sin(x) + cos(x)*sin(y)*sin(z);
	m->data[2][2] = cos(x)*cos(y);
}

void ScalePoint(NiPoint3 *point, float scale)
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	
	if (PlayerControlsFunc2())
	{
		point->x = fScriptedCameraPosX * scale;
		point->y = fScriptedCameraPosY * scale;
		point->z = fScriptedCameraPosZ * scale;
	}
	else if (g_transform == TRANSFORM_VAMPIRELORD)
	{
		point->x = fVampireLordCameraPosX * scale;
		point->y = fVampireLordCameraPosY * scale;
		point->z = fVampireLordCameraPosZ * scale;
	}
	else if (g_transform == TRANSFORM_WEREWOLF)
	{
		point->x = fWerewolfCameraPosX * scale;
		point->y = fWerewolfCameraPosY * scale;
		point->z = fWerewolfCameraPosZ * scale;
	}
	else if (SFunc2_3(player))
	{
		if (camera->cameraState->stateId == PlayerCamera::kCameraState_Horse)
		{
			if (IsFighting(player))
			{
				point->x = fHorseCombatCameraPosX * scale;
				point->y = fHorseCombatCameraPosY * scale;
				point->z = fHorseCombatCameraPosZ * scale;
			}
			else
			{
				point->x = fHorseCameraPosX * scale;
				point->y = fHorseCameraPosY * scale;
				point->z = fHorseCameraPosZ * scale;				
			}
		}
		else if (camera->cameraState->stateId == PlayerCamera::kCameraState_Dragon)
		{
			if (IsFighting(player))
			{
				point->x = fDragonCombatCameraPosX * scale;
				point->y = fDragonCombatCameraPosY * scale;
				point->z = fDragonCombatCameraPosZ * scale;
			}
			else
			{
				point->x = fDragonCameraPosX * scale;
				point->y = fDragonCameraPosY * scale;
				point->z = fDragonCameraPosZ * scale;
			}
		}
	}
	else
	{
		point->x = fCameraPosX * scale;
		point->y = fCameraPosY * scale;
		point->z = fCameraPosZ * scale;
	}
}

bool ShouldFixElderScrollRead()
{
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	
	if (!bFixReadingElderScroll || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson || !IsIdlePlaying())
		return false;

	InputManager *input = InputManager::GetSingleton();
	UInt32 val = input->unk118;
	UInt8 x = ~val & 1; // var_5
	UInt8 y = ~(val >> 6) & 1; // var_4
	UInt8 z = ~(val >> 1) & 1; // bl
	UInt8 v = ~(val >> 8) & 1; // var_3
	UInt8 w = ~(val >> 7) & 1; // var_2
	bool u = PlayerControlsFunc2(); // var_1
	bool ac_enabled = IsActivateControlsEnabled(); // !al

	if (!x)
	{
		// 1000242E
		if (z)
			return false;

		if (y)
		{
			if (w || v || u || ac_enabled)
				return false;
		}

		else if (w || !v || u || !ac_enabled)
			return false;

		return !(*Journal_TabsDisabled);
	}

	// L5
	if (!z)
	{
		// 10002459
		if (!y || !w || !v || !u || ac_enabled || !(*Journal_TabsDisabled))
			return false;

		return true;
	}

	// L6
	if (!y || w || !v || u || ac_enabled || (*Journal_TabsDisabled))
		return false;

	return true;
}

bool SFunc2_10_2_1(Actor *actor, BSFixedString *name)
{
	BSFixedString copy;
	uintptr_t unk = 0;

	BSFixedString_Copy(&copy, name);	
	
	IAnimationGraphManagerHolder *animGraphHolder = &actor->animGraphHolder;
	typedef bool(*_AnimGraphV90)(IAnimationGraphManagerHolder *, BSFixedString *, uintptr_t *);
	_AnimGraphV90 AnimGraphV90 = (_AnimGraphV90)GetVtblAddress(animGraphHolder, 0x90);
	
	bool ret = AnimGraphV90(animGraphHolder, &copy, &unk);	
	name->Release();

	if (!ret)
		return false;
	
	return (unk != 0);
}

bool IsRunningAnimation(Actor *actor, const char *str)
{
	IAnimationGraphManagerHolder *animGraphHolder = &actor->animGraphHolder;
	
	typedef bool(*_AnimGraphV10)(IAnimationGraphManagerHolder *, uintptr_t *);
	_AnimGraphV10 AnimGraphV10 = (_AnimGraphV10)GetVtblAddress(animGraphHolder, 0x10);

	uintptr_t unk = 0;	
	BSFixedString name(str);

	if (!AnimGraphV10(animGraphHolder, &unk))
		return false;

	return SFunc2_10_2_1(actor, &name);
}

inline bool IsEquippingWeapon(Actor *actor)
{
	return (IsRunningAnimation(actor, "IsEquipping") || IsRunningAnimation(actor, "IsUnequipping"));
}

bool IsTorchOut(Actor *actor)
{
	// Most of this func functionality seems to have been copied from the game IsTorchOut_cmd
	typedef ActorWeightModel *(*_GetWeightModel)(Actor *);
	_GetWeightModel GetWeightModel = (_GetWeightModel)GetVtblAddress(actor, 0x3F8);

	ActorWeightModel *model = GetWeightModel(actor);
	ActorWeightData *weightData = model->weightData;

	bool torch_out = false;

	if (weightData)
	{
		InterlockedIncrement(&weightData->refCount);

		torch_out = TorchFunc(weightData, 1);
		if (!torch_out)
			torch_out = TorchFunc(weightData, 0);

		if (InterlockedDecrement(&weightData->refCount) == 0)
		{
			CALL_MEMBER_FN(weightData, DeleteThis)();
			Heap_Free(weightData);
		}
	}

	return torch_out;
}

void UpdateArms()
{
	PlayerCharacter *player = (*g_thePlayer);
	NiNode *root1st = player->GetNiRootNode(1);

	NiNode *lclavicle = FindNode(root1st, "NPC L Clavicle [LClv]"); // var_C
	NiNode *rclavicle = FindNode(root1st, "NPC R Clavicle [RClv]"); // var_8

	if (!lclavicle || !rclavicle)
		return;

	PlayerCamera *camera = PlayerCamera::GetSingleton();

	UInt32 flags = 0; // ebx
	bool show_left = false;
	bool show_right = false;

	if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson || gi_var24)
	{
		// 10002B64
		if (!PlayerControlsFunc1())
		{
			if (!UseThirdPersonArms() || SFunc2_8() || ShouldFixElderScrollRead())
			{
				// 10002B96
				if ((player->actorState.flags08 & 0xE0) >= 0x60)
				{
					show_left = show_right = true;
				}
				else if (bEquipWeaponAnimFix && IsEquippingWeapon(player)) // Skipped duplicate bEquipWeaponAnimFix check
				{
					show_left = show_right = true;
				}
				else if (SFunc2_8())
				{
					show_left = show_right = true;
				}
				else if (ShouldFixElderScrollRead())
				{
					show_left = show_right = true;
				}
				else if (IsTorchOut(player))
				{
					if (bFixTorchWhenSitting)
					{
						if (!IsUnknown3C000(player))
						{
							show_left = true;
						}
					}
					else
					{
						show_left = true;
					}
				}
			}

			// Nothing here
		}

		// Nothing here
	}

	// L5
	if (show_left)
	{
		lclavicle->m_localTransform.scale = 1.0;
	}
	else
	{
		lclavicle->m_localTransform.scale = 0.001;
	}

	if (show_right)
	{
		rclavicle->m_localTransform.scale = 1.0;
	}
	else
	{
		rclavicle->m_localTransform.scale = 0.001;
	}
}

NiNode *GetHeadNode(NiNode *root)
{
#define NUM_NODES 29

	static UInt32 idx = 0;  // static var notice
	static const char *head_nodes[NUM_NODES] =
	{
		"NPC Head [Head]",  // 0
		"NPC Head",
		"Bip01 Head",
		"Boar_Reikling_Head",
		"Boar_Head",
		"Canine_Head",
		"ChaurusFlyerHead",
		"DragPriestNPC Head [Head]",
		"DwarvenSpiderHead_XYZ",
		"ElkScull",
		"FireAtronach_Head [Head]", // 10
		"Goat_Head",
		"HEAD",
		"Head [Head]",
		"Horker_Head01",
		"HorseScull",
		"IW Head",
		"MainBody",
		"Mammoth Head",
		"Mcrab_Body",
		"NetchPelvis [Pelv]", // 20
		"NPC M HeadNoodle01",
		"RabbitHead",
		"Sabrecat_Head [Head]",
		"Scull",
		"SlaughterfishHead",
		"Torso Main",
		"Wisp Head",
		"NPC UpperLid", // 28
	};
	
	NiNode *node = FindNode(root, head_nodes[idx]);
	if (node)
	{
		return node;
	}

	for (UInt32 i = 0; i < NUM_NODES; i++)
	{
		if (i != idx)
		{
			node = FindNode(root, head_nodes[i]);
			if (node)
			{
				idx = i;				
				return node;
			}
		}
	}

	return root;
}

float GetHeadRotation()
{
	float ret;

	if (PlayerControlsFunc2())
	{
		ret = fHeadBobRotationScripted;
	}
	else if (g_transform == TRANSFORM_VAMPIRELORD)
	{
		ret = fHeadBobRotationVampireLord;
	}
	else if (g_transform == TRANSFORM_WEREWOLF)
	{
		ret = fHeadBobRotationWerewolf;
	}
	else
	{
		PlayerCharacter *player = (*g_thePlayer);
		PlayerCamera *camera = PlayerCamera::GetSingleton();
		
		if (SFunc2_3(player))
		{
			if (camera->cameraState->stateId == PlayerCamera::kCameraState_Horse)
			{
				ret = IsFighting(player) ? fHeadBobRotationHorseCombat : fHeadBobRotationHorse;
			}
			else if (camera->cameraState->stateId == PlayerCamera::kCameraState_Dragon)
			{
				ret = IsFighting(player) ? fHeadBobRotationDragonCombat : fHeadBobRotationDragon;
			}
		}
		else if (IsSneaking(player)) // L1
		{
			if (IsSneakRolling(player))
			{
				ret = fHeadBobRotationSneakRoll;
			}
			else if (IsFighting(player))
			{
				if (EquippedItemType(player) == 7 || EquippedItemType(player) == 12) // Bow/crossbow
				{
					if (IsBowAiming(player))
					{
						ret = fHeadBobRotationSneakBowAim;
					}
					else
					{
						ret = fHeadBobRotationSneakBow;
					}
				}
				else
				{
					ret = fHeadBobRotationSneakCombat;
				}
			}
			else
			{
				ret = fHeadBobRotationSneak;
			}
		}
		else // L2
		{
			if (IsFighting(player))
			{
				if (IsSprinting(player))
				{
					ret = fHeadBobRotationCombatSprint;
				}
				else
				{
					if (EquippedItemType(player) == 7 || EquippedItemType(player) == 12) // Bow/crossbow
					{
						if (IsBowAiming(player))
						{
							ret = fHeadBobRotationBowAim;
						}
						else
						{
							ret = fHeadBobRotationBow;
						}
					}
					else
					{
						ret = fHeadBobRotationCombat;
					}
				}
			}
			else
			{
				ret = IsSprinting(player) ? fHeadBobRotationSprint : fHeadBobRotation;
			}
		}
	}
	
	return ret;
}

void RotMatrixToQuaternion(const NiMatrix33 *matrix, NiQuaternion *quat)
{
	float diag = matrix->data[0][0] + matrix->data[1][1] + matrix->data[2][2];

	if (diag > 0.0)
	{
		float s = sqrt(diag + 1.0);
		float d = 0.5 / s;

		quat->m_fW = 0.5 * s;
		quat->m_fX = (matrix->data[2][1] - matrix->data[1][2]) * d;
		quat->m_fY = (matrix->data[0][2] - matrix->data[2][0]) * d;
		quat->m_fZ = (matrix->data[1][0] - matrix->data[0][1]) * d;
	}
	else if ((matrix->data[0][0] >= matrix->data[1][1]) && (matrix->data[0][0] >= matrix->data[2][2]))
	{
		float s = sqrt(((1.0 + matrix->data[0][0]) - matrix->data[1][1]) - matrix->data[2][2]);
		float d = 0.5f / s;

		quat->m_fX = 0.5f * s;
		quat->m_fY = (matrix->data[1][0] + matrix->data[0][1]) * d;
		quat->m_fZ = (matrix->data[2][0] + matrix->data[0][2]) * d;
		quat->m_fW = (matrix->data[2][1] - matrix->data[1][2]) * d;
	}
	else if (matrix->data[1][1] > matrix->data[2][2])
	{
		float s = sqrt(((1.0 + matrix->data[1][1]) - matrix->data[0][0]) - matrix->data[2][2]);
		float d = 0.5f / s;

		quat->m_fX = (matrix->data[0][1] + matrix->data[1][0]) * d;
		quat->m_fY = 0.5f * s;
		quat->m_fZ = (matrix->data[1][2] + matrix->data[2][1]) * d;
		quat->m_fW = (matrix->data[0][2] - matrix->data[2][0]) * d;
	}
	else
	{
		float s = sqrt(((1.0 + matrix->data[2][2]) - matrix->data[0][0]) - matrix->data[1][1]);
		float d = 0.5f / s;

		quat->m_fX = (matrix->data[0][2] + matrix->data[2][0]) * d;
		quat->m_fY = (matrix->data[1][2] + matrix->data[2][1]) * d;
		quat->m_fZ = 0.5f * s;
		quat->m_fW = (matrix->data[1][0] - matrix->data[0][1]) * d;
	}
}

void SlerpQuat(NiQuaternion *ret, const NiQuaternion *quat1, const NiQuaternion *quat2, float amount)
{
	float sum = (((quat1->m_fX * quat2->m_fX) + (quat1->m_fY * quat2->m_fY)) + (quat1->m_fZ * quat2->m_fZ)) + (quat1->m_fW * quat2->m_fW);
	bool negative = false;

	float f1, f2;
	
	if (sum < 0.0f)
	{
		negative = true;
		sum = -sum;
	}
	if (sum > 0.999999f)
	{
		f2 = 1.0f - amount;
		f1 = negative ? -amount : amount;
	}
	else
	{
		float f3 = acos(sum);
		float f4 = 1.0 / sin(f3);

		f2 = sin(((1.0f - amount) * f3)) * f4;
		f1 = negative ? ((-sin((amount * f3))) * f4) : ((sin((amount * f3))) * f4);
	}

	ret->m_fX = (f2 * quat1->m_fX) + (f1 * quat2->m_fX);
	ret->m_fY = (f2 * quat1->m_fY) + (f1 * quat2->m_fY);
	ret->m_fZ = (f2 * quat1->m_fZ) + (f1 * quat2->m_fZ);
	ret->m_fW = (f2 * quat1->m_fW) + (f1 * quat2->m_fW);	
}

void QuaternionToMatrix(const NiQuaternion *quat, NiMatrix33 *matrix)
{
	float xx = quat->m_fX * quat->m_fX;
	float xy = quat->m_fX * quat->m_fY;
	float xz = quat->m_fX * quat->m_fZ;
	float xw = quat->m_fX * quat->m_fW;
	float yy = quat->m_fY * quat->m_fY;
	float yz = quat->m_fY * quat->m_fZ;
	float yw = quat->m_fY * quat->m_fW;
	float zz = quat->m_fZ * quat->m_fZ;
	float zw = quat->m_fZ * quat->m_fW;

	matrix->data[0][0] = 1.0 - (zz + yy) * 2.0;
	matrix->data[0][1] = (xy - zw) * 2.0;
	matrix->data[0][2] = (yw + xz) * 2.0;
	matrix->data[1][0] = (xy + zw) * 2.0;
	matrix->data[1][1] = 1.0 - (zz + xx) * 2.0;
	matrix->data[1][2] = (yz - xw) * 2.0;
	matrix->data[2][0] = (xz - yw) * 2.0;
	matrix->data[2][1] = (yz + xw) * 2.0;
	matrix->data[2][2] = 1.0 - (xx + yy) * 2.0;
}

void MatrixInverse(const NiMatrix33 *src, NiMatrix33 *dst)
{
	float f1 = (src->data[2][2] * src->data[1][1]) - (src->data[2][1] * src->data[1][2]);
	float f2 = src->data[2][2] * src->data[1][0];
	float f3 = src->data[1][2] * src->data[2][0];
	float f4 = (src->data[1][0] * src->data[2][1]) - (src->data[1][1] * src->data[2][0]);
	float det = f4 * src->data[0][2] + f1 * src->data[0][0] - (f2 - f3) * src->data[0][1];

	dst->data[0][0] = f1 / det;
	dst->data[0][1] = (src->data[2][1] * src->data[0][2] - src->data[2][2] * src->data[0][1]) / det;
	dst->data[0][2] = (src->data[1][2] * src->data[0][1] - src->data[1][1] * src->data[0][2]) / det;
	dst->data[1][0] = (f3 - f2) / det;
	dst->data[1][1] = (src->data[2][2] * src->data[0][0] - src->data[2][0] * src->data[0][2]) / det;
	dst->data[1][2] = (src->data[0][2] * src->data[1][0] - src->data[1][2] * src->data[0][0]) / det;
	dst->data[2][0] = f4 / det;
	dst->data[2][1] = (src->data[2][0] * src->data[0][1] - src->data[2][1] * src->data[0][0]) / det;
	dst->data[2][2] = (src->data[0][0] * src->data[1][1] - src->data[0][1] * src->data[1][0]) / det;
}

BOOL EnableHeadBobPosition()
{
	PlayerCharacter *player = (*g_thePlayer);
	
	if (IsSitting(player) && bHideBodySitting)
		return false;

	if (IsSneaking(player))
	{
		if (IsSneakRolling(player))
		{
			return bEnableHeadBobPositionSneakRoll;
		}
		else if (IsFighting(player))
		{
			if (EquippedItemType(player) == 7 || EquippedItemType(player) == 12) // bow/crossbow
			{
				if (IsBowAiming(player))
				{
					return bEnableHeadBobPositionSneakBowAim;
				}
				else
				{
					return bEnableHeadBobPositionSneakBow;
				}
			}
			else
			{
				return bEnableHeadBobPositionSneakCombat;
			}
		}
		else
		{
			return bEnableHeadBobPositionSneak;
		}
	}
	else // L1
	{		
		if (IsFighting(player))
		{
			if (IsSprinting(player))
			{
				return bEnableHeadBobPositionCombatSprint;
			}
			else
			{
				if (EquippedItemType(player) == 7 || EquippedItemType(player) == 12) // bow/crossbow
				{
					if (IsBowAiming(player))
					{
						return bEnableHeadBobPositionBowAim;
					}
					else
					{
						return bEnableHeadBobPositionBow;
					}
				}
				else
				{
					return bEnableHeadBobPositionCombat;
				}
			}
		}
		else
		{
			if (IsSprinting(player))
			{
				return bEnableHeadBobPositionSprint;
			}
			else
			{
				return bEnableHeadBobPosition;
			}
		}
	}
	
	return false;
}

void MatrixVectorMultiply(NiPoint3 *dst, const NiMatrix33 *matrix, const NiPoint3 *vec)
{
	dst->x = (matrix->data[0][0] * vec->x) + (matrix->data[0][1] * vec->y) + (matrix->data[0][2] * vec->z);
	dst->y = (matrix->data[1][0] * vec->x) + (matrix->data[1][1] * vec->y) + (matrix->data[1][2] * vec->z);
	dst->z = (matrix->data[2][0] * vec->x) + (matrix->data[2][1] * vec->y) + (matrix->data[2][2] * vec->z);
}

static inline bool IsLoadingMenuOpen()
{
	UIStringHolder *uistr = UIStringHolder::GetSingleton();
	return IsMenuOpen(MenuManager::GetSingleton(), uistr, &uistr->loadingMenu);
}

// UpdateSwitchPOV
void Patch1(PlayerCharacter *pthis, void *param1)
{
	Patch1Orig(pthis, param1);

	PlayerCamera *camera = PlayerCamera::GetSingleton();
	PlayerCharacter *player = (*g_thePlayer);
	bool isFirstPerson = (camera->cameraState->stateId == 0);

	if (bSwitchPOVMatchCameraRotZ && isFirstPerson)
	{
		player->rot.z += gf_var1;
	}

	NiNode *root1st = player->GetNiRootNode(1);
	NiNode *root3rd = player->GetNiRootNode(0);	

	if (!bEnableFirstPersonBody)
		return;

	bool doFirstPerson = false;

	MenuManager *menu = MenuManager::GetSingleton();
	UIStringHolder *uistr = UIStringHolder::GetSingleton();

	if (isFirstPerson && !UseThirdPersonArms() && !IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
	{
		doFirstPerson = true;
	}

	if (doFirstPerson)
	{
		// Show both
		root1st->m_flags &= ~1;
		root3rd->m_flags &= ~1;			
	}
	else
	{
		// Show only third
		root1st->m_flags |= 1;
		root3rd->m_flags &= ~1;
	}	
}

// UpdateCamera
void Patch2(void *obj, void *param)
{
	Func2Type Func2Orig = (Func2Type) GetVtblAddress(obj, 0x18);
	Func2Orig(obj, param);

	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	NiNode *cameraNode = camera->cameraNode;
	ThirdPersonState_ *cameraState = (ThirdPersonState_ *)camera->cameraState;	
	NiNode *root1st = player->GetNiRootNode(1);
	NiNode *root3rd = player->GetNiRootNode(0);
	NiCamera *cameraNI = (NiCamera *)  ((cameraNode->m_children.m_size == 0) ? nullptr : cameraNode->m_children.m_data[0]);
	int horseCamState = PlayerCamera::kCameraState_Horse; 
	int dragonCamState = PlayerCamera::kCameraState_Dragon;

	MenuManager *menu = MenuManager::GetSingleton();
	UIStringHolder *uistr = UIStringHolder::GetSingleton();

	typedef bool(*_InCannibalAction)(Actor *);
	typedef bool(*_InVampireFeedAction)(Actor *);

	_InCannibalAction InCannibalAction = (_InCannibalAction)GetVtblAddress(player, 0x5E0);  
	_InVampireFeedAction InVampireFeedAction = (_InVampireFeedAction)GetVtblAddress(player, 0x5F0);  

	if (bEnableFOVOverride)
	{
		if (IsMenuOpen(menu, uistr, &uistr->dialogueMenu))
		{
			if (!g_inDialogueMenu)
				g_inDialogueMenu = true;
		}
		else
		{
			float fov;
			
			if (g_inDialogueMenu && cameraState->stateId != PlayerCamera::kCameraState_TweenMenu) 
			{
				if (g_isThirdPerson == 0)
				{
					fov = fThirdPersonFOV;
				}
				else if (PlayerControlsFunc2())
				{
					fov = fFirstPersonFOVScripted;
				}
				else if (PlayerControlsFunc1())
				{
					fov = fFirstPersonFOVWerewolf;
				}
				else if (SFunc2_3(player))
				{
					if (IsRiding(player))
					{
						if (IsFighting(player))
						{
							if (cameraState->stateId == horseCamState)
							{
								fov = fFirstPersonFOVHorseCombat;
							}
							else if (cameraState->stateId == dragonCamState)
							{
								fov = fFirstPersonFOVDragonCombat;
							}
						}
					}
					else
					{
						if (cameraState->stateId == horseCamState)
						{
							fov = fFirstPersonFOVHorse;
						}
						else if (cameraState->stateId == dragonCamState)
						{
							fov = fFirstPersonFOVDragon;
						}
					}
				}
				else
				{
					if (IsFighting(player))  
					{
						fov = fFirstPersonFOVCombat;
					}
					else
					{
						fov = fFirstPersonFOV;
					}
				}

				// L2
				if (camera->worldFOV > (fov - 1.0))
				{
					g_inDialogueMenu = false;
				}
			}
		}

		// L1
		if (cameraState->stateId != PlayerCamera::kCameraState_TweenMenu && !g_inDialogueMenu)
		{
			// 10004007
			float fov;

			if (g_isThirdPerson == 0)
			{
				fov = fThirdPersonFOV;
			}
			else
			{
				// 1000401B
				if (EquippedItemType(player) != 7) //bow
				{
					EquippedItemType(player); // ???
				}

				bool unk = SFunc2_3(player);

				if (unk)
				{
					// 10004067
					if (IsRiding(player))
					{
						if (IsFighting(player))  
						{
							if (cameraState->stateId == horseCamState)
							{
								fov = fFirstPersonFOVHorseCombat;
							}
							else if (cameraState->stateId == dragonCamState)
							{
								fov = fFirstPersonFOVDragonCombat;
							}
						}
						else
						{
							if (cameraState->stateId == horseCamState)
							{
								fov = fFirstPersonFOVHorse;
							}
							else if (cameraState->stateId == dragonCamState)
							{
								fov = fFirstPersonFOVDragon;
							}
						}
					}
					else
					{
						if (cameraState->stateId == horseCamState)
						{
							fov = fFirstPersonFOVHorseTransition;
						}
						else if (cameraState->stateId == dragonCamState)
						{
							fov = fFirstPersonFOVDragonTransition;
						}
					}
				}
				else
				{
					// 100040DB
					UInt32 flags = (player->actorState.flags04 >> 14) & 0xF;
					
					if (flags && !gi_var4 && flags != 3 && flags != 7)
					{
						fov = fFirstPersonFOVSitting;
					}
					else if (flags && gi_var4)
					{
						fov = fFirstPersonFOVCrafting;
					}
					else if (IsKnockout(player))
					{
						fov = fFirstPersonFOVKnockout;
					}
					else if (player->IsDead(1))
					{
						fov = fFirstPersonFOVDeath;
					}
					else if (PlayerControlsFunc2())
					{
						fov = fFirstPersonFOVScripted;
					}
					else if (g_transform == TRANSFORM_VAMPIRELORD)
					{
						fov = fFirstPersonFOVVampireLord;
					}
					else if (g_transform == TRANSFORM_WEREWOLF)
					{
						fov = fFirstPersonFOVWerewolf;
					}
					else
					{
						if (InCannibalAction(player) || g_inCanibal)
						{
							fov = fFirstPersonFOVCannibal;
						}
						else
						{
							// 10004199
							if (InVampireFeedAction(player))
							{
								fov = fFirstPersonFOVVampireFeed;
							}
							else if (IsFighting(player))  
							{
								fov = fFirstPersonFOVCombat;
							}
							else
							{
								fov = fFirstPersonFOV;
							}
						}
					}

				}
			}  // if else g_isThirdPerson

			// L3
			if (fov != camera->worldFOV)
			{
				camera->worldFOV = fov;
				*fDefaultWorldFov = fov;
			}
		}
		// Nothing else here
	} // bEnableFOVOverride

	if (bEnableNearDistanceOverride && cameraState->stateId != PlayerCamera::kCameraState_TweenMenu)
	{
		if (!IsMenuOpen(menu, uistr, &uistr->mapMenu))
		{
			if (g_isThirdPerson == 0)
			{
				if (cameraNI->m_frustum.m_fNear != fNearDistanceThirdPerson)
				{
					*fNearDistance = fNearDistanceThirdPerson;
					cameraNI->m_frustum.m_fNear = fNearDistanceThirdPerson;
				}
			}
			else
			{
				float fov;
				bool is_bow = (EquippedItemType(player) == 7 || EquippedItemType(player) == 12); // bow/crossbow	var_9			
				bool is_killcam = (cameraState->stateId == PlayerCamera::kCameraState_VATS); // bl
				bool unk = SFunc2_3(player);

				if (unk)
				{
					// 100042D7
					if (IsRiding(player))
					{
						if (IsFighting(player))
						{
							if (cameraState->stateId == horseCamState)
							{
								fov = fNearDistanceHorseCombat;
							}
							else if (cameraState->stateId == dragonCamState)
							{
								fov = fNearDistanceDragonCombat;
							}
						}
						else
						{
							if (cameraState->stateId == horseCamState)
							{
								fov = fNearDistanceHorse;
							}
							else if (cameraState->stateId == dragonCamState)
							{
								fov = fNearDistanceDragon;
							}
						}
					}
					else
					{
						if (cameraState->stateId == horseCamState)
						{
							fov = fNearDistanceHorseTransition;
						}
						else if (cameraState->stateId == dragonCamState)
						{
							fov = fNearDistanceDragonTransition;
						}
					}
				}
				else
				{
					// 10004387
					UInt32 flags = (player->actorState.flags04 >> 14) & 0xF;

					if (flags && !gi_var4 && flags != 3 && flags != 7)
					{
						fov = fNearDistanceSitting;
					}
					else if (flags && gi_var4)
					{
						fov = fNearDistanceCrafting;
					}
					else
					{
						UInt32 flag = IsKillMove(player);

						if ((flag || is_killcam) && !is_bow)
						{
							if (g_transform == TRANSFORM_VAMPIRELORD)
							{
								fov = fNearDistanceVampireLordKillmove;
							}
							else if (g_transform == TRANSFORM_WEREWOLF)
							{
								fov = fNearDistanceWerewolfKillmove;
							}
							else
							{
								fov = fNearDistanceKillmove;
							}
						}
						else if ((flag || is_killcam) && is_bow)
						{
							fov = fNearDistanceKillmoveBow;
						}
						else if (IsKnockout(player))
						{
							fov = fNearDistanceKnockout;
						}
						else if (player->IsDead(1))
						{
							fov = fNearDistanceDeath;
						}
						else if (DoGameFunc5())
						{
							fov = fNearDistancePairedAnim;
						}
						else if (IsIdlePlaying())
						{
							fov = fNearDistanceIdleAnim;
						}
						else if (PlayerControlsFunc2())
						{
							fov = fNearDistanceScripted;
						}
						else if (g_transform == TRANSFORM_VAMPIRELORD)
						{
							fov = fNearDistanceVampireLord;
						}
						else if (g_transform == TRANSFORM_WEREWOLF)
						{
							fov = fNearDistanceWerewolf;
						}
						else
						{
							if (InCannibalAction(player) || g_inCanibal)
							{
								fov = fNearDistanceCannibal;
							}
							else
							{
								if (InVampireFeedAction(player))
								{
									fov = fNearDistanceVampireFeed;
								}
								else if (g_restrained)
								{
									fov = fNearDistanceRestrained;
								}
								else if (g_inDontMove)
								{
									fov = fNearDistanceDontMove;
								}
								else
								{
									fov = fNearDistanceFirstPerson;									
								}
							}
						}
					}
				}

				// L4 = LL4 = LLL4 = LLLL4

				if (cameraNI->m_frustum.m_fNear != fov)
				{
					*fNearDistance = fov;
					cameraNI->m_frustum.m_fNear = fov;
				}
			}

			// Nothing here
		}
		// Nothing here

	} // bEnableNearDistanceOverride 

	
	if (g_isThirdPerson)
	{
		bool is_bow = (EquippedItemType(player) == 7 || EquippedItemType(player) == 12); // bow/crossbow   (bl reg)
		bool is_killcam = (cameraState->stateId == PlayerCamera::kCameraState_VATS); // var_9
		bool b33 = true;
		bool b2D = true;
		bool b3D = !gi_var10;
		bool untransformed = (g_transform == TRANSFORM_NONE);
		bool b34 = (!bFirstPersonScripted || !gi_var11);
		bool b32 = true;
		bool b37 = true;
		bool b2E = true;
		bool b30 = true;
		bool b38 = true;
		bool b14 = true;
		bool b35 = true;
		bool b2F = true;
		bool b36 = true;
		bool b31 = true;
		bool bal = true;
		bool bbl = true;
		
		if (bFirstPersonSitting && !gi_var4)
		{
			UInt32 state = (player->actorState.flags04 >> 14) & 0xF;

			if ((player->unkBDA & 6) || (state != 0 && state != 3 && state != 7))
			{
				b33 = false;
			}			
		}

		if (bFirstPersonCrafting && gi_var4)
		{
			if ((player->unkBDA & 6) || IsUnknown3C000(player))
				b2D = false;
		}

		PlayerControls *pc = PlayerControls::GetSingleton();

		bool cannibal_check = (InCannibalAction(player) || g_inCanibal);

		if (bFirstPersonIdleAnim && IsIdlePlaying() && !IsUnknown3C000(player) && !PlayerControlsFunc1() &&
			!PlayerControlsFunc2() && !InVampireFeedAction(player) && !cannibal_check)
		{			
			b32 = false;
		}	

		bool killcam_check = (IsKillMove(player) || is_killcam);

		if (bFirstPersonPairedAnim && DoGameFunc5() && !killcam_check)
		{
			b37 = false;
		}		

		if (bFirstPersonKillmove && !is_bow && killcam_check && !DoGameFunc5())
		{
			b2E = false;
		}

		if (bFirstPersonKillmoveBow && is_bow && killcam_check && !DoGameFunc5())
		{
			b30 = false;
		}

		if (bFirstPersonKnockout && IsKnockout(player))
		{
			b38 = false;
		}

		if (bFirstPersonDeath && player->IsDead(1))
		{
			b14 = false;
		}		

		if (bFirstPersonCannibal && cannibal_check)
		{
			b35 = false;
		}

		if (bFirstPersonVampireFeed && InVampireFeedAction(player))
		{
			b2F = false;
		}

		if (bFirstPersonRestrained && g_restrained && !PlayerControlsFunc1() && !PlayerControlsFunc2())
		{
			b36 = false;
		}

		if (bFirstPersonDontMove && g_inDontMove && !PlayerControlsFunc1() && !PlayerControlsFunc2())
		{
			b31 = false;
		}

		bool st_check = (!IsUnknown3C000(player) || bForceFirstPersonSitting);
		bool unk_check = (!IsIdlePlaying() || IsUnknown3C000(player));

		if (st_check && gi_var10 == 0 && gi_var11 == 0 && g_transform == TRANSFORM_NONE && !IsKillMove(player) && 
			!is_killcam && !IsKnockout(player) && !player->IsDead(1) && !InCannibalAction(player) && !g_inCanibal &&
			!InVampireFeedAction(player) && unk_check && !DoGameFunc5() && !IsUnknown1E00000_0C00000(player) &&
			!IsUnknown1E00000_1200000(player))
		{
			bal = false;
		}

		// L6_

		bool unk_check2 = (!bal || bForceFirstPersonCamera == 2);
		
		if (bForceFirstPersonCamera && unk_check2 && !PlayerControlsFunc3() && !gi_var12)
		{
			bbl = false;
		}

		if (cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && cameraState->stateId != PlayerCamera::kCameraState_TweenMenu && !IsMenuOpen(menu, uistr, &uistr->inventoryMenu) &&
			b33 && b2D && b3D && untransformed && b34 && b32 && b37 && b2E && b30 && b35 && b2F && b36 && b31 && b38 && b14 && bbl)
		{
			if (gi_var9 != 0)
			{
				gi_var9--;
			}
			else
			{
				g_isThirdPerson = 0; 

				if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
				{
					ThirdPersonState_ *state = (ThirdPersonState_ *)cameraState;

					if (*fMinCurrentZoom >= state->unk74[1])
					{
						state->unk74[1] = *fMinCurrentZoom + 0.009999999776482582;
						state->unk74[0] = state->unk74[4];
					}
				}

				// Nothing here
			}

			// Nothing here
		}
		else
		{
			gi_var9 = 3;
		}

		// Nothing here
	}
	else
	{
		// L5  
		if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson) 
		{
			g_isThirdPerson = 1;			
		}

		if (gi_var9 != 3)
		{
			gi_var9 = 3;
		}
	}

	// L10	
	
	if (g_inCanibal != 0)
	{
		if (g_inCanibal <= 1)
		{
			if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
			{
				g_inCanibal = 0;
			}
		}
		else 
		{
			g_inCanibal--;
		}

		// Nothing
	}
	else if (g_isThirdPerson != 0 && InCannibalAction(player))
	{
		g_inCanibal = 10;
	}

	// L11
	if (gi_var13 == 0)
	{
		bool flag = SFunc2_3(player); // al

		if (IsUnknown3C000(player) && !flag)
		{
			TESFurniture *furniture = GetPlayerFurniture();

			if (furniture)
			{
				BGSKeyword *kw = GameVar3.GetPtr();
								
				//if (KeyWordFunc(obj, kw))
				if (furniture->keyword.HasKeyword(kw))
				{
					bool ret = HasEnchantingKeyword(&furniture->keyword);
					gi_var4 = 0;

					if (ret)
						gi_var4 = 1;

				}
				else
				{
					gi_var4 = 1;
				}				
			}
			
			gi_var13 = 1;
		}

		// Nothing
	}
	else if (!IsUnknown3C000(player))
	{
		gi_var4 = 0;
		gi_var13 = 0;
	}

	// L12

	bool cf_20 = false, cf_17 = false;
	bool jumped_l15 = false;

	if (g_transform == TRANSFORM_NONE)
	{

L15:
		if (jumped_l15 || cameraState->stateId == PlayerCamera::kCameraState_FirstPerson || !g_isThirdPerson || !PlayerControlsFunc1())
		{
			// L15
			if (gi_var14 == 0)
			{
				cf_20 = true;
			}
			else if (gi_var14 != 1)
			{
				cf_17 = true;
			}
		}
		else
		{
			// 10004C9B
			g_transform = TRANSFORM_WEREWOLF;
			gi_var14 = 1;

			if (!bFirstPersonTransform)
			{
				ThirdPersonState_ *thirdState = (ThirdPersonState_ *)camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson2];

				if (thirdState)
				{
					float zoom = *fMinCurrentZoom;
					
					if (zoom >= thirdState->unk74[1])
					{
						thirdState->unk74[1] = *fMinCurrentZoom;

						if (thirdState->unk74[4] > zoom)
						{
							thirdState->unk74[0] = thirdState->unk74[4];
						}
						else
						{
							thirdState->unk74[4] = zoom + 0.009999999776482582;
							thirdState->unk74[0] = thirdState->unk74[4];
						}
					}
				}

				// Nothing here
			}

			// Nothing here
		}

		// L16

		if (!cf_20)
		{
			if (!cf_17)
			{
				if (IsIdlePlaying() || (IsFighting(player) && bWerewolfTransformDetectWeaponOut))
				{
					gi_var14 = 2;
				}				
			}
			else
			{
				// L17
				if (!IsIdlePlaying() || (IsFighting(player) && bWerewolfTransformDetectWeaponOut))
				{
					// 10004E17
					if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2) 
					{
						bool trCheck;
						ThirdPersonState_ *thirdState = (ThirdPersonState_ *)cameraState;

						if (IsVampireLord(player))
						{
							trCheck = (bFirstPersonVampireLord != 0);
						}
						else
						{
							trCheck = (bFirstPersonWerewolf != 0);
						}

						if (trCheck)
						{
							if (!g_isThirdPerson)
							{
								thirdState->unk74[0] = *fMinCurrentZoom;
							}
						}
						else
						{
							if (g_isThirdPerson)
							{
								if (*fMinCurrentZoom >= thirdState->unk74[4])
								{
									thirdState->unk74[4] = *fMinCurrentZoom + 0.009999999776482582;
								}

								thirdState->unk74[0] = thirdState->unk74[4];
							}
						}

						gi_var15 = 2;
					}

					gi_var14 = 0;
					// Nothing else
				}
			}

			// Nothing here
		}
		
		// Nothing here 
	}
	else 
	{
		// 10004D13

		if (g_transform == TRANSFORM_WEREWOLF)
		{
			if (IsVampireLord(player))
			{
				g_transform = TRANSFORM_VAMPIRELORD;
			}			
		}
		else
		{
			if (!IsVampireLord(player))
			{
				g_transform = TRANSFORM_WEREWOLF;
			}
		}

		// 10004D43
		if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
		{
			g_transform = TRANSFORM_NONE;
			gi_var14 = 0;
		}
		else
		{
			if (cameraState->stateId != PlayerCamera::kCameraState_ThirdPerson2 || PlayerControlsFunc1())
			{
				jumped_l15 = true;
				goto L15;
			}

			ForceFirstPerson();
			g_transform = TRANSFORM_NONE;
			gi_var14 = 0;
		}
	}	

	// L20

	if (PlayerControlsFunc1())
	{
		bool is_killcam = (cameraState->stateId == PlayerCamera::kCameraState_VATS);

		if (g_inKillMoveTransform == 0)
		{
			if (g_transform != 0 && (IsKillMove(player) || is_killcam))
			{
				// 10004EE0
				if (g_transform == TRANSFORM_WEREWOLF || g_transform == TRANSFORM_VAMPIRELORD)
				{
					float chance = (g_transform == TRANSFORM_VAMPIRELORD) ? fFirstPersonVampireLordKillmoveChance : fFirstPersonWerewolfKillmoveChance;

					if (EvaluateProbability(chance))
					{
						g_transform = TRANSFORM_NONE;

						if (g_isThirdPerson)
							g_isThirdPerson = 0;
					}
				}

				g_inKillMoveTransform = 1;
			}

			// Nothing here
		}
		else
		{
			// 10004F5D
			
			if (!IsKillMove(player) && !is_killcam)
			{
				if (g_transform == TRANSFORM_NONE)
				{
					g_transform = ((int)IsVampireLord(player)) + 1;
				}

				if (g_isThirdPerson == 0)
					g_isThirdPerson = 1;

				g_inKillMoveTransform = 0;
			}

			// Nothing here
		}

		// Nothing here
	}

	// L25
	
	if (bFirstPersonScripted)
	{
		if (!gi_var11)
		{
			if (cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && g_isThirdPerson && PlayerControlsFunc2())
			{
				gi_var11 = 1;
			}
		}
		else
		{
			if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
			{
				gi_var11 = 0;
			}
			else if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 && !PlayerControlsFunc2())
			{
				ForceFirstPerson();
				gi_var11 = 0;
			}
		}

		// Nothing here
	}

	// L26

	if (bFirstPersonPairedAnim)
	{
		if (!g_inPairedAnim)
		{
			if (cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && g_isThirdPerson && DoGameFunc5() && 
				!IsKillMove(player) && !SFunc2_3(player) && !PlayerControlsFunc1() && !PlayerControlsFunc2())
			{
				g_inPairedAnim = 1;
			}
		}
		else
		{
			// 100050A2
			if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
			{
				g_inPairedAnim = 0;
			}
			else if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2  && !DoGameFunc5())
			{
				ForceFirstPerson();
				g_inPairedAnim = 0;
			}

			// Nothing here
		}
	}

	// L27

	if (bFirstPersonRestrained)
	{
		if (!g_restrained)
		{
			if (cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && g_isThirdPerson && 
				IsUnknown1E00000_0C00000(player) && !IsKillMove(player) && !PlayerControlsFunc1() && !PlayerControlsFunc2())
			{
				g_restrained = 1;
			}
		}
		else
		{
			// 1000514E
			if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
			{
				g_restrained = 0;
			}
			else if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 && !IsUnknown1E00000_0C00000(player))
			{
				ForceFirstPerson();
				g_restrained = 0;
			}
		}

		// Nothing here
	}

	// L28

	if (bFirstPersonDontMove)
	{
		if (!g_inDontMove)
		{
			if (cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && g_isThirdPerson &&
				IsUnknown1E00000_1200000(player) && !IsKillMove(player) && !PlayerControlsFunc1() && !PlayerControlsFunc2())
			{
				g_inDontMove = 1;
			}
		}
		else
		{
			// 100051F6
			if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
			{
				g_inDontMove = 0;
			}
			else if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 && !IsUnknown1E00000_1200000(player))
			{
				ForceFirstPerson();
				g_inDontMove = 0;
			}
		}
	}

	// L29

	if (bFixCamKillmovePlayerDeath)
	{
		if (!g_inKillmovePlayerDeath)
		{
			if (cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && g_isThirdPerson && IsKillMove(player))
			{
				g_inKillmovePlayerDeath = 1;
			}
		}
		else if (!IsKillMove(player))
		{
			if (player->IsDead(1) && !g_isThirdPerson)
			{
				g_isThirdPerson = 1;
			}	

			g_inKillmovePlayerDeath = 0;
		}
	}

	// L30

	if (!gi_var19)
	{
		bool b = SFunc2_3(player);

		if (b)
		{
			if (!IsRiding(player))
			{
				// 100052FC
				ThirdPersonState_ *horseState = (ThirdPersonState_ *)camera->cameraStates[PlayerCamera::kCameraState_Horse];
				ThirdPersonState_ *dragonState = (ThirdPersonState_ *)camera->cameraStates[PlayerCamera::kCameraState_Dragon];

				if (horseState)
				{
					float zoom;
					
					if (bFirstPersonHorse && g_isThirdPerson)
					{
						zoom = *fMinCurrentZoom;
					}
					else
					{
						// 10005321
						if (*fMinCurrentZoom >= horseState->unk74[4])
						{
							horseState->unk74[4] = horseState->unk74[1] + 0.009999999776482582;							
						}

						zoom = horseState->unk74[4];
					}

					// 10005349
					horseState->unk74[0] = horseState->unk74[1] = zoom;
				}

				// 1000534F

				if (dragonState)
				{
					float zoom;

					if (bFirstPersonDragon && g_isThirdPerson)
					{
						zoom = *fMinCurrentZoom;
					}
					else
					{
						// 10005374
						if (*fMinCurrentZoom >= dragonState->unk74[4])
						{
							dragonState->unk74[4] = dragonState->unk74[1] + 0.009999999776482582;
						}

						zoom = dragonState->unk74[4];
					}

					// 1000539C
					dragonState->unk74[0] = dragonState->unk74[1] = zoom;
				}

				// Nothing here

			} // end !IsRiding	

			// 100053A2
			if (g_isThirdPerson)
			{
				gi_var10 = 1;
				gi_var19 = 1;
			}
			else
			{
				gi_var19 = 2;
				gi_var10 = 0;
			}

			// Nothing here
		}

		// Nothing here
	}
	else
	{
		// 100053C9
		bool b = SFunc2_3(player);

		if (!b)
		{
			if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson || cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 ||
				cameraState->stateId == PlayerCamera::kCameraState_Transition)
			{
				if (cameraState->stateId == PlayerCamera::kCameraState_Transition)
				{
					ThirdPersonState_ *horseState = (ThirdPersonState_ *)camera->cameraStates[PlayerCamera::kCameraState_Horse];
					ThirdPersonState_ *dragonState = (ThirdPersonState_ *)camera->cameraStates[PlayerCamera::kCameraState_Dragon];

					if (horseState)
					{
						if (*fMinCurrentZoom < horseState->unk74[1])
							horseState->unk74[4] = horseState->unk74[1];
					}

					if (dragonState)
					{
						if (*fMinCurrentZoom < dragonState->unk74[1])
							dragonState->unk74[4] = dragonState->unk74[1];
					}
				}

				// 10005451
				if (gi_var19 == 1)
				{
					// 1000545E
					if (gi_var10 == 0)
					{
						// 10005475
						ForceThirdPerson();

						ThirdPersonState_ *thirdState = (ThirdPersonState_ *)camera->cameraStates[PlayerCamera::kCameraState_ThirdPerson2];

						if (thirdState)
						{
							thirdState->unk74[0] = thirdState->unk74[1] = thirdState->unk74[4];
						}

						// Nothing here
					}
					else
					{
						ForceFirstPerson();
					}
				}
				else if (gi_var10 == 1)
				{
					ForceFirstPerson();
				}

				// 100054B2
				gi_var19 = 0;
				gi_var10 = 0;
			}

			// Nothing here
		}

		// Nothing here
	}

	// L40
	if (*fMountedMaxLookingDown != fMountedMaxLookingDownOverride)
		*fMountedMaxLookingDown = fMountedMaxLookingDownOverride;

	if (*fFlyingMountedMaxLookingDown != fFlyingMountedMaxLookingDownOverride)
		*fFlyingMountedMaxLookingDown = fFlyingMountedMaxLookingDownOverride;

	// L41
	
	if (g_isThirdPerson)
	{
		bool b = SFunc2_3(player);
		bool cf_42 = true, cf_43 = true;
		float angle = 0.0f;
		ThirdPersonState_ *thirdState = (ThirdPersonState_ *)cameraState;

		if (b)
		{
			if (cameraState->stateId == PlayerCamera::kCameraState_Dragon)
			{
				angle = fFlyingMountedRestrictAngle;
				if (angle >= thirdState->unkC0[5]) 
				{
					angle = -angle;
				}
				else
				{
					cf_42 = false;
				}
			}
			else if (cameraState->stateId == PlayerCamera::kCameraState_Horse)
			{
				angle = fMountedRestrictAngle;
				if (angle >= thirdState->unkC0[5])
				{
					angle = -angle;					
				}
				else
				{
					cf_42 = false;
				}
			}
			else
			{
				cf_42 = false;
				cf_43 = false;
			}
		}
		else
		{
			// 10005596
			if (!PlayerControlsFunc2())
			{
				// 100055FB
				if (PlayerControlsFunc1())
				{
					angle = fWerewolfRestrictAngle;

					if (angle < M_PI && cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
					{
						if (angle >= thirdState->unkC0[5])
						{
							angle = -angle;
						}
						else
						{
							cf_42 = false;
						}
					}
					else
					{
						cf_42 = false;
						cf_43 = false;
					}
				}
				else
				{
					cf_42 = false;
					cf_43 = false;
				}
			}
			else
			{
				// 100055AE
				angle = fScriptedRestrictAngle;
				if (angle < M_PI && cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
				{
					if (angle >= thirdState->unkC0[5])
					{
						angle = -angle;
					}
					else
					{
						cf_42 = false;
					}
				}
				else
				{
					cf_42 = false;
					cf_43 = false;
				}
			}
		}

		// L42
		if (cf_42)
		{
			if (angle <= thirdState->unkC0[5]) 
			{
				cf_43 = false;
			}
		}

		// L43
		if (cf_43)
		{
			thirdState->unkC0[5] = angle; 
		}

		// Nothing here
	}

	// L45

	if (SFunc2_3(player) && cameraState->stateId == PlayerCamera::kCameraState_Horse && !IsRiding(player))
	{
		ThirdPersonState_ *thirdState = (ThirdPersonState_ *)cameraState;

		if (thirdState->unkC0[5] != 0.0)
		{
			if (g_isThirdPerson && bFirstPersonHorse)
			{
				if (bFixHorseMountCamera1st)
					thirdState->unkC0[5] = 0.0f;
			}
			else if (bFixHorseMountCamera3rd)
			{
				thirdState->unkC0[5] = 0.0f;
			}
		}
	}

	// L46

	if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson1)
	{
		bool craft_reset_check = ((g_isThirdPerson)) ? (bCraftingResetCamera1st != 0) : (bCraftingResetCamera3rd != 0);
		
		if (craft_reset_check && player->rot.x != 0.0)
		{
			player->rot.x = 0.0;
		}
	}

	// L47

	if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
	{
		ThirdPersonState_ *thirdState = (ThirdPersonState_ *)cameraState;
		
		if (bSwitchPOVMatchCameraRotX)
		{
			player->rot.x = player->rot.x - thirdState->unkD8;
			thirdState->unkD8 = 0.0;
		}

		if (bSwitchPOVMatchCameraRotZ)
		{
			gf_var1 = thirdState->unkC0[5];
		}
	}

	// L48

	if (SFunc2_3(player))
	{
		if (IsRiding(player) && (cameraState->stateId == PlayerCamera::kCameraState_Horse || cameraState->stateId == PlayerCamera::kCameraState_Dragon))
		{
			// 1000577F
			PlayerControls *pc = PlayerControls::GetSingleton();
			ThirdPersonState_ *thirdState = (ThirdPersonState_ *)cameraState;
			float zoom = fMountedSwitchPOVDetectDistance + *fMinCurrentZoom;

			if (pc && PlayerControlsFunc3() && !gi_var15 && thirdState->unk8C > fMountedSwitchPOVDetectDistance)
			{
				if (zoom >= thirdState->unk74[1])
				{
					thirdState->unk74[0] = thirdState->unk74[1] = zoom + 0.001000000047497451;					 
				}
			}

			// L49
			if (zoom >= thirdState->unk74[1])
			{
				// 10005820
				if (*fMinCurrentZoom >= thirdState->unk74[1])
				{
					if (!gi_var10)
					{
						gi_var10 = 1;

						if (gi_var15 != 2 && !IsLoadingMenuOpen())
						{
							thirdState->unk74[4] = zoom;
						}
					}

					if (!g_isThirdPerson)
						g_isThirdPerson = 1;
				}
			}
			else
			{
				if (gi_var10 == 0)
					gi_var10 = 0;

				if (g_isThirdPerson == 0)
					g_isThirdPerson = 0;
			}

			// Nothing here
		}

		// Nothing here
	}
	else
	{
		// L50
		ThirdPersonState_ *thirdState = (ThirdPersonState_ *)cameraState;		
		
		if (PlayerControlsFunc1())
		{
			// 10005898
			if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 && !IsKillMove(player) && !gi_var14)
			{
				if (PlayerControlsFunc3() && gi_var15 != 0 && thirdState->unk8C > 0.0)
				{
					float zoom = *fMinCurrentZoom + 0.009999999776482582;

					if (zoom >= thirdState->unk74[1])
					{
						thirdState->unk74[0] = thirdState->unk74[1] = zoom;
					}
				}

				// L52
				if (*fMinCurrentZoom >= thirdState->unk74[1])
				{
					// 1000594A
					// Skipped identical comparison to above

					if (g_transform == TRANSFORM_NONE)
					{
						g_transform = (int)IsVampireLord(player) + 1;

						if (gi_var15 != 2 && !IsLoadingMenuOpen())
						{
							thirdState->unk74[4] = *fMinCurrentZoom + 0.009999999776482582;
						}
					}

					// L58
					if (!g_isThirdPerson)
						g_isThirdPerson = 1;
				}
				else
				{
					if (g_transform)
						g_transform = 0;

					if (g_isThirdPerson)
						g_isThirdPerson = 0;
				}
			}
		}
		else if (PlayerControlsFunc2()) // L51
		{
			// 10005999
			if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
			{
				if (PlayerControlsFunc3() && !gi_var15 && thirdState->unk8C > 0.0)
				{
					float zoom = *fMinCurrentZoom + 0.009999999776482582;

					if (zoom >= thirdState->unk74[1])
					{
						thirdState->unk74[0] = thirdState->unk74[1] = zoom;
					}
				}

				// L55
				if (*fMinCurrentZoom >= thirdState->unk74[1])
				{
					// Skipped duplicate comparison
					if (!gi_var11)
					{
						gi_var11 = 1;

						if (gi_var15 != 2 && !IsLoadingMenuOpen())
						{
							thirdState->unk74[4] = *fMinCurrentZoom + 0.009999999776482582;
						}
					}

					// L58
					if (!g_isThirdPerson)
						g_isThirdPerson = 1;
				}
				else
				{
					if (gi_var11)
						gi_var11 = 0;

					if (g_isThirdPerson)
						g_isThirdPerson = 0;
				}
			}

			// Nothing here
		}
		else if (g_isThirdPerson && bForceFirstPersonCamera && cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 &&
			     thirdState->unk74[1] < thirdState->unk74[0]) // L59
		{
			g_isThirdPerson = 0;
		}
	}

	// L60

	if (gi_var12)
	{
		if (SFunc2_3(player))
		{
			if (cameraState->stateId == PlayerCamera::kCameraState_Horse || cameraState->stateId == PlayerCamera::kCameraState_Dragon)
			{
				if (fMountedSwitchPOVDetectDistance < cameraState->unk8C)
				{
					// 10005B81
					gi_var15 = 2;
				}
				else if (g_isThirdPerson)
				{
					// 10005B06
					float zoom = *fMinCurrentZoom;
					g_isThirdPerson = 0;
					cameraState->unk74[1] = zoom;

					if (cameraState->unk74[4] > zoom)
					{
						gi_var15 = 2;
						cameraState->unk74[0] = cameraState->unk74[4];
					}
					else
					{
						gi_var15 = 2;
						cameraState->unk74[0] = cameraState->unk74[4] = fMountedSwitchPOVDetectDistance + zoom + 0.001000000047497451;
					}
				}
				else
				{
					// 10005B62
					gi_var15 = 2;
					cameraState->unk74[4] = gf_var20;
					cameraState->unk74[0] = *fMinCurrentZoom;
				}

				// L69
				gi_var12 = 0;
			}

			// Nothing here
		}
		else if (PlayerControlsFunc1()) // L65
		{
			if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
			{
				if (cameraState->unk8C > 0.0)
				{
					// 10005C2E
					gi_var15 = 2;
				}
				else // 10005BC4
				{
					float zoom = *fMinCurrentZoom; // eax

					if (g_isThirdPerson)
					{
						g_transform = TRANSFORM_NONE;
						g_isThirdPerson = 0;
						cameraState->unk74[1] = zoom;

						if (zoom >= cameraState->unk74[4])
						{
							gi_var15 = 2;
							cameraState->unk74[0] = cameraState->unk74[4] = zoom + 0.009999999776482582;
						}
						else
						{
							gi_var15 = 2;
							cameraState->unk74[0] = cameraState->unk74[4];
						}
					}
					else
					{
						// L67
						cameraState->unk74[4] = cameraState->unk74[1];
						cameraState->unk74[0] = zoom;
						gi_var15 = 2;
					}
				}

				// L69
				gi_var12 = 0;
			}

			// Nothing here
		}
		else if (PlayerControlsFunc2()) // L66
		{
			// 10005C49
			if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
			{
				if (cameraState->unk8C > 0.0)
				{
					gi_var15 = 2;
				}
				else
				{
					// 10005C5B
					float zoom = *fMinCurrentZoom; // eax
					gi_var11 = 0;
					g_isThirdPerson = 0;
					cameraState->unk74[1] = zoom;

					if (zoom >= cameraState->unk74[4])
					{
						gi_var15 = 2;
						cameraState->unk74[0] = cameraState->unk74[4] = zoom + 0.009999999776482582;
					}
					else
					{
						gi_var15 = 2;
						cameraState->unk74[0] = cameraState->unk74[4];
					}
				}

				// L69
				gi_var12 = 0;
			}

			// Nothing here
		}
		else if (gi_var9 == 3) // L68
		{
			gi_var12 = 0; // L69
		}
		
		// Nothing here
	}

	// L70

	if (!gi_var21)
	{
		if (PlayerControlsFunc3())
		{
			gi_var21 = 1;
		}
		else
		{
			// 10005CB2
			if (cameraState->stateId == PlayerCamera::kCameraState_Horse || cameraState->stateId == PlayerCamera::kCameraState_Dragon)
			{
				if (gf_var20 != cameraState->unk74[1])
				{
					gf_var20 = cameraState->unk74[1];
				}
			}
		}
	}
	else if (!PlayerControlsFunc3()) // 10005CDC
	{
		gi_var21 = 0;

		if (!gi_var15)
			gi_var12 = 1; // (Not a mistake)
	}

	// L75
	
	if (cameraState->stateId == PlayerCamera::kCameraState_Horse || cameraState->stateId == PlayerCamera::kCameraState_Dragon ||
		cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2)
	{
		if (gi_var15)
		{
			// 10005D3D
			if (!gi_var21 && cameraState->unk74[0] == cameraState->unk74[1])
				gi_var15 = 0;
		}
		else if (cameraState->unk74[0] != cameraState->unk74[1])
		{
			gi_var15 = 1;
		}
	}

	// L76

	if (g_savedCameraState != cameraState->stateId)
	{
		g_previousCameraState = g_savedCameraState;
		g_savedCameraState = cameraState->stateId;
	}

	if (g_previousCameraState == PlayerCamera::kCameraState_FirstPerson)
	{
		gi_var24 = 1;

		if (cameraState->stateId != PlayerCamera::kCameraState_TweenMenu)
			gi_var24 = 0;
	}
	else
	{
		gi_var24 = 0;
	}

	// L77

	if (bEnableFirstPersonBody)
	{
		// 10005D97
		bool is_first_person = (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson);
		bool is_bow = (EquippedItemType(player) == 7 || EquippedItemType(player) == 12); // bow/crossbow
		bool hide_body = false;
		
		if (is_first_person && !IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
		{
			if (SFunc2_8() || ShouldFixSkyrimIntro())
			{
				hide_body = true;
			}
			else if (bHideBodySitting && IsSitting(player))
			{
				hide_body = true;
			}
			else if (bHideBodySleeping && IsSleeping(player))
			{
				hide_body = true;
			}
			else if (bHideBodyJumping && IsJumping(player))
			{
				hide_body = true;
			}
			else if (bHideBodySwimming && IsSwimming(player))
			{
				hide_body = true;
			}
			else if (bHideBodySneakRoll && IsSneakRolling(player))
			{
				hide_body = true;
			}
			else if (bHideBodyKillmove && IsKillMove(player))
			{
				hide_body = true;
			}
			else if (bHideBodyAttack && IsAttacking(player) && !IsPowerAttacking(player) && !is_bow)
			{
				hide_body = true;
			}
			else if (bHideBodyPowerAttack && IsPowerAttacking(player) && !is_bow)
			{
				hide_body = true;
			}
			else if (bHideBodyAttack && IsAttacking(player) && is_bow)
			{
				hide_body = true;
			}
		}

		if (hide_body)
		{
			// L78
			if (!(root3rd->m_flags & 1))
			{
				root3rd->m_flags |= 1;
			}
		}
		else
		{
			// L80
			if (root3rd->m_flags & 1)
			{
				root3rd->m_flags &= ~1;
			}
		}

		// L81
		UpdateArms();

		hide_body = false;

		if (!is_first_person && !gi_var24)
		{
			hide_body = true;
		}
		else if (IsMenuOpen(menu, uistr, &uistr->mapMenu))
		{
			hide_body = true;
		}
		else if (IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
		{
			hide_body = true;
		}

		if (hide_body)
		{
			if (!(root1st->m_flags & 1))
			{
				root1st->m_flags |= 1;
			}
		}
		else
		{
			// 10005FA1
			if (root1st->m_flags & 1)
			{
				root1st->m_flags &= ~1;
			}
		}
	}
	else
	{
		// 10005FD0
		if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
		{
			if (!(root3rd->m_flags & 1))
			{
				root3rd->m_flags |= 1;
			}
		}
		else
		{
			if (root3rd->m_flags & 1)
			{
				root3rd->m_flags &= ~1;
			}
		}
	}

	// L85 
	bool b9 = (SFunc2_3(player) && !IsRiding(player));

	if (!cameraNI || !g_isThirdPerson)
		return;

	if (IsMenuOpen(menu, uistr, &uistr->mapMenu) || IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
		return;

	if (SFunc2_8())
		return;

	if (!bFirstPersonHorse && IsRiding(player) && cameraState->stateId == PlayerCamera::kCameraState_Horse)
	{
		return;
	}

	if (!bFirstPersonDragon && IsRiding(player) && cameraState->stateId == PlayerCamera::kCameraState_Dragon)
	{
		return;
	}

	if (b9)
	{
		if (!bFirstPersonHorseTransition && cameraState->stateId == PlayerCamera::kCameraState_Horse)
		{
			return;
		}
		
		if (!bFirstPersonDragonTransition && cameraState->stateId == PlayerCamera::kCameraState_Dragon)
		{
			return;
		}
	}

	// L86

	NiNode *head = GetHeadNode(root3rd); // var_8
	NiPoint3 point; // var_4C (x), var_48(y), var_44(z)
	NiMatrix33 matrix1; // var_A0
	NiMatrix33 matrix2; // var_7C
	NiQuaternion quat1; // var_2C (w), var_28(x), var_24(y) var_20 (z)
	NiQuaternion quat2; // var_50 (w), var_4C (x), var_48 (y) var_44 (z)

	NiPoint3 *point1 = (NiPoint3 *)&quat1.m_fX;
	NiPoint3 *point2 = (NiPoint3 *)&quat2.m_fX;

	if (!head)
		return;

	int stateId = cameraState->stateId; // var_58

	if (stateId == PlayerCamera::kCameraState_Furniture)
	{
		FurnitureCameraState_ *furnState = (FurnitureCameraState_ *)cameraState;
		
		MatrixToEuler(&head->m_worldTransform.rot, &point.x, &point.y, &point.z);
		furnState->unk20 = -point.z;
	}

	// L87
	float hrot = GetHeadRotation();

	if (hrot != 0.0)
	{
		point.x = -player->rot.x;
		point.y = M_PI / 2.0;
		point.z = M_PI / 2.0;

		EulerToMatrix(&matrix1, point.x, point.y, point.z);
		matrix2 = head->m_worldTransform.rot * matrix1;
		
		RotMatrixToQuaternion(&cameraNI->m_worldTransform.rot, &quat1);
		RotMatrixToQuaternion(&matrix2, &quat2);

		NiQuaternion ret;
		SlerpQuat(&ret, &quat1, &quat2, hrot);
		quat2 = ret;

		QuaternionToMatrix(&quat2, &cameraNI->m_worldTransform.rot);
		MatrixInverse(&cameraNI->m_localTransform.rot, &matrix1);

		matrix2 = cameraNI->m_worldTransform.rot * matrix1;
		cameraNode->m_worldTransform.rot = cameraNode->m_localTransform.rot = matrix2;
	}

	// L90

	if (cameraState->stateId == PlayerCamera::kCameraState_FirstPerson || gi_var11 || g_transform != TRANSFORM_NONE || gi_var10)
	{
		bool doit = true;
		
		if (cameraState->stateId == PlayerCamera::kCameraState_Transition || cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson1)
		{
			doit = false;
		}

		else if (IsSleeping(player) || IsKnockout(player) || player->IsDead(1))
		{
			doit = false;
		}

		else if (IsKillMove(player) && cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
		{
			doit = false;
		}

		else if (b9 && ((bFirstPersonHorseTransition && cameraState->stateId == PlayerCamera::kCameraState_Horse) || 
			(bFirstPersonDragonTransition && cameraState->stateId == PlayerCamera::kCameraState_Dragon)))
		{
			doit = false;
		}

		else if (IsIdlePlaying() && cameraState->stateId != PlayerCamera::kCameraState_FirstPerson && g_transform == TRANSFORM_NONE &&
			(!gi_var11 || bScriptedIdleAnimMatchHeadRotation))
		{
			doit = false;
		}

		else if (DoGameFunc5() && cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
		{
			doit = false;
		}

		if (doit)
		{
			// 10006343
			
			if (!EnableHeadBobPosition() || (IsKillMove(player) && cameraState->stateId == PlayerCamera::kCameraState_FirstPerson))
			{
				// 10006368
				if (!gi_var11 && g_transform == TRANSFORM_NONE && !gi_var10)
				{
					if (!IsUnknown3C000(player) || b9 || !bEnableFirstPersonBody || bHideBodySitting || ShouldFixSkyrimIntro())
						return;
				}
			}

			// L91
			ScalePoint(point1, root3rd->m_worldTransform.scale);			

			// L95 (skip fmul, fstp)
			if (gi_var10 || (IsUnknown3C000(player) && bEnableFirstPersonBody && !bHideBodySitting))
			{
				// 1000658E
				MatrixVectorMultiply(point2, &head->m_worldTransform.rot, point1);
			}
			else
			{
				// 1000657B
				MatrixVectorMultiply(point2, &root3rd->m_worldTransform.rot, point1);
			}

			// L96
			cameraNI->m_worldTransform.pos = cameraNode->m_worldTransform.pos = cameraNode->m_localTransform.pos = head->m_worldTransform.pos + *point2;

			if (g_transform != TRANSFORM_NONE)
			{
				bool copy = false;

				if (bWerewolfCameraRotateIdleAnim && IsIdlePlaying())
				{
					copy = true;
				}
				else if (bWerewolfCameraRotateRestrained && IsUnknown1E00000_0C00000(player))
				{
					copy = true;
				}

				if (copy)
				{
					// 10006628
					cameraNode->m_worldTransform.rot = cameraNode->m_localTransform.rot = head->m_worldTransform.rot;
					cameraNI->m_worldTransform.rot = head->m_worldTransform.rot * cameraNI->m_localTransform.rot;
				}
			}

			// L97
			UpdateCamera(cameraNI);
			return;
		}

		// Nothing here
	}
	
	// L100
	ScalePoint(point1, root3rd->m_worldTransform.scale);

	// L110 (skip fmul, fstp)
	MatrixVectorMultiply(point2, &head->m_worldTransform.rot, point1);
	cameraNI->m_worldTransform.pos = cameraNode->m_worldTransform.pos = cameraNode->m_localTransform.pos = head->m_worldTransform.pos + *point2;
	
	if (cameraState->stateId == PlayerCamera::kCameraState_TweenMenu)
		return;

	if (cameraState->stateId == PlayerCamera::kCameraState_ThirdPerson2 || IsUnknown3C000_1C000(player))
	{
		// 10006898
		EulerToMatrix(&matrix2, cameraState->unkD8 - player->rot.x + 0.0, M_PI / 2, M_PI / 2);
		cameraNI->m_worldTransform.rot = head->m_worldTransform.rot * matrix2;
		MatrixInverse(&cameraNI->m_localTransform.rot, &matrix1);
		matrix2 = cameraNI->m_worldTransform.rot * matrix1;

		cameraNode->m_worldTransform.rot = cameraNode->m_localTransform.rot = matrix2;
	}
	else
	{
		// 10006877
		cameraNode->m_worldTransform.rot = cameraNode->m_localTransform.rot = head->m_worldTransform.rot;
		cameraNI->m_worldTransform.rot = head->m_worldTransform.rot * cameraNI->m_localTransform.rot;
	}

	// L_FINAL
	UpdateCamera(cameraNI); 
}

void TranslateThirdPerson()
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	NiNode *root3rd = player->GetNiRootNode(0);

	if (IsUnknown3C000(player) || DoGameFunc5())
		return;

	bool cf5 = false;
	
	if (!EnableHeadBobPosition() && camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson && !gi_var11 && !g_transform && !gi_var10)
	{
		cf5 = true;
	}
	else if (bEnableHeadBobPositionScripted != 2  && gi_var11 == 1)
	{
		cf5 = true;
	}
	else if (bEnableHeadBobPositionWerewolf != 2 && g_transform == TRANSFORM_WEREWOLF)
	{
		cf5 = true;
	}
	else if (!bEnableHeadBobPositionVampireLord && g_transform == TRANSFORM_VAMPIRELORD)
	{
		cf5 = true;
	}
	else if (!bEnableHeadBobPositionHorse && !IsFighting(player) && gi_var10 && camera->cameraState->stateId == PlayerCamera::kCameraState_Horse)
	{
		cf5 = true;
	}
	else if (!bEnableHeadBobPositionHorseCombat && IsFighting(player) && gi_var10 && camera->cameraState->stateId == PlayerCamera::kCameraState_Horse)
	{
		cf5 = true;
	}
	else if (!bEnableHeadBobPositionDragon && !IsFighting(player) && gi_var10 && camera->cameraState->stateId == PlayerCamera::kCameraState_Dragon)
	{
		cf5 = true;
	}
	else if (!bEnableHeadBobPositionDragonCombat && IsFighting(player) && gi_var10 && camera->cameraState->stateId == PlayerCamera::kCameraState_Dragon)
	{
		cf5 = true;
	}
	else if (IsKillMove(player) && camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
	{
		cf5 = true;
	}

	if (cf5)
	{
		// L5
		NiNode *root1st = player->GetNiRootNode(1);
		NiNode *camera1st = FindNode(root1st, "Camera1st [Cam1]");
		NiNode *head = GetHeadNode(root3rd);

		if (!camera1st || !head)
			return;

		NiPoint3 point; // var_14, var_10, var_C 
		ScalePoint(&point, root3rd->m_worldTransform.scale);			

		// L6 (skipped fmult/fstp)
		NiPoint3 point2; // var_20, var_1C, var_18
		MatrixVectorMultiply(&point2, &root3rd->m_worldTransform.rot, &point);

		root3rd->m_localTransform.pos.x = camera1st->m_worldTransform.pos.x - (head->m_worldTransform.pos.x + point2.x) + root3rd->m_localTransform.pos.x;
		root3rd->m_localTransform.pos.y = camera1st->m_worldTransform.pos.y - (head->m_worldTransform.pos.y + point2.y) + root3rd->m_localTransform.pos.y;

		if (gi_var11 && bEnableHeadBobPositionScripted)
			return;

		if (g_transform && bEnableHeadBobPositionWerewolf)
			return;

		root3rd->m_localTransform.pos.z = camera1st->m_worldTransform.pos.z - (head->m_worldTransform.pos.z + point2.z) + root3rd->m_localTransform.pos.z;
	}
	else if (EnableHeadBobPosition() == 2 || IsSwimming(player)) // L10
	{
		// L15
		NiNode *head = GetHeadNode(root3rd);

		if (!head)
			return;

		NiPoint3 point; // var_14, var_10, var_C 
		ScalePoint(&point, root3rd->m_worldTransform.scale);		

		// L20 (skip fmul/fstp)
		NiPoint3 point2; // var_20, var_1C, var_18
		MatrixVectorMultiply(&point2, &root3rd->m_worldTransform.rot, &point);

		root3rd->m_localTransform.pos.x = (root3rd->m_worldTransform.pos.x - (head->m_worldTransform.pos.x + point2.x)) * 0.8 + root3rd->m_localTransform.pos.x;
		root3rd->m_localTransform.pos.y = 0.8 * (root3rd->m_worldTransform.pos.y - (head->m_worldTransform.pos.y + point2.y)) + root3rd->m_localTransform.pos.y;
	}
	else
	{
		// 100037EA
		NiPoint3 point; // var_14, var_10, var_C 

		point.x = root3rd->m_worldTransform.scale * 0.0;
		
		if (IsSneaking(player))
		{
			point.y = root3rd->m_worldTransform.scale * -35.0;
		}
		else
		{
			point.y = root3rd->m_worldTransform.scale * -25.0;
		}

		point.z = point.x;

		NiPoint3 point2; // var_20, var_1C, var_18
		MatrixVectorMultiply(&point2, &root3rd->m_worldTransform.rot, &point);
		root3rd->m_localTransform.pos += point2;
	}
}

void DoUpdateNode(NiNode *node)
{
	NiAVObject::ControllerUpdateContext ctx;
	ctx.delta = *ControllerVar1;
	ctx.flags = 0x1;

	CALL_MEMBER_FN(node, UpdateNode)(&ctx);
}

void UpdateSkeletonNodes(bool show)
{
	// Static vars notice
	static float n_scale = 1.0f;
	static float lclav_scale = 1.0f;
	static float rclav_scale = 1.0f;
	static float back_scale = 1.0f;
	static float bow_scale = 1.0f;
	static float quiver_scale = 1.0f;
	
	PlayerCharacter *player = (*g_thePlayer);	
	NiNode *root3rd = player->GetNiRootNode(0);

	NiNode *lclavicle = FindNode(root3rd, "NPC L Clavicle [LClv]");
	NiNode *rclavicle = FindNode(root3rd, "NPC R Clavicle [RClv]");
	NiNode *head = GetHeadNode(root3rd);
	NiNode *weapon_back = FindNode(root3rd, "WeaponBack");
	NiNode *weapon_bow = FindNode(root3rd, "WeaponBow");
	NiNode *quiver = FindNode(root3rd, "QUIVER");
	bool b4 = SFunc2_3(player) && !IsRiding(player); 

	if (bFixWerewolfTransformation && g_transform == TRANSFORM_WEREWOLF && gi_var14 == 2)
	{
		head = FindNode(root3rd, "NPC Spine2 [Spn2]");
	}

	// 10002D38
	if (show)
	{
		if (lclavicle && rclavicle)
		{
			lclavicle->m_localTransform.scale = lclav_scale;
			rclavicle->m_localTransform.scale = rclav_scale;
		}

		if (head)
		{
			head->m_localTransform.scale = n_scale;
		}

		if (weapon_back && bHide2HWeaponFirstPerson)
		{
			weapon_back->m_localTransform.scale = back_scale;
		}

		if (weapon_bow && bHideBowFirstPerson)
		{
			weapon_bow->m_localTransform.scale = bow_scale;
		}

		if (quiver && bHideQuiverFirstPerson)
		{
			quiver->m_localTransform.scale = quiver_scale;
		}
	}
	else
	{
		// L5
		PlayerCamera *camera = PlayerCamera::GetSingleton();

		if (lclavicle && rclavicle)
		{
			if (lclavicle->m_localTransform.scale > 0.002)
				lclav_scale = lclavicle->m_localTransform.scale;

			if (rclavicle->m_localTransform.scale > 0.002)
				rclav_scale = rclavicle->m_localTransform.scale;

			if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson || gi_var24)
			{
				// 10002E21
				if (!PlayerControlsFunc1() && (!UseThirdPersonArms() || SFunc2_8() || ShouldFixElderScrollRead()))
				{
					// 10002E58
					bool left = false, right = false;

					if ((player->actorState.flags08 & 0xE0) >= 0x60)
					{
						left = right = true;
					}
					else if (bEquipWeaponAnimFix && IsEquippingWeapon(player))
					{
						left = right = true;						
					}
					else if (SFunc2_8() || ShouldFixElderScrollRead()) // L6
					{
						left = right = true;
					}
					else if (IsTorchOut(player) && (!bFixTorchWhenSitting || !IsUnknown3C000(player)))
					{
						left = true;
					}

					// L7 (more or less)

					if (left)
					{
						lclavicle->m_localTransform.scale = 0.001;
					}

					if (right)
					{
						rclavicle->m_localTransform.scale = 0.001;
					}
				}

				// Nothing here
			}

			// Nothing here

		} // lclavicle && rclavicle

		// L10

		if (head)
		{
			if (head->m_localTransform.scale > 0.002)
				n_scale = head->m_localTransform.scale;

			if (camera->cameraState->stateId != PlayerCamera::kCameraState_Free && !SFunc2_8())
			{
				// 10002F2E
				bool cf14 = false;
				
				if (!bEnableHeadFirstPerson && !SFunc2_3(player) && !PlayerControlsFunc2() && !PlayerControlsFunc1())
				{
					cf14 = true;
				}
				else if (!bEnableHeadFirstPersonHorse && SFunc2_3(player) && camera->cameraState->stateId == PlayerCamera::kCameraState_Horse)
				{
					cf14 = true;
				}
				else if (!bEnableHeadFirstPersonDragon && SFunc2_3(player) && camera->cameraState->stateId == PlayerCamera::kCameraState_Dragon)
				{
					cf14 = true;
				}
				else if (!bEnableHeadFirstPersonScripted && PlayerControlsFunc2())
				{
					cf14 = true;
				}
				else if (!bEnableHeadFirstPersonVampireLord && g_transform == TRANSFORM_VAMPIRELORD)
				{
					cf14 = true;
				}
				else if (!bEnableHeadFirstPersonWerewolf && g_transform == TRANSFORM_WEREWOLF)
				{
					cf14 = true;
				}
				else if (bFixWerewolfTransformation && g_transform == TRANSFORM_WEREWOLF && gi_var14 == 2)
				{
					cf14 = true;
				}

				if (cf14)
				{
					// L14
					if (!b4 || ((bFirstPersonHorseTransition || camera->cameraState->stateId != PlayerCamera::kCameraState_Horse)
						&& (bFirstPersonDragonTransition || camera->cameraState->stateId != PlayerCamera::kCameraState_Dragon)))
					{
						// 10003024
						head->m_localTransform.scale = 0.001;
					}
				}
			}

			// Nothing here

		} // head

		// L15
		if (camera->cameraState->stateId == PlayerCamera::kCameraState_Free)
			return;

		if (weapon_back && bHide2HWeaponFirstPerson)
		{
			if (weapon_back->m_localTransform.scale > 0.002)
				back_scale = weapon_back->m_localTransform.scale;

			weapon_back->m_localTransform.scale = 0.001;
		}		

		// 10003084
		if (weapon_bow && bHideBowFirstPerson)
		{
			if (weapon_bow->m_localTransform.scale > 0.002)
				bow_scale = weapon_bow->m_localTransform.scale;

			weapon_bow->m_localTransform.scale = 0.001;
		}

		// 100030AC
		if (quiver && bHideQuiverFirstPerson)
		{
			if (quiver->m_localTransform.scale > 0.002)
				quiver_scale = quiver->m_localTransform.scale;

			quiver->m_localTransform.scale = 0.001;
		}
	}
}

void SFunc3_4()
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	NiNode *root1st = player->GetNiRootNode(1);
	NiNode *root3rd = player->GetNiRootNode(0);

	NiNode *camera1st = FindNode(root1st, "Camera1st [Cam1]");
	NiPoint3 point = camera1st->m_worldTransform.pos - root1st->m_worldTransform.pos; // var_2C, var_28, var_24
	NiNode *head = FindNode(root3rd, "NPC Head [Head]");
	NiPoint3 point2; // var_14, var_10, var_C

	ScalePoint(&point2, root3rd->m_worldTransform.scale);

	// L5 (skip fmul, fstp)
	NiPoint3 point3; // var_14, var_10, var_C
	NiPoint3 point4; // var_20, var_1C, var_18

	MatrixVectorMultiply(&point3, &root3rd->m_worldTransform.rot, &point2);
	point4 = head->m_worldTransform.pos + point3 - root3rd->m_worldTransform.pos;
	point3 = root3rd->m_localTransform.pos - root1st->m_localTransform.pos;
	root1st->m_localTransform.pos = point4 - point + point3 + root1st->m_localTransform.pos;
}

// UpdateActor?
void Patch3(NiAVObject *pthis, NiAVObject::ControllerUpdateContext *ctx)
{	
	// Original call
	CALL_MEMBER_FN(pthis, UpdateNode)(ctx);

	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	NiNode *root1st = player->GetNiRootNode(1);
	NiNode *root3rd = player->GetNiRootNode(0);

	static UInt32 s3i_var1 = 0; // Static var notice

	if (bFix360Animations)
	{
		if ((camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson && !IsFighting(player)) || s3i_var1)
		{
			// 10003AA9
			NiNode *head = FindNode(root3rd, "NPC Head [Head]");
			NiNode *npc = FindNode(root3rd, "NPC");

			if (head && npc)
			{
				NiPoint3 root_euler; // var_1C, var_18, var_14
				NiPoint3 head_euler; // var_28, var_24, var_20

				MatrixToEuler(&root3rd->m_worldTransform.rot, &root_euler.x, &root_euler.y, &root_euler.z);
				MatrixToEuler(&head->m_worldTransform.rot, &head_euler.x, &head_euler.y, &head_euler.z); 

				float m = fmod(fabs(root_euler.z - head_euler.z + M_PI), 2*M_PI);

				if (fDetect360AnimDegrees >= m || fDetect360AnimDegrees >= ((2 * M_PI) - m))
				{
					// 10003B4A
					NiPoint3 npc_euler; // var_1C, var_18, var_14

					s3i_var1 = !s3i_var1;
					
					MatrixToEuler(&npc->m_localTransform.rot, &npc_euler.x, &npc_euler.y, &npc_euler.z);
					npc_euler.z += M_PI;
					EulerToMatrix(&npc->m_localTransform.rot, npc_euler.x, npc_euler.y, npc_euler.z);

					DoUpdateNode(root3rd);
				}

				// Nothing here
			}

			// Nothing here
		}		

		// Nothing here
	}

	// L5
	saved_pos_3rd = root3rd->m_localTransform.pos;

	if (gi_var25)
	{
		gi_var25++;

		if (gi_var25 > 5)
			gi_var25 = 0;
	}

	if (!g_isThirdPerson)
		return;

	MenuManager *menu = MenuManager::GetSingleton();
	UIStringHolder *uistr = UIStringHolder::GetSingleton();

	if (IsMenuOpen(menu, uistr, &uistr->mapMenu) || IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
		return;

	if (SFunc2_8() || gi_var25)
		return;

	if (bAdjustPlayerScale && root1st->m_localTransform.scale != root3rd->m_localTransform.scale)
	{
		root1st->m_localTransform.scale = root3rd->m_localTransform.scale;
	}

	TranslateThirdPerson();

	if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
	{
		if (!EnableHeadBobPosition())
		{
			root3rd->m_localTransform.pos.z += 5.0;
		}

		root3rd->m_localTransform.pos.z += fCameraHeightOffset;
	}

	if (!bEnableFirstPersonBody && camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
	{
		// 10003CD6
		DoUpdateNode(root3rd);
	}
	else // 10003CB1
	{
		if (bFirstPersonShadows)
		{
			DoUpdateNode(root3rd);			
		}
		else
		{
			UpdateSkeletonNodes(false);
			DoUpdateNode(root3rd);
			UpdateSkeletonNodes(true);
		}
	} 

	// L6

	if (EnableHeadBobPosition())
		SFunc3_4();

	if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
	{
		if (!EnableHeadBobPosition())
		{
			root1st->m_localTransform.pos.z += 5.0;
		}

		root1st->m_localTransform.pos.z += fCameraHeightOffset;
	}

	DoUpdateNode(root1st);
}

NiNode *Patch4(TESObjectREFR *pthis)
{
	// Replace function with GetNiNode
	return GetNiNode(pthis);
}

void Patch5(NiNode *node, void *arg2, bool arg3, bool arg4)
{
	PlayerCharacter *player = (*g_thePlayer);

	node = player->GetNiRootNode(1);
	Patch5Orig(node, arg2, arg3, arg4);

	node = player->GetNiRootNode(0);
	Patch5Orig(node, arg2, arg3, arg4);
}

void Patch6(Actor *pthis, float arg1, void *arg2)
{
	Patch6Orig(pthis, arg1, arg2);
	
	PlayerCharacter *player = (*g_thePlayer);

	if (pthis == player)
	{
		player->unkBDB ^= 1;
		Patch6Orig(pthis, arg1, arg2);
		player->unkBDB ^= 1;
	}
}

bool Patch7(Actor *pthis)
{
	typedef bool (* Func7Type)(Actor *);
	Func7Type Patch7Orig = (Func7Type)GetVtblAddress(pthis, 0x388);

	Patch7Orig(pthis);

	PlayerCamera *camera = PlayerCamera::GetSingleton();

	if (camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
		return true;

	InputManager *input = InputManager::GetSingleton();
	UInt32 val = ~input->unk118;

	if (val & 1)
		return false;

	return true;
}

float Patch8()
{
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	float ret;

	if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
	{
		ret = fControllerBufferDepth1stOverride;
	}
	else
	{
		ret = fControllerBufferDepth3rdOverride;
	}

	return ret;
}

void Patch9_Func(bool b)
{
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	
	if (!bEnableFirstPersonBody || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson || UseThirdPersonArms())
		return;

	MenuManager *menu = MenuManager::GetSingleton();
	UIStringHolder *uistr = UIStringHolder::GetSingleton();

	if (IsMenuOpen(menu, uistr, &uistr->mapMenu) || IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
		return;

	PlayerCharacter *player = (*g_thePlayer);
	NiNode *root3rd = player->GetNiRootNode(0);

	if (b)
	{
		root3rd->m_flags &= ~1;
	}
	else
	{
		root3rd->m_flags |= 1;
	}
}

void Patch9(Actor *pthis)
{
	Patch9_Func(false);
	Patch9Orig(pthis);
	Patch9_Func(true);
}

void Patch10_Func()
{
	if (!bEnableFirstPersonBody)
		return;

	PlayerCharacter *player = (*g_thePlayer);
	NiNode *root3rd = player->GetNiRootNode(0);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	MenuManager *menu = MenuManager::GetSingleton();
	UIStringHolder *uistr = UIStringHolder::GetSingleton();

	if (camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson || UseThirdPersonArms() ||
		IsMenuOpen(menu, uistr, &uistr->mapMenu) || IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
	{
		root3rd->m_flags &= ~1;
	}
	else
	{
		root3rd->m_flags |= 1;
	}
}

void Patch10(Actor *pthis)
{
	Patch10_Func();
	// Original func now
	Patch9Orig(pthis);
}

bool Patch11_Func()
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	bool bl = SFunc2_3(player) && !IsRiding(player);
	int stateId = camera->cameraState->stateId;

	if (stateId == PlayerCamera::kCameraState_FirstPerson || !g_isThirdPerson || PlayerControlsFunc2() || PlayerControlsFunc1())
		return false;

	if (stateId == PlayerCamera::kCameraState_TweenMenu)
		return false;

	return (!SFunc2_3(player) || bl);
}

void Patch11(void *pthis, void *arg1, void *arg2)
{
	if (Patch11_Func())
	{
		// Skip original call
	}
	else
	{
		Patch11Orig(pthis, arg1, arg2);
	}
}

bool Patch14_Func()
{
	PlayerCamera *camera = PlayerCamera::GetSingleton();
	TESCameraState *state = camera->cameraState;

	if (!state)
		return false;

	if (state->stateId == PlayerCamera::kCameraState_FirstPerson || !g_isThirdPerson || state->stateId == PlayerCamera::kCameraState_TweenMenu)
		return false;

	return true;
}

bool UseThirdPersonArmsBow()
{
	PlayerCharacter *player = (*g_thePlayer);

	if (EquippedItemType(player) == 7 || EquippedItemType(player) == 12) // bow/crossbow
	{
		if (!bUseThirdPersonArmsBowAim)
			return false;
	}
	else if (!bUseThirdPersonArms)
	{
		return false;
	}
	
	return true;
}

void Patch15(void *pthis, Actor *actor)
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	if (actor != player || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
	{
		Patch15Orig(pthis);
		return;
	}

	if (IsFighting(player) && !UseThirdPersonArmsBow())
	{
		Patch15Orig(pthis);
	}
	else
	{
		UInt8 s = player->unkBDB;
		player->unkBDB |= 1;
		Patch15Orig(pthis);
		player->unkBDB = s;
	}
}

bool Patch16(void *pthis, Actor *actor)
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	if (actor != player || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
	{
		return Patch16Orig(pthis);		
	}

	bool ret;

	if (IsFighting(player) && !UseThirdPersonArmsBow())
	{
		ret = Patch16Orig(pthis);
	}
	else
	{
		UInt8 s = player->unkBDB;
		player->unkBDB |= 1;
		ret = Patch16Orig(pthis);
		player->unkBDB = s;
	}
	
	return ret;
}

bool Patch17(void *pthis, Actor *actor)
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	if (actor != player || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
	{
		return Patch17Orig(pthis);
	}
	
	bool ret;

	if (IsFighting(player) && !UseThirdPersonArms())
	{
		ret = Patch17Orig(pthis);
	}
	else
	{
		UInt8 s = player->unkBDB;
		player->unkBDB |= 1;
		ret = Patch17Orig(pthis);
		player->unkBDB = s;
	}

	return ret;
}

void Patch18(void *pthis, Actor *actor)
{
	typedef void(*Func18Type)(void *);
	Func18Type Patch18Orig = (Func18Type)GetVtblAddress(pthis, 0x1D8);

	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	if (actor != player || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
	{
		Patch18Orig(pthis);
		return;
	}

	if (IsFighting(player) && !UseThirdPersonArms())
	{
		Patch18Orig(pthis);
	}
	else
	{
		UInt8 s = player->unkBDB;
		player->unkBDB |= 1;
		Patch18Orig(pthis);
		player->unkBDB = s;
	}
}

void SFunc19(Actor *actor)
{
	PlayerCharacter *player = (*g_thePlayer);

	if (actor == player)
	{
		NiNode *root3rd = player->GetNiRootNode(0);
		root3rd->m_localTransform.pos = saved_pos_3rd;
		DoUpdateNode(root3rd);
		gi_var25 = 1;
	}
}

void *Patch19(BSTaskPool *pthis, Actor *actor, float a2, void *a3, UInt8 a4, UInt8 a5)
{
	SFunc19(actor);
	return Patch19Orig(pthis, actor, a2, a3, a4, a5);
}

void SFunc20(Actor *actor)
{
	PlayerCharacter *player = (*g_thePlayer);

	if (actor == player)
	{
		if (gi_var25 <= 1)
		{
			NiNode *root3rd = player->GetNiRootNode(0);
			root3rd->m_localTransform.pos = saved_pos_3rd;
			DoUpdateNode(root3rd);
		}

		gi_var25 = 0;
	}
}

void *Patch20(Actor *pthis, void *a1, float a2, UInt8 a3, UInt8 a4)
{
	SFunc20(pthis);
	return Patch20Orig(pthis, a1, a2, a3, a4);
}

void *Patch21(BSTaskPool *pthis, Actor *actor, float a2, float a3, float a4, float a5)
{
	SFunc19(actor);
	return Patch21Orig(pthis, actor, a2, a3, a4, a5);
}

void Patch22(NiAVObject *pthis, NiAVObject::ControllerUpdateContext *ctx)
{
	PlayerCharacter *player = (*g_thePlayer);
	NiNode *node = player->GetNiNode();
	
	if (node == pthis)
	{
		SFunc20(*g_thePlayer);
	}

	CALL_MEMBER_FN(pthis, UpdateNode)(ctx);
}

bool Patch23(Actor *actor)
{
	if (!Patch23Orig())
	{
		PlayerCharacter *player = (*g_thePlayer);
		return (actor == player && gi_var25 <= 1);
	}

	return true;
}

void Patch24(NiAVObject *pthis, void *a1, UInt32 a2, Actor *actor)
{
	PlayerCharacter *player = (*g_thePlayer);
	if (actor == player)
	{
		NiNode *root3rd = player->GetNiRootNode(0);
		DoUpdateNode(root3rd);
	}

	return Patch24Orig(pthis, a1, a2, 1);
}

void ShowNotification(const char *msg)
{
	BSFixedString str(msg);

	typedef void(*DebugNotificationType)(void *, void *, void *, BSFixedString *);
	RelocAddr<DebugNotificationType> DebugNotification(0x96E260);

	DebugNotification(nullptr, nullptr, nullptr, &str);
}

void UpdateShadowNodes(bool show)
{
	PlayerCharacter *player = (*g_thePlayer);
	PlayerCamera *camera = PlayerCamera::GetSingleton();

	MenuManager *menu = MenuManager::GetSingleton();
	UIStringHolder *uistr = UIStringHolder::GetSingleton();
	NiAVObject* root3rd = player->GetNiRootNode(0);

	if (IsMenuOpen(menu, uistr, &uistr->console) && camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson && 
		!IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
	{
		// 100069AA
		if (bEnableFirstPersonBodyConsole)
		{
			root3rd->m_flags &= ~1;
		}
		else
		{
			root3rd->m_flags |= 1;
		}
		return;
	}

	// L5
	if (!g_isThirdPerson || IsMenuOpen(menu, uistr, &uistr->mapMenu) || IsMenuOpen(menu, uistr, &uistr->raceSexMenu))
		return;

	if (bFirstPersonShadows)
	{
		// 10006A82
		bool is_bow = (EquippedItemType(player) == 7 || EquippedItemType(player) == 12); // bow/crossbow   var_4
		bool check10 = false;

		if (!bEnableFirstPersonBody)
		{
			check10 = true;
		}
		else if (bHideBodySitting && IsSitting(player))
		{
			check10 = true;
		}
		else if (bHideBodySleeping && IsSleeping(player))
		{
			check10 = true;
		}
		else if (bHideBodyJumping && IsJumping(player))
		{
			check10 = true;
		}
		else if (bHideBodySwimming && IsSwimming(player))
		{
			check10 = true;
		}
		else if (bHideBodySneakRoll && IsSneakRolling(player))
		{
			check10 = true;
		}
		else if (bHideBodyKillmove && IsKillMove(player))
		{
			check10 = true;
		}
		else if (bHideBodyAttack && IsAttacking(player) && !IsPowerAttacking(player) && !is_bow)
		{
			check10 = true;
		}
		else if (bHideBodyPowerAttack && IsPowerAttacking(player) && !is_bow)
		{
			check10 = true;
		}
		else if (bHideBodyAttackBow && IsAttacking(player) && is_bow)
		{
			check10 = true;
		}

		if (check10)
		{
			// L10
			if (camera->cameraState->stateId == PlayerCamera::kCameraState_FirstPerson)
			{
				if (!show)
				{
					root3rd->m_flags |= 1;
				}
				else
				{
					root3rd->m_flags &= ~1;
				}

				return;
			}
		}

		// L11
		if (show)
		{
			DoUpdateNode((NiNode *)root3rd);			
		}
		else
		{
			UpdateSkeletonNodes(false);
			DoUpdateNode((NiNode *)root3rd);
			UpdateSkeletonNodes(true);			
		}
	}
	else
	{
		// 10006C17
		if (bEnableFirstPersonBody || camera->cameraState->stateId != PlayerCamera::kCameraState_FirstPerson)
		{
			// 10006C2F
			if (show)
			{
				root3rd->m_flags |= 1;
			}
			else
			{
				root3rd->m_flags &= ~1;
			}
		}

		// nothing
	}

	// nothing
}

/*
	Needs Reviewing although some code above runs from this the rest is actually doing nothing/getting overwritten from Patch2.
*/
void *Patch12(void *pthis, void *a2, void *a3, void *a4)
{
	UpdateShadowNodes(true);
	void *ret = Patch12Orig(pthis, a2, a3, a4);
	UpdateShadowNodes(false);

	return ret;
}

/*void *Patch12(void *pthis)
{
	UpdateShadowNodes(true);
	return Patch12Orig(pthis);
}*/

void DoPatches()
{
	g_branchTrampoline.Create(1024 * 64);
	g_localTrampoline.Create(1024 * 64);
	
	// /// Patch 1
	PatchUtils::HookCall((void *)Patch1_Address.GetUIntPtr(), (void **)&Patch1Orig, Patch1);
	// ///

	// /// Patch 2
	struct Patch2_Code : Xbyak::CodeGenerator {
		Patch2_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			lea(rdx, ptr[rsp + 0x40]);
			mov(rax, (uintptr_t)Patch2);
			call(rax);

			// Return to original code
			jmp(ptr[rip]);
			dq(Patch2_Address.GetUIntPtr() + 0x8);
		}
	};

	void * codeBuf = g_localTrampoline.StartAlloc();
	Patch2_Code code2(codeBuf);
	g_localTrampoline.EndAlloc(code2.getCurr());

	g_branchTrampoline.Write6Branch(Patch2_Address.GetUIntPtr(), uintptr_t(code2.getCode()));
	SafeWrite16(Patch2_Address.GetUIntPtr() + 6, 0x9090); 
	// ///

	// /// Patch 3
	PatchUtils::HookCall((void *)Patch3_Address.GetUIntPtr(), nullptr, Patch3);
	// ///

	if (bEnableFirstPersonBody)
	{
		// /// Patch 25
		SafeWrite8(Patch25_Address.GetUIntPtr(), 0xB8); // mov eax, 0xe (1)
		SafeWrite32(Patch25_Address.GetUIntPtr() + 1, 0xE); // mov eax, 0xe (2)
		SafeWrite8(Patch25_Address.GetUIntPtr() + 5, 0xB9); // mov ecx, 0xe (1)
		SafeWrite32(Patch25_Address.GetUIntPtr() + 6, 0xE); // mov ecx, 0xe (2)
		SafeWrite16(Patch25_Address.GetUIntPtr() + 10, 0x9090);
		SafeWrite32(Patch25_Address.GetUIntPtr() + 12, 0x90909090);

		SafeWrite16(Patch25_Address2.GetUIntPtr(), 0x9090);
		// ///

		// /// Patch 4
		g_branchTrampoline.Write6Call(Patch4_Address.GetUIntPtr(), (uintptr_t)Patch4);
		// ///

		// /// Patch 5
		SafeWrite32(Patch5_Address.GetUIntPtr(), 0x90909090);
		SafeWrite16(Patch5_Address.GetUIntPtr()+4, 0x9090);

		PatchUtils::HookCall((void *)Patch5_Address2.GetUIntPtr(), (void **)&Patch5Orig, Patch5);
		// ///

		// /// Patch 6
		PatchUtils::HookCall((void *)Patch6_Address.GetUIntPtr(), (void **)&Patch6Orig, Patch6);
		// ///
	}

	if (bSmoothAnimationTransitions)
	{
		// /// Patch 7
		g_branchTrampoline.Write6Call(Patch7_Address.GetUIntPtr(), (uintptr_t)Patch7);
		// ///
	}

	// /// Patch8
	struct Patch8_Code : Xbyak::CodeGenerator {
		Patch8_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			sub(rsp, 0x18); // Remainder, that after adding the 8 bytes of call, the stack must remain 16 bytes aligned
			// Only need to preserve these two
			movss(dword[rsp], xmm0);
			mov(qword[rsp + 8], rdx);

			mov(rax, (uintptr_t)Patch8);
			call(rax);
			
			movss(xmm1, dword[rsp]);
			mov(rdx, qword[rsp + 8]);
			add(rsp, 0x18);

			comiss(xmm1, xmm0);
			movss(xmm0, xmm1);

			// Return to original code
			jmp(ptr[rip]);
			dq(Patch8_Address.GetUIntPtr() + 0x7);
		}
	};

	codeBuf = g_localTrampoline.StartAlloc();
	Patch8_Code code8(codeBuf);
	g_localTrampoline.EndAlloc(code8.getCurr());

	g_branchTrampoline.Write6Branch(Patch8_Address.GetUIntPtr(), uintptr_t(code8.getCode()));
	SafeWrite8(Patch8_Address.GetUIntPtr() + 6, 0x90);
	// ///

	// /// Patch 9
	PatchUtils::HookCall((void *)Patch9_Address.GetUIntPtr(), (void **)&Patch9Orig, Patch9);
	// ///

	// /// Patch 10
	PatchUtils::HookCall((void *)Patch10_Address.GetUIntPtr(), nullptr, Patch10);
	// ///

	// /// Patch 11
	PatchUtils::Hook((void *)Patch11_Address.GetUIntPtr(), (void **)&Patch11Orig, Patch11);
	// /// 

	// /// Patch 12
	PatchUtils::HookCall((void *)Patch12_Address.GetUIntPtr(), (void **)&Patch12Orig, Patch12);
	// /// 

	// Patch 14
	struct Patch14_Code : Xbyak::CodeGenerator {
		Patch14_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			Xbyak::Label L1;
			
			mov(rbx, rax); // rbx is not longer used, and it is a saved reg
			mov(rax, (uintptr_t)Patch14_Func);
			call(rax);
			test(al, al);
			mov(rax, rbx);
			jnz(L1);

			// Original code
			mov(ecx, dword[rdi]);
			mov(dword[rax + 0x6C], ecx);

			// Return to original code
			jmp(ptr[rip]);
			dq(Patch14_Address.GetUIntPtr() + 0x5);

			// Return a bit more
			L(L1);
			jmp(ptr[rip]);
			dq(Patch14_Address.GetUIntPtr() + 0x11);
		}
	};

	codeBuf = g_localTrampoline.StartAlloc();
	Patch14_Code code14(codeBuf);
	g_localTrampoline.EndAlloc(code14.getCurr());

	g_branchTrampoline.Write5Branch(Patch14_Address.GetUIntPtr(), uintptr_t(code14.getCode()));
	// ///

	if (bUseThirdPersonArms)
	{
		// /// Patch 26
		// Patch in an ActorMagicCaster member
		SafeWrite32(Patch26_Address.GetUIntPtr(), 0x9090D231); // xor edx, edx; 2-nop
		// ///

		// /// Patch 27
		SafeWrite32(Patch27_Address.GetUIntPtr(), 0x90909090);
		SafeWrite16(Patch27_Address.GetUIntPtr() + 4, 0x9090);
		////

		// /// Patch 28
		SafeWrite32(Patch28_Address.GetUIntPtr(), 0x90909090);
		SafeWrite16(Patch28_Address.GetUIntPtr() + 4, 0x9090);
		////
	}

	if (bFixEnchantmentArt)
	{
		// /// Patch 15
		// Patch in a member of ModelReferenceEffect
		struct Patch15_Code : Xbyak::CodeGenerator {
			Patch15_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				mov(rdx, rbx); // Additional param
				mov(rax, (uintptr_t)Patch15);
				call(rax);

				// Return to original code
				jmp(ptr[rip]);
				dq(Patch15_Address.GetUIntPtr() + 0x5);
			}
		};

		codeBuf = g_localTrampoline.StartAlloc();
		Patch15_Code code15(codeBuf);
		g_localTrampoline.EndAlloc(code15.getCurr());

		g_branchTrampoline.Write5Branch(Patch15_Address.GetUIntPtr(), uintptr_t(code15.getCode()));
		// ///

		// /// Patch 16
		// Patch in a(nother) member of ModelReferenceEffect
		struct Patch16_Code : Xbyak::CodeGenerator {
			Patch16_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
			{
				mov(rdx, rbx); // Additional param
				mov(rax, (uintptr_t)Patch16);
				call(rax);

				// Return to original code
				jmp(ptr[rip]);
				dq(Patch16_Address.GetUIntPtr() + 0x5);
			}
		};

		codeBuf = g_localTrampoline.StartAlloc();
		Patch16_Code code16(codeBuf);
		g_localTrampoline.EndAlloc(code16.getCurr());

		g_branchTrampoline.Write5Branch(Patch16_Address.GetUIntPtr(), uintptr_t(code16.getCode()));
		// ///
	}

	// /// Patch 17
	// Patch in a member of ShaderReferenceEffect
	struct Patch17_Code : Xbyak::CodeGenerator {
		Patch17_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			mov(rdx, rbx); // Additional param
			mov(rax, (uintptr_t)Patch17);
			call(rax);

			// Return to original code
			jmp(ptr[rip]);
			dq(Patch17_Address.GetUIntPtr() + 0x5);
		}
	};

	codeBuf = g_localTrampoline.StartAlloc();
	Patch17_Code code17(codeBuf);
	g_localTrampoline.EndAlloc(code17.getCurr());

	g_branchTrampoline.Write5Branch(Patch17_Address.GetUIntPtr(), uintptr_t(code17.getCode()));
	// ///

	// /// Patch 18
	// Patch in a member of ShaderReferenceEffect (same member than Patch 17)
	struct Patch18_Code : Xbyak::CodeGenerator {
		Patch18_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			mov(rdx, rbx); // Additional param
			mov(rax, (uintptr_t)Patch18);
			call(rax);

			// Return to original code
			jmp(ptr[rip]);
			dq(Patch18_Address.GetUIntPtr() + 0x6);
		}
	};

	codeBuf = g_localTrampoline.StartAlloc();
	Patch18_Code code18(codeBuf);
	g_localTrampoline.EndAlloc(code18.getCurr());

	g_branchTrampoline.Write6Branch(Patch18_Address.GetUIntPtr(), uintptr_t(code18.getCode()));
	// ///

	if (fSittingMaxLookingDownOverride != 40.0)
		*fSittingMaxLookingDown = fSittingMaxLookingDownOverride;

	// /// Patch 19
	PatchUtils::HookCall((void *)Patch19_Address.GetUIntPtr(), (void **)&Patch19Orig, Patch19);
	// /// 19

	// /// Patch 20
	PatchUtils::HookCall((void *)Patch20_Address.GetUIntPtr(), (void **)&Patch20Orig, Patch20);
	// /// 

	// /// Patch 21
	PatchUtils::HookCall((void *)Patch21_Address.GetUIntPtr(), (void **)&Patch21Orig, Patch21);
	// /// 

	// /// Patch 22
	PatchUtils::HookCall((void *)Patch22_Address.GetUIntPtr(), nullptr, Patch22);
	// /// 

	// /// Patch 23
	struct Patch23_Code : Xbyak::CodeGenerator {
		Patch23_Code(void * buf) : Xbyak::CodeGenerator(4096, buf)
		{
			mov(rcx, rbx); // Additional param
			mov(rax, (uintptr_t)Patch23);
			call(rax);

			// Return to original code
			jmp(ptr[rip]);
			dq(Patch23_Address.GetUIntPtr() + 0x5);
		}
	};

	codeBuf = g_localTrampoline.StartAlloc();
	Patch23_Code code23(codeBuf);
	g_localTrampoline.EndAlloc(code23.getCurr());

	g_branchTrampoline.Write5Branch(Patch23_Address.GetUIntPtr(), uintptr_t(code23.getCode()));
	// ///

	// /// Patch 24
	// replace second parameter with our parameter. Since it is always 1, we can restore that later in original call.
	// mov r9b, 1 -> mov r9, rdi (same length)
	SafeWrite8(Patch24_Address.GetUIntPtr(), 0x49);
	SafeWrite8(Patch24_Address.GetUIntPtr()+1, 0x89);
	SafeWrite8(Patch24_Address.GetUIntPtr()+2, 0xF9);

	PatchUtils::HookCall((void *)Patch24_Address2.GetUIntPtr(), (void **)&Patch24Orig, Patch24);
	// ///
}

extern "C"
{

bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
{
#ifdef LOGME
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\ImprovedCamera.log");
	gLog.SetPrintLevel(IDebugLog::kLevel_Error);
	gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);
#endif
	
	_MESSAGE("Running query");
	_MESSAGE("Runtime: %08X; SKSE: %08X", skse->runtimeVersion, skse->skseVersion);

	/*if (skse->runtimeVersion != RUNTIME_VERSION)
	{
		_MESSAGE("This plugin is not compatible with this version of game.");
		return false;
	}*/

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "ImprovedCamera";
	info->version = 1;		

	if (skse->isEditor)
	{
		_MESSAGE("loaded in editor, marking as incompatible");
		return false;
	}

	return true;
}

bool SKSEPlugin_Load(const SKSEInterface * skse)
{
	_MESSAGE("Running load");

	ReadConfig();
	DoPatches();

	return true;
}

} // extern "C"