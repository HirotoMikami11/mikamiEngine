#define NOMINMAX
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#pragma comment(lib, "xinput.lib")
#include <cmath>

#include "Input.h"
#include "Managers/ImGui/ImGuiManager.h"
#include <cassert>



Input* Input::GetInstance() {
	static Input instance;
	return &instance;
}

void Input::Initialize(WinApp* winApp) {
	assert(winApp);
	//WindwoAppのポインタを保存
	winApp_ = winApp;


	// DirectInputの初期化
	HRESULT result = DirectInput8Create(
		winApp_->GetHInstance(),
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&directInput,
		nullptr);
	assert(SUCCEEDED(result));

	/// キーボードデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard, &devKeyboard, NULL);
	assert(SUCCEEDED(result));//作成できなかったら停止

	///入力データ形式のセット
	result = devKeyboard->SetDataFormat(&c_dfDIKeyboard);//キーボードの標準形式
	assert(SUCCEEDED(result));//作成できなかったら停止

	///排他制御レベルのセット
	result = devKeyboard->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));//作成できなかったら停止

	/// マウスデバイスの生成
	result = directInput->CreateDevice(GUID_SysMouse, &devMouse, NULL);
	assert(SUCCEEDED(result));//作成できなかったら停止

	///入力データ形式のセット
	result = devMouse->SetDataFormat(&c_dfDIMouse2);
	assert(SUCCEEDED(result));//作成できなかったら停止

	///排他制御レベルのセット
	result = devMouse->SetCooperativeLevel(
		winApp_->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(result));//作成できなかったら停止

	// キーボードの初期化
	devKeyboard->Acquire();
	devKeyboard->GetDeviceState(sizeof(key), key);
	// 前回のキー入力状態を初期化
	memcpy(preKey, key, sizeof(key));

	// マウスの初期化
	devMouse->Acquire();
	devMouse->GetDeviceState(sizeof(mouse), &mouse);
	// 前回のマウス入力状態を初期化
	preMouse = mouse;

	// ゲームパッドの初期化
	for (int i = 0; i < MAX_CONTROLLERS; ++i) {
		ZeroMemory(&gamePadState[i], sizeof(XINPUT_STATE));
		ZeroMemory(&preGamePadState[i], sizeof(XINPUT_STATE));
		gamePadConnected[i] = false;
	}

	Logger::Log(Logger::GetStream(), "Complete Input initialized !!\n");
}

void Input::Finalize() {
	// 全てのコントローラーの振動を停止
	for (int i = 0; i < MAX_CONTROLLERS; ++i) {
		StopGamePadVibration(i);
	}
}

void Input::Update() {
	// 前回のキー入力状態を保存
	memcpy(preKey, key, sizeof(key));

	// キーボード情報の取得開始
	HRESULT result = devKeyboard->Acquire();

	// キーの入力状態を取得
	result = devKeyboard->GetDeviceState(sizeof(key), key);
	if (FAILED(result)) {
		// 失敗したら再取得
		devKeyboard->Acquire();
		devKeyboard->GetDeviceState(sizeof(key), key);
	}

	// 前回のマウス入力状態を保存
	preMouse = mouse;
	preMousePosition = mousePosition;

	// マウス情報の取得開始
	result = devMouse->Acquire();
	// マウスの入力状態を取得
	result = devMouse->GetDeviceState(sizeof(mouse), &mouse);
	if (FAILED(result)) {
		// 失敗したら再取得
		devMouse->Acquire();
		devMouse->GetDeviceState(sizeof(mouse), &mouse);
	}

	// マウス座標の取得（スクリーン座標）
	POINT point;
	GetCursorPos(&point);

	// POINTからVector2に変換
	mousePosition.x = static_cast<float>(point.x);
	mousePosition.y = static_cast<float>(point.y);

	// ゲームパッドの状態更新
	for (int i = 0; i < MAX_CONTROLLERS; ++i) {
		// 前回の状態を保存
		preGamePadState[i] = gamePadState[i];

		// 現在の状態を取得
		DWORD dwResult = XInputGetState(i, &gamePadState[i]);

		// 接続状態を更新
		gamePadConnected[i] = (dwResult == ERROR_SUCCESS);

		// 接続されていない場合は状態をクリア
		if (!gamePadConnected[i]) {
			ZeroMemory(&gamePadState[i], sizeof(XINPUT_STATE));
		}
	}
}

