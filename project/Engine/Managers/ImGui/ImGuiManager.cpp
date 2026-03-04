#include "ImGuiManager.h"

#ifdef USEIMGUI
ImFont* ImGuiManager::defaultFont_ = nullptr;
#endif

ImGuiManager* ImGuiManager::GetInstance() {
	static ImGuiManager instance;
	return &instance;
}

void ImGuiManager::Initialize([[maybe_unused]] WinApp* winApp, [[maybe_unused]] DirectXCommon* directXCommon) {

#ifdef USEIMGUI

	// ImGuiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	//ドッキング用の設定
	ImGuiIO& imguiIO = ImGui::GetIO();
	imguiIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// プラットフォームとレンダラーの初期化
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(
		directXCommon->GetDevice(),
		directXCommon->GetSwapChainDesc().BufferCount,
		directXCommon->GetRTVDesc().Format,
		directXCommon->GetSRVDescriptorHeap(),
		directXCommon->GetSRVDescriptorHeap()->GetCPUDescriptorHandleForHeapStart(),
		directXCommon->GetSRVDescriptorHeap()->GetGPUDescriptorHandleForHeapStart()
	);


	// 日本語フォントを読み込む
	LoadJapaneseFont();

	// スタイルを設定
	SetupImGuiStyle();

#endif

}

void ImGuiManager::Finalize() {
#ifdef USEIMGUI

	// ImGuiの終了処理
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
}

void ImGuiManager::Begin() {
#ifdef USEIMGUI

	// ImGuiにフレームが始まることを伝える
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGuiManager::GetInstance()->ShowStyleEditor();
#endif
}

void ImGuiManager::End() {
#ifdef USEIMGUI

	// ImGuiの内部コマンドを生成する
	ImGui::Render();
#endif
}

void ImGuiManager::Draw([[maybe_unused]] ID3D12GraphicsCommandList* commandList) {
#ifdef USEIMGUI
	//実際のDirectXCommon-> GetCommandList()のImGuiの描画コマンドを積む
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#endif
}

void ImGuiManager::SceneName([[maybe_unused]] const char* SceneName)
{
#ifdef USEIMGUI

	// 現在のシーン名を表示
	ImGui::Text("Current Scene: %s", SceneName);
	ImGui::Separator();

#endif
}

