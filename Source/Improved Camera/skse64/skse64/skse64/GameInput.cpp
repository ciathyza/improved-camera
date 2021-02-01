#include "GameInput.h"

bool InputEventDispatcher::IsGamepadEnabled(void)
{
	return (gamepad != NULL) && gamepad->IsEnabled();
}

InputEventDispatcher* InputEventDispatcher::GetSingleton()
{
	// 5202C5E930BB4CD8F477F91C9434AB37DBDB10B3+7A
	static RelocPtrEx<InputEventDispatcher*> g_inputEventDispatcher(0x50E33B, "C7 45 F8 40 00 00 00 - 48 8D 05 XXXXXXXX - 48 89 45 E0 - 48 8B 0D XXXXXXXX", 0x15, true); // 2F4C7A8
	return *g_inputEventDispatcher;
}

InputManager * InputManager::GetSingleton(void)
{
	// 61FAE6E8975F0FA7B3DD4D5A410A240E86A58F7B+E
	static RelocPtrEx<InputManager*> g_inputManager(0x22E20F, "48 8B 05 XXXXXXXX - 8B 88 18 01 00 00 - C1 E9 06", 3, true); // 2EECBD0 Note: two matches for the signature, both will resolve the same var
	return *g_inputManager;
}

UInt8 InputManager::AllowTextInput(bool allow)
{
	if(allow)
	{
		if(allowTextInput == 0xFF)
			_WARNING("InputManager::AllowTextInput: counter overflow");
		else
			allowTextInput++;
	}
	else
	{
		if(allowTextInput == 0)
			_WARNING("InputManager::AllowTextInput: counter underflow");
		else
			allowTextInput--;
	}

	if(IsConsoleMode())
		Console_Print("%s text input, count = %d", allow ? "allowed" : "disallowed", allowTextInput);

	return allowTextInput;
}

UInt32 InputManager::GetMappedKey(BSFixedString name, UInt32 deviceType, UInt32 contextIdx)
{
	ASSERT(contextIdx < kContextCount);

	tArray<InputContext::Mapping> * mappings;
	if (deviceType == kDeviceType_Mouse)
		mappings = &context[contextIdx]->mouseMap;
	else if (deviceType == kDeviceType_Gamepad)
		mappings = &context[contextIdx]->gamepadMap;
	else
		mappings = &context[contextIdx]->keyboardMap;

	for (UInt32 i=0; i < mappings->count; i++)
	{
		InputContext::Mapping m;
		if (!mappings->GetNthItem(i, m))
			break;
		if (m.name == name)
			return m.buttonID;
	}

	// Unbound
	return 0xFF;
}

BSFixedString InputManager::GetMappedControl(UInt32 buttonID, UInt32 deviceType, UInt32 contextIdx)
{
	ASSERT(contextIdx < kContextCount);

	// 0xFF == unbound
	if (buttonID == 0xFF)
		return BSFixedString();

	tArray<InputContext::Mapping> * mappings;
	if (deviceType == kDeviceType_Mouse)
		mappings = &context[contextIdx]->mouseMap;
	else if (deviceType == kDeviceType_Gamepad)
		mappings = &context[contextIdx]->gamepadMap;
	else
		mappings = &context[contextIdx]->keyboardMap;

	for (UInt32 i=0; i < mappings->count; i++)
	{
		InputContext::Mapping m;
		if (!mappings->GetNthItem(i, m))
			break;
		if (m.buttonID == buttonID)
			return m.name;
	}

	return BSFixedString();
}

PlayerControls * PlayerControls::GetSingleton(void)
{
	// F1E82AFF2615653A5A14A2E7C229B4B0466688EF+19
	static RelocPtrEx<PlayerControls*> g_playerControls(0x2FB4C6, "39 5D 23 - 74 0E - 48 8B 0D XXXXXXXX", 8, true); // 2EECBD8
	return *g_playerControls;
}

MenuControls * MenuControls::GetSingleton(void)
{
	// DC378767BEB0312EBDE098BC7E0CE53FCC296377+D9
	static RelocPtrEx<MenuControls*> g_menuControls(0x2FB5E3, "83 7D 17 00 - 74 10 - 48 8B 0D XXXXXXXX", 9, true); // 2F273F8
	return *g_menuControls;
}