///*-----------------------------------------------------------------------*///
//																			//
///								キーボード関連								   ///
//																			//
///*-----------------------------------------------------------------------*///

bool Input::IsKeyDown(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) != 0;
}

bool Input::IsKeyUp(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) == 0;
}

bool Input::IsKeyTrigger(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) != 0 && (preKey[keyCode] & 0x80) == 0;
}

bool Input::IsKeyRelease(uint8_t keyCode) const {
	return (key[keyCode] & 0x80) == 0 && (preKey[keyCode] & 0x80) != 0;
}

///*-----------------------------------------------------------------------*///
//																			//
///								マウス関連								   ///
//																			//
///*-----------------------------------------------------------------------*///

bool Input::IsMouseButtonDown(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) != 0;
}

bool Input::IsMouseButtonUp(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return true;  // 範囲外なら押されていないとみなす
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) == 0;
}

bool Input::IsMouseButtonTrigger(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) != 0 &&
		(preMouse.rgbButtons[buttonNumber] & 0x80) == 0;
}

bool Input::IsMouseButtonRelease(int buttonNumber) const {
	// ボタン番号の範囲チェック（0〜7）
	if (buttonNumber < 0 || buttonNumber >= 8) {
		return false;
	}
	return (mouse.rgbButtons[buttonNumber] & 0x80) == 0 &&
		(preMouse.rgbButtons[buttonNumber] & 0x80) != 0;
}

bool Input::IsMoveMouseWheel() const {
	//前フレームの情報が今の座標と違う場合にtrue
	if (preMouse.lZ != mouse.lZ) {
		return true;
	}
	return false;
}

///*-----------------------------------------------------------------------*///
//																			//
///							ゲームパッド関連								   ///
//																			//
///*-----------------------------------------------------------------------*///

bool Input::IsGamePadConnected(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	return gamePadConnected[controllerIndex];
}

bool Input::IsGamePadButtonDown(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}
	return (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;
}

bool Input::IsGamePadButtonUp(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return true;
	}
	if (!gamePadConnected[controllerIndex]) {
		return true;
	}
	return (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) == 0;
}

bool Input::IsGamePadButtonTrigger(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	bool currentPressed = (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;
	bool prevPressed = (preGamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;

	return currentPressed && !prevPressed;
}

bool Input::IsGamePadButtonRelease(GamePadButton button, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	bool currentPressed = (gamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;
	bool prevPressed = (preGamePadState[controllerIndex].Gamepad.wButtons & static_cast<WORD>(button)) != 0;

	return !currentPressed && prevPressed;
}

float Input::GetAnalogStick(AnalogStick stick, int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return 0.0f;
	}
	if (!gamePadConnected[controllerIndex]) {
		return 0.0f;
	}

	const XINPUT_GAMEPAD& gamepad = gamePadState[controllerIndex].Gamepad;

	switch (stick) {
	case AnalogStick::LEFT_X:
		return ApplyDeadZone(gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	case AnalogStick::LEFT_Y:
		return ApplyDeadZone(gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	case AnalogStick::RIGHT_X:
		return ApplyDeadZone(gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	case AnalogStick::RIGHT_Y:
		return ApplyDeadZone(gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	default:
		return 0.0f;
	}
}

float Input::GetLeftTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return 0.0f;
	}
	if (!gamePadConnected[controllerIndex]) {
		return 0.0f;
	}

	BYTE triggerValue = gamePadState[controllerIndex].Gamepad.bLeftTrigger;

	// デッドゾーン処理
	if (triggerValue < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		return 0.0f;
	}

	// 0.0f ~ 1.0f に正規化
	return static_cast<float>(triggerValue) / 255.0f;
}

float Input::GetRightTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return 0.0f;
	}
	if (!gamePadConnected[controllerIndex]) {
		return 0.0f;
	}

	BYTE triggerValue = gamePadState[controllerIndex].Gamepad.bRightTrigger;

	// デッドゾーン処理
	if (triggerValue < XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		return 0.0f;
	}

	// 0.0f ~ 1.0f に正規化
	return static_cast<float>(triggerValue) / 255.0f;
}





bool Input::IsLeftTriggerDown(int controllerIndex) const {
	return GetLeftTrigger(controllerIndex) >= TRIGGER_BUTTON_THRESHOLD;
}

bool Input::IsRightTriggerDown(int controllerIndex) const {
	return GetRightTrigger(controllerIndex) >= TRIGGER_BUTTON_THRESHOLD;
}

bool Input::IsLeftTriggerTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetLeftTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bLeftTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return currentPressed && !prevPressed;
}

bool Input::IsRightTriggerTrigger(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetRightTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bRightTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return currentPressed && !prevPressed;
}

bool Input::IsLeftTriggerRelease(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetLeftTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bLeftTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return !currentPressed && prevPressed;
}

bool Input::IsRightTriggerRelease(int controllerIndex) const {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return false;
	}
	if (!gamePadConnected[controllerIndex]) {
		return false;
	}

	// 現在の状態
	float currentTrigger = GetRightTrigger(controllerIndex);
	bool currentPressed = currentTrigger >= TRIGGER_BUTTON_THRESHOLD;

	// 前フレームの状態を計算
	BYTE prevTriggerValue = preGamePadState[controllerIndex].Gamepad.bRightTrigger;
	float prevTrigger = 0.0f;
	if (prevTriggerValue >= XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
		prevTrigger = static_cast<float>(prevTriggerValue) / 255.0f;
	}
	bool prevPressed = prevTrigger >= TRIGGER_BUTTON_THRESHOLD;

	return !currentPressed && prevPressed;
}