void ImGuiManager::ShowStyleEditor([[maybe_unused]] bool* isOpen)
{
#ifdef USEIMGUI
	if (isOpen != nullptr) {
		if (!*isOpen) return;

		if (ImGui::Begin("ImGui Style Editor", isOpen)) {
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
	} else {
		if (ImGui::Begin("ImGui Style Editor")) {
			ImGui::ShowStyleEditor();
			ImGui::End();
		}
	}
#endif
}

void ImGuiManager::LoadJapaneseFont() {
#ifdef USEIMGUI
	ImGuiIO& io = ImGui::GetIO();

	// 日本語グリフ範囲を取得
	const ImWchar* glyphRanges = io.Fonts->GetGlyphRangesJapanese();

	// フォントファイルのパス候補（優先度順）
	const char* fontPaths[] = {
		"C:/Windows/Fonts/meiryo.ttc",			// メイリオ
	};

	const char* selectedFontPath = nullptr;

	// 存在するフォントファイルを探す
	for (const char* path : fontPaths) {
		// ファイルが存在するかチェック
		DWORD fileAttr = GetFileAttributesA(path);
		if (fileAttr != INVALID_FILE_ATTRIBUTES && !(fileAttr & FILE_ATTRIBUTE_DIRECTORY)) {
			selectedFontPath = path;
			break;
		}
	}

	if (selectedFontPath == nullptr) {
		return;
	}

	// デフォルトフォント（18px）
	defaultFont_ = io.Fonts->AddFontFromFileTTF(
		selectedFontPath,
		18.0f,
		nullptr,
		glyphRanges
	);

	// フォントアトラスを構築
	io.Fonts->Build();
#endif
}

void ImGuiManager::SetupImGuiStyle()
{
#ifdef USEIMGUI
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;


	//赤色
#pragma region 赤
//
//	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
//	colors[ImGuiCol_TextDisabled] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
//	colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
//	colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.66f);
//	colors[ImGuiCol_PopupBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.95f);
//	colors[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
//	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
//	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
//	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.26f, 0.10f, 0.90f);
//	colors[ImGuiCol_FrameBgActive] = ImVec4(0.50f, 0.29f, 0.15f, 1.00f);
//	colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
//	colors[ImGuiCol_TitleBgActive] = ImVec4(0.03f, 0.04f, 0.11f, 1.00f);
//	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.04f, 0.11f, 1.00f);
//	colors[ImGuiCol_MenuBarBg] = ImVec4(0.07f, 0.03f, 0.03f, 1.00f);
//	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.62f);
//	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
//	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.85f, 0.70f, 0.40f, 0.90f);
//	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.85f, 0.70f, 0.40f, 0.90f);
//	colors[ImGuiCol_CheckMark] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
//	colors[ImGuiCol_SliderGrab] = ImVec4(0.85f, 0.70f, 0.40f, 0.90f);
//	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.71f, 0.50f, 0.09f, 0.90f);
//	colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.06f, 1.00f);
//	colors[ImGuiCol_ButtonHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
//	colors[ImGuiCol_ButtonActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
//	colors[ImGuiCol_Header] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
//	colors[ImGuiCol_HeaderHovered] = ImVec4(0.40f, 0.10f, 0.10f, 0.90f);
//	colors[ImGuiCol_HeaderActive] = ImVec4(0.50f, 0.15f, 0.15f, 1.00f);
//	colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.11f, 0.05f, 0.25f);
//	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.65f, 0.30f, 0.30f, 0.75f);
//	colors[ImGuiCol_SeparatorActive] = ImVec4(0.75f, 0.35f, 0.35f, 1.00f);
//	colors[ImGuiCol_ResizeGrip] = ImVec4(0.98f, 0.76f, 0.26f, 0.20f);
//	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.98f, 0.26f, 0.26f, 0.67f);
//	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.98f, 0.26f, 0.26f, 0.95f);
//	colors[ImGuiCol_TabHovered] = ImVec4(0.50f, 0.17f, 0.15f, 1.00f);
//	colors[ImGuiCol_Tab] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
//	colors[ImGuiCol_TabSelected] = ImVec4(0.81f, 0.29f, 0.05f, 1.00f);
//	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
//	colors[ImGuiCol_TabDimmed] = ImVec4(0.99f, 0.44f, 0.00f, 0.91f);
//	colors[ImGuiCol_TabDimmedSelected] = ImVec4(1.00f, 0.55f, 0.00f, 0.80f);
//	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
//	colors[ImGuiCol_DockingPreview] = ImVec4(0.98f, 0.45f, 0.26f, 0.70f);
//	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
//	colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.40f, 0.12f, 1.00f);
//	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.50f, 0.18f, 0.18f, 1.00f);
//	colors[ImGuiCol_PlotHistogram] = ImVec4(0.45f, 0.15f, 0.15f, 1.00f);
//	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.55f, 0.20f, 0.20f, 1.00f);
//	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
//	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
//	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
//	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
//	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
//	colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
//	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
//	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
//	colors[ImGuiCol_NavCursor] = ImVec4(0.98f, 0.26f, 0.26f, 1.00f);
//	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
//	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
//	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
//
#pragma endregion 


	//キーボードカラー
#pragma region "keyboardColors"

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.97f, 0.97f, 0.97f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.01f, 0.01f, 0.01f, 0.66f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.03f, 0.03f, 0.03f, 0.95f);
	colors[ImGuiCol_Border] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.01f, 0.02f, 0.06f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.03f, 0.04f, 0.11f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.03f, 0.04f, 0.11f, 1.00f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 0.62f, 0.00f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.62f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_Button] = ImVec4(0.02f, 0.02f, 0.06f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_Header] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.03f, 0.03f, 0.02f, 0.80f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.04f, 0.03f, 0.02f, 0.80f);
	colors[ImGuiCol_Separator] = ImVec4(0.10f, 0.10f, 0.10f, 0.80f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.85f, 0.70f, 0.40f, 0.90f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_Tab] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_TabSelected] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_TabDimmed] = ImVec4(0.03f, 0.04f, 0.16f, 1.00f);
	colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.50f, 0.50f, 0.50f, 0.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.94f, 0.44f, 0.05f, 0.70f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.83f, 0.45f, 0.00f, 0.80f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_TextLink] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavCursor] = ImVec4(0.83f, 0.30f, 0.00f, 0.80f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

#pragma endregion

	// スタイル設定
	style.WindowRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 4.0f;
	style.FrameBorderSize = 1.0f;

	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

#endif
}