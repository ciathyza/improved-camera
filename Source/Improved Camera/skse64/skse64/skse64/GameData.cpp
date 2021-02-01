// (UDP: partial TODO)


#include "skse64/GameData.h"

// 9615953AF64D4A2EB76F7BDE0F38D30B311B1351+40
RelocAddrEx <UInt32*> g_gameTime(0x13A3A0, "2B 05 XXXXXXXX - 3D F4 01 00 00", 2, true); // 2F92950

// AB8F3A77254A4A7EF23D7EA9C232AF6674856B23+2B7
RelocPtrEx <DataHandler *> g_dataHandler(0xFB6E1, "83 7F 14 00 - 75 XX - 48 8B 0D XXXXXXXX", 9, true); // 1EE5428

// 5F417CF4D2EB33C7D6903EA38BB5CDDEF48A6F83+4B
RelocPtr <PersistentFormManager *> g_persistentFormManager(0x01EE5AE8); // UDP: TODO

// 1AF6E849D5819F74A0FAC96D2E9D6AD46900704B+60
RelocPtr <FaceGen *> g_faceGen(0x01EE5B40);  // UDP: TODO

// 6F7C1B16C6DF82EB76AEECA5A719A0A1EC196C45+28
RelocPtr<MagicFavorites *>	g_MagicFavorites(0x02F4D730);  // UDP: TODO

// C744C8E2830617136B1E2036CE7FAA915F0BB6FF+1D
RelocPtr<MenuTopicManager *> g_MenuTopicManager(0x02F26988);  // UDP: TODO

// 72603162CC51078584DECEF18ACB8B61C91A63FC+86
RelocPtr <ActorValueList *> g_actorValueList(0x01EE5418);  // UDP: TODO

// aWerewolfSpell
RelocPtr <DefaultObjectList> g_defaultObjectList(0x01DDBE80);  // UDP: TODO

// 2667BD6593B94B34C30A11FA27E9E27627704C71+AB
RelocPtr <BGSSaveLoadManager *> g_saveLoadManager(0x02F4D740);  // UDP: TODO

// 3A2D747A1809205B8E28BD34724286AF76CF511B+2A
RelocPtr <MiscStatManager>	g_MiscStatManager(0x02EEC4D8);  // UDP: TODO

// A9D0A72CC9E5F85E2169118F999943FD43AF51EA+95
RelocPtr <EquipManager *> g_equipManager(0x02EEB838);  // UDP: TODO

// 60C21F969EDFE69EBC96CEEF9620AAF752E2E28B+2
RelocPtr <RelationshipRanks> g_relationshipRanks(0x01DF9EF8);  // UDP: TODO

RelocAddrEx<_ChangeActorHeadPart> ChangeActorHeadPart(0x003DC5B0, "48 89 5C 24 10 - 48 89 74 24 18 - 57 - 48 83 EC 20 - 48 8B 01 - 49 8B F8 - 48 8B DA  - 48 8B F1");
// E596A4244F8A3A25FD8DB7E62A3904933060BEA8+DD
RelocAddrEx<_GetEitherHandSlot> GetEitherHandSlot(0x00331840, "48 83 EC 28 - E8 XXXXXXXX - 80 B8 95 0B 00 00 00");
// A57D77CB5250B7D84828312B34413A9123EDDD53+35
RelocAddrEx<_GetRightHandSlot> GetRightHandSlot(0x00331810, "48 83 EC 28 - E8 XXXXXXXX - 80 B8 94 0B 00 00 00");
// A9D0A72CC9E5F85E2169118F999943FD43AF51EA+83
RelocAddrEx<_GetLeftHandSlot> GetLeftHandSlot(0x003317E0, "48 83 EC 28 - E8 XXXXXXXX - 80 B8 93 0B 00 00 00"); // Note: two matches for the signature, but the two functions are 100% identical
RelocAddrEx<_LookupActorValueByName> LookupActorValueByName(0x003E1640, "48 89 5C 24 08 - 48 89 6C 24 10 - 48 89 74 24 18 - 57 - 48 83 EC 20 33 FF - 48 8B E9 - 8B F7");
RelocAddrEx<_UpdatePlayerTints> UpdatePlayerTints(0x008B42B0, "40 53 - 48 83 EC 50 - 48 C7 44 24 20 FE FF FF FF - 0F 57 C0 - F3 0F 7F 44 24 28");
RelocAddrEx<_GetActorBaseOverlays> GetActorBaseOverlays(0x00368D20 + 0x66 /* 368D86 */, "48 8B 41 08 - 48 85 C0 - 74 EF - 83 78 10 00 - 74 E9", -0x66); // Note: two matches for the signature, but the two functions are 100% identical
RelocAddrEx<_GetNumActorBaseOverlays> GetNumActorBaseOverlays(0x00368DB0 + 0x66/* 368E16  */, "48 8B 41 08 - 48 85 C0 - 74 EF - 8B 40 10 - 48 83 C4 20", -0x66);