void Input::SetGamePadVibration(float leftMotor, float rightMotor, int controllerIndex) {
	if (controllerIndex < 0 || controllerIndex >= MAX_CONTROLLERS) {
		return;
	}
	if (!gamePadConnected[controllerIndex]) {
		return;
	}

	// 値を0.0f ~ 1.0f にクランプ
	leftMotor = std::max(0.0f, std::min(1.0f, leftMotor));
	rightMotor = std::max(0.0f, std::min(1.0f, rightMotor));

	XINPUT_VIBRATION vibration;
	vibration.wLeftMotorSpeed = static_cast<WORD>(leftMotor * 65535.0f);
	vibration.wRightMotorSpeed = static_cast<WORD>(rightMotor * 65535.0f);

	XInputSetState(controllerIndex, &vibration);
}

void Input::StopGamePadVibration(int controllerIndex) {
	SetGamePadVibration(0.0f, 0.0f, controllerIndex);
}
// Input.cppの既存ImGui()関数を以下に置き換え

void Input::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Input")) {
		// キーボード表示
		ImGuiKeyboard();

		ImGui::Separator();
		ImGui::Spacing();

		// マウス表示
		ImGuiMouse();

		ImGui::Separator();
		ImGui::Spacing();

		// ゲームパッド表示
		ImGuiGamePad();
	}
#endif
}