RelocAddrEx<_ApplyMasksToRenderTarget> ApplyMasksToRenderTarget(0x003DB610, "48 8B C4 - 55 56 57 41 54 41 55 41 56 41 57 - 48 81 EC 80 00 00 00 - 48 C7 40 88 FE FF FF FF - 48 89 58 20");

// 0A2FCE1738344AE17FCD2B406BDCAAD46AA64394+DC | +1A
RelocAddr<_UpdateModelSkin> UpdateModelSkin(0x003DC985); // UDP: TODO    // Applies tint to ShaderType 5 nodes
// BFB8C9723EF563C7B5A0E336C4A44311725F8047+F4 | +1A
RelocAddr<_UpdateModelHair> UpdateModelHair(0x003DC9D0); // UDP: TODO // Applies tint to ShaderType 6 nodes
RelocAddr<_UpdateModelFace> UpdateModelFace(0x003DBF90); // UDP: TODO
RelocAddr<_UpdateHarvestModel> UpdateHarvestModel(0x0019D030); // UDP: TODO

RelocAddrEx<_GetRelationshipIndex> GetRelationshipIndex(0x003460C0, "48 83 EC 28 - E8 XXXXXXXX - 48 85 C0 - 74 0C - 8B 40 38");

// C5B21010DCF340FCDDDC7866C50C3D78AEF34CB5+6B
//RelocPtr <bool> g_isGameDataReady(0x058FEAB4);

RelocAddrEx<_HasLOS> HasLOS(0x0091C810, "41 C6 00 00 - 48 85 C9 - 74 3B");

class LoadedModFinder
{
	const char * m_stringToFind;

public:
	LoadedModFinder(const char * str) : m_stringToFind(str) { }

	bool Accept(ModInfo* modInfo)
	{
		return _stricmp(modInfo->name, m_stringToFind) == 0;
	}
};

const ModInfo * DataHandler::LookupModByName(const char * modName)
{
	return modList.modInfoList.Find(LoadedModFinder(modName));
}

SInt32 DataHandler::GetModIndex(const char* modName)
{
	return modList.modInfoList.GetIndexOf(LoadedModFinder(modName));
}

const ModInfo* DataHandler::LookupLoadedModByName(const char* modName)
{
	for(UInt32 i = 0; i < modList.loadedMods.count; i++) {
		ModInfo * modInfo = modList.loadedMods[i];
		if(_stricmp(modInfo->name, modName) == 0)
			return modInfo;
	}

	return nullptr;
}

UInt8 DataHandler::GetLoadedModIndex(const char* modName)
{
	const ModInfo * modInfo = LookupLoadedModByName(modName);
	if(modInfo) {
		return modInfo->modIndex;
	}

	return -1;
}

const ModInfo* DataHandler::LookupLoadedLightModByName(const char* modName)
{
	for (UInt32 i = 0; i < modList.loadedCCMods.count; i++) {
		ModInfo * modInfo = modList.loadedCCMods[i];
		if (_stricmp(modInfo->name, modName) == 0)
			return modInfo;
	}

	return nullptr;
}