const char* Input::GetKeyName(uint8_t keyCode) const {
	// DirectInputキーコードから文字列への変換

	switch (keyCode) {
		// アルファベット
	case DIK_A: return "A";
	case DIK_B: return "B";
	case DIK_C: return "C";
	case DIK_D: return "D";
	case DIK_E: return "E";
	case DIK_F: return "F";
	case DIK_G: return "G";
	case DIK_H: return "H";
	case DIK_I: return "I";
	case DIK_J: return "J";
	case DIK_K: return "K";
	case DIK_L: return "L";
	case DIK_M: return "M";
	case DIK_N: return "N";
	case DIK_O: return "O";
	case DIK_P: return "P";
	case DIK_Q: return "Q";
	case DIK_R: return "R";
	case DIK_S: return "S";
	case DIK_T: return "T";
	case DIK_U: return "U";
	case DIK_V: return "V";
	case DIK_W: return "W";
	case DIK_X: return "X";
	case DIK_Y: return "Y";
	case DIK_Z: return "Z";

		// 数字
	case DIK_0: return "0";
	case DIK_1: return "1";
	case DIK_2: return "2";
	case DIK_3: return "3";
	case DIK_4: return "4";
	case DIK_5: return "5";
	case DIK_6: return "6";
	case DIK_7: return "7";
	case DIK_8: return "8";
	case DIK_9: return "9";

		// ファンクションキー
	case DIK_F1: return "F1";
	case DIK_F2: return "F2";
	case DIK_F3: return "F3";
	case DIK_F4: return "F4";
	case DIK_F5: return "F5";
	case DIK_F6: return "F6";
	case DIK_F7: return "F7";
	case DIK_F8: return "F8";
	case DIK_F9: return "F9";
	case DIK_F10: return "F10";
	case DIK_F11: return "F11";
	case DIK_F12: return "F12";

		// 矢印キー
	case DIK_UP: return "UP";
	case DIK_DOWN: return "DOWN";
	case DIK_LEFT: return "LEFT";
	case DIK_RIGHT: return "RIGHT";

		// 特殊キー
	case DIK_ESCAPE: return "ESC";
	case DIK_RETURN: return "ENTER";
	case DIK_SPACE: return "SPACE";
	case DIK_BACK: return "BACKSPACE";
	case DIK_TAB: return "TAB";
	case DIK_LSHIFT: return "LSHIFT";
	case DIK_RSHIFT: return "RSHIFT";
	case DIK_LCONTROL: return "LCTRL";
	case DIK_RCONTROL: return "RCTRL";
	case DIK_LALT: return "LALT";
	case DIK_RALT: return "RALT";
	case DIK_LWIN: return "LWIN";
	case DIK_RWIN: return "RWIN";

		// その他
	case DIK_CAPSLOCK: return "CAPSLOCK";
	case DIK_NUMLOCK: return "NUMLOCK";
	case DIK_SCROLL: return "SCROLL";
	case DIK_INSERT: return "INSERT";
	case DIK_DELETE: return "DELETE";
	case DIK_HOME: return "HOME";
	case DIK_END: return "END";
	case DIK_PRIOR: return "PAGEUP";
	case DIK_NEXT: return "PAGEDOWN";

		// テンキー
	case DIK_NUMPAD0: return "NUM0";
	case DIK_NUMPAD1: return "NUM1";
	case DIK_NUMPAD2: return "NUM2";
	case DIK_NUMPAD3: return "NUM3";
	case DIK_NUMPAD4: return "NUM4";
	case DIK_NUMPAD5: return "NUM5";
	case DIK_NUMPAD6: return "NUM6";
	case DIK_NUMPAD7: return "NUM7";
	case DIK_NUMPAD8: return "NUM8";
	case DIK_NUMPAD9: return "NUM9";
	case DIK_MULTIPLY: return "NUM*";
	case DIK_ADD: return "NUM+";
	case DIK_SUBTRACT: return "NUM-";
	case DIK_DECIMAL: return "NUM.";
	case DIK_DIVIDE: return "NUM/";
	case DIK_NUMPADENTER: return "NUMENTER";

		// 記号
	case DIK_MINUS: return "-";
	case DIK_EQUALS: return "=";
	case DIK_LBRACKET: return "[";
	case DIK_RBRACKET: return "]";
	case DIK_SEMICOLON: return ";";
	case DIK_APOSTROPHE: return "'";
	case DIK_GRAVE: return "`";
	case DIK_BACKSLASH: return "\\";
	case DIK_COMMA: return ",";
	case DIK_PERIOD: return ".";
	case DIK_SLASH: return "/";

	default:
		// 不明なキーコードは16進数で表示
		static char buffer[16];
		sprintf_s(buffer, "0x%02X", keyCode);
		return buffer;
	}
}

void Input::ImGuiKeyboard()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Keyboard")) {

		//今押されているキー
		ImGui::Text("Currently Pressed Keys:");
		ImGui::Indent();
		bool foundPressed = false;
		int count = 0;
		for (int i = 0; i < 256; ++i) {
			// 0x94を除外
			if (i == 0x94) continue;

			if (IsKeyDown(static_cast<uint8_t>(i))) {
				if (count > 0 && count % 6 == 0) {
					// 6個ごとに改行
					ImGui::Text("");
				}
				if (count % 6 != 0) {
					ImGui::SameLine();
				}
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", GetKeyName(static_cast<uint8_t>(i)));
				foundPressed = true;
				count++;
			}
		}
		if (!foundPressed) {
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1), "None");
		}
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		//最後にトリガーされたキー
		static char lastTriggerKey[64] = "None";

		// トリガーをチェック
		for (int i = 0; i < 256; ++i) {
			// 0x94を除外
			if (i == 0x94) continue;

			if (IsKeyTrigger(i)) {
				sprintf_s(lastTriggerKey, "%s", GetKeyName(static_cast<uint8_t>(i)));
			}
		}

		ImGui::Text("Last Trigger:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 1, 0, 1), "%s", lastTriggerKey);

		ImGui::Spacing();

		//最後に書き換えたキー
		static char lastReleaseKey[64] = "None";

		// リリースをチェック
		for (int i = 0; i < 256; ++i) {
			// 0x94を除外
			if (i == 0x94) continue;

			if (IsKeyRelease(i)) {
				sprintf_s(lastReleaseKey, "%s", GetKeyName(static_cast<uint8_t>(i)));
			}
		}

		ImGui::Text("Last Release:");
		ImGui::SameLine();
		ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "%s", lastReleaseKey);

		ImGui::TreePop();
	}
#endif
}

void Input::ImGuiMouse()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("Mouse")) {
		// マウスボタンの状態
		ImGui::Text("Mouse Buttons:");
		ImGui::SameLine();
		if (IsMouseButtonDown(0)) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "LEFT");
		} else {
			ImGui::Text("LEFT");
		}
		ImGui::SameLine();
		if (IsMouseButtonDown(1)) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "RIGHT");
		} else {
			ImGui::Text("RIGHT");
		}
		ImGui::SameLine();
		if (IsMouseButtonDown(2)) {
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "MIDDLE");
		} else {
			ImGui::Text("MIDDLE");
		}

		// 拡張マウスボタン
		bool anyExtendedButton = false;
		for (int i = 3; i < 8; ++i) {
			if (IsMouseButtonDown(i)) {
				anyExtendedButton = true;
				break;
			}
		}

		if (anyExtendedButton) {
			ImGui::Text("Extended Buttons:");
			for (int i = 3; i < 8; ++i) {
				ImGui::SameLine();
				if (IsMouseButtonDown(i)) {
					ImGui::TextColored(ImVec4(0, 1, 0, 1), "X%d", i - 2);
				}
			}
		}

		ImGui::Separator();

		// マウス座標
		Vector2 mousePos = GetMousePosition();
		Vector2 preMousePos = GetPreMousePosition();
		ImGui::Text("Position: (%.0f, %.0f)", mousePos.x, mousePos.y);
		ImGui::Text("Previous: (%.0f, %.0f)", preMousePos.x, preMousePos.y);

		// マウス移動量
		ImGui::Separator();
		ImGui::Text("Movement (Delta):");
		int moveX = GetMouseMoveX();
		int moveY = GetMouseMoveY();
		ImGui::Text("X: %d", moveX);
		ImGui::SameLine();

		ImGui::Text("Y: %d", moveY);
		ImGui::SameLine();


		// マウスホイール
		ImGui::Separator();
		int wheelDelta = GetMouseWheel();
		ImGui::Text("Wheel Delta: %d", wheelDelta);
		if (IsMoveMouseWheel()) {
			if (wheelDelta > 0) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(0, 1, 1, 1), "[UP]");
			} else if (wheelDelta < 0) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "[DOWN]");
			}
		}

		// トリガー/リリースイベント
		ImGui::Separator();
		ImGui::Text("Button Events:");

		// トリガーイベント
		if (IsMouseButtonTrigger(0)) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "L-TRIGGER");
		}
		if (IsMouseButtonTrigger(1)) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "R-TRIGGER");
		}
		if (IsMouseButtonTrigger(2)) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(0, 1, 0, 1), "M-TRIGGER");
		}

		// リリースイベント
		if (IsMouseButtonRelease(0)) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "L-RELEASE");
		}
		if (IsMouseButtonRelease(1)) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "R-RELEASE");
		}
		if (IsMouseButtonRelease(2)) {
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "M-RELEASE");
		}


		ImGui::TreePop();
	}
#endif
}