UInt16 DataHandler::GetLoadedLightModIndex(const char* modName)
{
	for (UInt32 i = 0; i < modList.loadedCCMods.count; i++) {
		ModInfo * modInfo = modList.loadedCCMods[i];
		if (_stricmp(modInfo->name, modName) == 0)
			return i;
	}

	return -1;
}

DataHandler* DataHandler::GetSingleton()
{
	return *(g_dataHandler.GetPtr());
}

MiscStatManager * MiscStatManager::GetSingleton(void)
{
	return g_MiscStatManager.GetPtr(); 
}

MiscStatManager::MiscStat * MiscStatManager::Get(const char * name)
{
	for (UInt32 i = 0; i < m_stats.count; i++)
	{
		MiscStat	* stat = &m_stats[i];

		if (!_stricmp(name, stat->name))
		{
			return stat;
		}
	}

	return NULL;
}

EquipManager * EquipManager::GetSingleton(void)
{
	return *g_equipManager;
}

PersistentFormManager * PersistentFormManager::GetSingleton(void)
{
	return *g_persistentFormManager;
}

FaceGen * FaceGen::GetSingleton(void)
{
	return *g_faceGen;
}

MagicFavorites * MagicFavorites::GetSingleton()
{
	return *(g_MagicFavorites.GetPtr());
}

void MagicFavorites::ClearHotkey(SInt8 idx)
{
	if (idx < 0 || idx >= hotkeys.count)
		return;

	hotkeys[idx] = NULL;
}

void MagicFavorites::SetHotkey(TESForm * form, SInt8 idx)
{
	if (idx < 0 || idx >= hotkeys.count)
		return;

	SInt8 oldIdx = hotkeys.GetItemIndex(form);
	if (idx == oldIdx)
		return;

	if (IsFavorited(form))
	{
		hotkeys[oldIdx] = NULL;
		hotkeys[idx] = form;
	}
}

TESForm * MagicFavorites::GetSpell(SInt8 idx)
{
	TESForm * form = NULL;
	hotkeys.GetNthItem(idx, form);
	return form;
}

bool MagicFavorites::IsFavorited(TESForm * form)
{
	SInt32 indexOut = -1;
	if (GetSortIndex(spells, form, indexOut) && indexOut != -1)
		return true;
	else
		return false;
}

MenuTopicManager * MenuTopicManager::GetSingleton(void)
{
	return *(g_MenuTopicManager.GetPtr());
}

TESObjectREFR * MenuTopicManager::GetDialogueTarget()
{
	TESObjectREFR * refr = NULL;
	if (talkingHandle == (*g_invalidRefHandle) || talkingHandle == 0)
		return NULL;

	LookupREFRByHandle(&talkingHandle, &refr);
	return refr;
}

ActorValueList * ActorValueList::GetSingleton(void)
{
	return *g_actorValueList;
}

ActorValueInfo * ActorValueList::GetActorValue(UInt32 id)
{
	return (id < kNumActorValues) ? actorValues[id] : NULL;
}

UInt32 ActorValueList::ResolveActorValueByName(const char * name)
{
	UInt32 id = LookupActorValueByName(name);
	if (id >= kNumActorValues)
		return 255;

	return id;
}

DefaultObjectList * DefaultObjectList::GetSingleton(void)
{
	return g_defaultObjectList.GetPtr();
}

void BGSSaveLoadManager::Save(const char * name)
{
	CALL_MEMBER_FN(this, Save_Internal)(2, 0, name);
}

void BGSSaveLoadManager::Load(const char * name)
{
	CALL_MEMBER_FN(this, Load_Internal)(name, -1, 0, 1);
}

BGSSaveLoadManager * BGSSaveLoadManager::GetSingleton(void)
{
	return *g_saveLoadManager;
}

RelationshipRanks * RelationshipRanks::GetSingleton(void)
{
	return g_relationshipRanks;
}

SInt32 RelationshipRanks::GetRelationshipRank(TESForm * form1, TESForm * form2)
{
	RelationshipRanks * ranks = RelationshipRanks::GetSingleton();
	if (form1 && form2) {
		return ranks->value[GetRelationshipIndex(form1, form2)];
	}

	return ranks->value[kRelationshipAcquaintance];
}