void Input::ImGuiGamePad()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode("GamePad")) {
		// ゲームパッドが接続されているか確認
		bool anyConnected = false;
		for (int i = 0; i < MAX_CONTROLLERS; ++i) {
			if (IsGamePadConnected(i)) {
				anyConnected = true;
				break;
			}
		}

		// ゲームパッドが接続されていなかったら早期リターンする
		if (!anyConnected) {
			ImGui::Text("Not GamePad Connected");
			ImGui::TreePop();
			return;
		}

		// 現在入力されている信号を表示
		for (int controllerIndex = 0; controllerIndex < MAX_CONTROLLERS; ++controllerIndex) {
			if (!IsGamePadConnected(controllerIndex)) {
				continue;
			}

			ImGui::Text("Controller No. %d Player", controllerIndex);

			ImGui::Separator();
#pragma region "ボタン表示"

			// ボタン表示
			ImGui::Text("Buttons");
			// 十字キー
			ImGui::Text("D-Pad:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::UP, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "UP");
			} else {
				ImGui::Text("UP");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::DOWN, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "DOWN");
			} else {
				ImGui::Text("DOWN");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::LEFT, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "LEFT");
			} else {
				ImGui::Text("LEFT");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::RIGHT, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "RIGHT");
			} else {
				ImGui::Text("RIGHT");
			}

			// ABXYボタン
			ImGui::Text("ABXY:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::A, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "A");
			} else {
				ImGui::Text("A");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::B, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "B");
			} else {
				ImGui::Text("B");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::X, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "X");
			} else {
				ImGui::Text("X");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::Y, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "Y");
			} else {
				ImGui::Text("Y");
			}
			// Stickボタン
			ImGui::Text("Stick:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::LS, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "LS");
			} else {
				ImGui::Text("LS");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::RS, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "RS");
			} else {
				ImGui::Text("RS");
			}
			// ショルダーボタン
			ImGui::Text("Shoulder:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::LB, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "LB");
			} else {
				ImGui::Text("LB");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::RB, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "RB");
			} else {
				ImGui::Text("RB");
			}

			// その他のボタン
			ImGui::Text("Other:");
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::START, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "START");
			} else {
				ImGui::Text("START");
			}
			ImGui::SameLine();
			if (IsGamePadButtonDown(GamePadButton::BACK, controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "BACK");
			} else {
				ImGui::Text("BACK");
			}
#pragma endregion
			ImGui::Separator();
#pragma region "スティック表示"

			// スティック表示（横並び）
			ImGui::Text("Analog Sticks");

			// 左スティック
			ImGui::BeginGroup();
			ImGui::Text("Left Stick");
			float lx = GetAnalogStick(AnalogStick::LEFT_X, controllerIndex);
			float ly = GetAnalogStick(AnalogStick::LEFT_Y, controllerIndex);
			DrawJoystickVisualizer(lx, ly);
			ImGui::Text("X: %.3f", lx);
			ImGui::Text("Y: %.3f", ly);
			ImGui::EndGroup();

			// 右スティックを同じ行に
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 20.0f);

			ImGui::BeginGroup();
			ImGui::Text("Right Stick");
			float rx = GetAnalogStick(AnalogStick::RIGHT_X, controllerIndex);
			float ry = GetAnalogStick(AnalogStick::RIGHT_Y, controllerIndex);
			DrawJoystickVisualizer(rx, ry);
			ImGui::Text("X: %.3f", rx);
			ImGui::Text("Y: %.3f", ry);
			ImGui::EndGroup();

#pragma endregion
			ImGui::Separator();
#pragma region "トリガーの具合"
			// トリガー表示（アナログ値＋ボタン判定）
			float leftTrigger = GetLeftTrigger(controllerIndex);
			float rightTrigger = GetRightTrigger(controllerIndex);

			ImGui::Text("Triggers (Analog + Button)");

			// 左トリガー
			ImGui::Text("LT: %.3f", leftTrigger);
			ImGui::SameLine();
			ImGui::ProgressBar(leftTrigger, ImVec2(100, 0));
			ImGui::SameLine();
			if (IsLeftTriggerDown(controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "[PRESSED]");
			} else {
				ImGui::Text("[RELEASED]");
			}

			// 右トリガー
			ImGui::Text("RT: %.3f", rightTrigger);
			ImGui::SameLine();
			ImGui::ProgressBar(rightTrigger, ImVec2(100, 0));
			ImGui::SameLine();
			if (IsRightTriggerDown(controllerIndex)) {
				ImGui::TextColored(ImVec4(0, 1, 0, 1), "[PRESSED]");
			} else {
				ImGui::Text("[RELEASED]");
			}

			// トリガーボタンのTrigger/Release判定表示
			ImGui::Text("Trigger Events:");
			if (IsLeftTriggerTrigger(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "LT-TRIGGER");
			}
			if (IsLeftTriggerRelease(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "LT-RELEASE");
			}
			if (IsRightTriggerTrigger(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 1, 0, 1), "RT-TRIGGER");
			}
			if (IsRightTriggerRelease(controllerIndex)) {
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(1, 0.5f, 0, 1), "RT-RELEASE");
			}
#pragma endregion

			ImGui::Separator();
#pragma region "振動テスト"
			// 振動テスト
			ImGui::Text("Vibration Test");
			static float vibrationLeft = 0.0f;
			static float vibrationRight = 0.0f;

			ImGui::SliderFloat("Left Motor", &vibrationLeft, 0.0f, 1.0f);
			ImGui::SliderFloat("Right Motor", &vibrationRight, 0.0f, 1.0f);

			if (ImGui::Button("Apply Vibration")) {
				SetGamePadVibration(vibrationLeft, vibrationRight, controllerIndex);
			}
			ImGui::SameLine();
			if (ImGui::Button("Stop Vibration")) {
				StopGamePadVibration(controllerIndex);
				vibrationLeft = 0.0f;
				vibrationRight = 0.0f;
			}

			// 複数のコントローラーがある場合の区切り
			if (controllerIndex < MAX_CONTROLLERS - 1 && IsGamePadConnected(controllerIndex + 1)) {
				ImGui::Separator();
				ImGui::Text("");
			}
#pragma endregion
		}

		ImGui::TreePop();
	}
#endif
}
float Input::ApplyDeadZone(SHORT value, SHORT deadZone) const {
	// デッドゾーン処理
	if (std::abs(value) < deadZone) {
		return 0.0f;
	}

	// デッドゾーンを超えた分を0 ~ 1の範囲に正規化
	float normalizedValue;
	if (value > 0) {
		normalizedValue = static_cast<float>(value - deadZone) / static_cast<float>(32767 - deadZone);
	} else {
		normalizedValue = static_cast<float>(value + deadZone) / static_cast<float>(32768 - deadZone);
	}

	// -1.0f ~ 1.0f にクランプ
	return std::max(-1.0f, std::min(1.0f, normalizedValue));
}



void Input::DrawJoystickVisualizer(float lx, float ly) {
#ifdef USEIMGUI
	// 円の半径
	const float radius = 50.0f;
	const float canvas_size = radius * 2.5f;

	// ImGuiに描画領域を確保
	ImGui::Dummy(ImVec2(canvas_size, canvas_size));

	// 描画用の座標を取得
	ImVec2 screen_pos = ImGui::GetItemRectMin();
	ImVec2 center = ImVec2(screen_pos.x + canvas_size * 0.5f, screen_pos.y + canvas_size * 0.5f);

	// 正規化された値（-1.0～1.0）を円内の座標に変換
	ImVec2 stick_pos = ImVec2(
		center.x + lx * radius * 0.9f,  // 0.9倍で円の内側に制限
		center.y - ly * radius * 0.9f   // Y軸反転
	);

	// 描画
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	// 背景円
	draw_list->AddCircleFilled(center, radius, IM_COL32(40, 40, 40, 255));

	// 外周円
	draw_list->AddCircle(center, radius, IM_COL32(200, 200, 200, 255), 32, 2.0f);

	// 十字線
	draw_list->AddLine(
		ImVec2(center.x - radius * 0.8f, center.y),
		ImVec2(center.x + radius * 0.8f, center.y),
		IM_COL32(120, 120, 120, 255), 1.0f
	);
	draw_list->AddLine(
		ImVec2(center.x, center.y - radius * 0.8f),
		ImVec2(center.x, center.y + radius * 0.8f),
		IM_COL32(120, 120, 120, 255), 1.0f
	);

	// デッドゾーン円（実際のデッドゾーンの大きさに合わせる）
	draw_list->AddCircle(center, radius * 0.25f, IM_COL32(255, 255, 255, 255), 16, 1.0f);

	// 中心点
	draw_list->AddCircleFilled(center, 3.0f, IM_COL32(255, 255, 255, 255));

	// スティック位置
	draw_list->AddCircleFilled(stick_pos, 8.0f, IM_COL32(255, 0, 0, 255));
#endif
}