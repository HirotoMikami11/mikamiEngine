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


	// ==============================
	// カラーパレット
	// ==============================
	const ImVec4 text(0.82f, 0.82f, 0.82f, 1.00f);
	const ImVec4 textDisabled(0.50f, 0.50f, 0.50f, 1.00f);

	// 青みのある暗色背景
	const ImVec4 bgDark(0.14f, 0.142f, 0.149f, 0.865f);
	const ImVec4 bg(0.11f, 0.12f, 0.14f, 1.00f);
	const ImVec4 bgMid(0.16f, 0.17f, 0.19f, 1.00f);
	const ImVec4 bgLight(0.22f, 0.23f, 0.26f, 1.00f);

	// Blender風オレンジ
	//const ImVec4 orange(0.90f, 0.48f, 0.15f, 1.00f);
	//const ImVec4 orangeHover(1.00f, 0.58f, 0.20f, 1.00f);
	const ImVec4 orangeActive(0.80f, 0.40f, 0.10f, 1.00f);

	const ImVec4 blenderOrange(0.83f, 0.3f, 0.0f, 1.0f);			// 選択されている濃い目のオレンジ
	const ImVec4 blenderOrangeMid(0.915f, 0.44f, 0.0f, 0.8f);		// 非選択時の薄いオレンジ
	const ImVec4 blenderOrangePreview(0.96f, 0.51f, 0.26f, 0.644f);	// 透明オレンジ



	// ==============================
	// Text
	// ==============================
	colors[ImGuiCol_Text] = text;
	colors[ImGuiCol_TextDisabled] = textDisabled;
	colors[ImGuiCol_TextLink] = blenderOrangeMid;

	// ==============================
	// Backgrounds
	// ==============================
	colors[ImGuiCol_WindowBg] = bg;
	colors[ImGuiCol_ChildBg] = bgDark;
	colors[ImGuiCol_PopupBg] = bg;

	// ==============================
	// Borders
	// ==============================
	colors[ImGuiCol_Border] = bgLight;
	colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);

	// ==============================
	// Frames
	// ==============================
	colors[ImGuiCol_FrameBg] = bgMid;
	colors[ImGuiCol_FrameBgHovered] = blenderOrangeMid;
	colors[ImGuiCol_FrameBgActive] = blenderOrange;

	// ==============================
	// Title bar
	// ==============================
	colors[ImGuiCol_TitleBg] = bgDark;
	colors[ImGuiCol_TitleBgActive] = bgMid;
	colors[ImGuiCol_TitleBgCollapsed] = bgDark;

	// ==============================
	// Menu / Scrollbar
	// ==============================
	colors[ImGuiCol_MenuBarBg] = bgDark;
	colors[ImGuiCol_ScrollbarBg] = bgDark;
	colors[ImGuiCol_ScrollbarGrab] = bgLight;
	colors[ImGuiCol_ScrollbarGrabHovered] = blenderOrangeMid;
	colors[ImGuiCol_ScrollbarGrabActive] = orangeActive;

	// ==============================
	// Check / Slider
	// ==============================
	colors[ImGuiCol_CheckMark] = blenderOrangeMid;
	colors[ImGuiCol_SliderGrab] = orangeActive;
	colors[ImGuiCol_SliderGrabActive] = blenderOrangeMid;

	// ==============================
	// Buttons
	// ==============================
	colors[ImGuiCol_Button] = blenderOrange;
	colors[ImGuiCol_ButtonHovered] = blenderOrangeMid;
	colors[ImGuiCol_ButtonActive] = ImVec4(orangeActive.x, orangeActive.y, orangeActive.z, 1.00f);

	// ==============================
	// Header
	// ==============================
	colors[ImGuiCol_Header] = blenderOrange;
	colors[ImGuiCol_HeaderHovered] = blenderOrangeMid;
	colors[ImGuiCol_HeaderActive] = ImVec4(orangeActive.x, orangeActive.y, orangeActive.z, 1.00f);

	// ==============================
	// Separator / Resize
	// ==============================
	colors[ImGuiCol_Separator] = bgLight;
	colors[ImGuiCol_SeparatorHovered] = blenderOrangeMid;
	colors[ImGuiCol_SeparatorActive] = orangeActive;

	colors[ImGuiCol_ResizeGrip] = blenderOrange;
	colors[ImGuiCol_ResizeGripHovered] = blenderOrangeMid;
	colors[ImGuiCol_ResizeGripActive] = ImVec4(orangeActive.x, orangeActive.y, orangeActive.z, 1.00f);

	// ==============================
	// Tabs
	// ==============================
	colors[ImGuiCol_Tab] = bgMid;
	colors[ImGuiCol_TabHovered] = blenderOrangeMid;
	colors[ImGuiCol_TabSelected] = blenderOrange;
	colors[ImGuiCol_TabSelectedOverline] = blenderOrangePreview;
	colors[ImGuiCol_TabDimmed] = bgDark;
	colors[ImGuiCol_TabDimmedSelected] = blenderOrange;
	colors[ImGuiCol_TabDimmedSelectedOverline] = blenderOrangePreview;

	// ==============================
	// Docking
	// ==============================
	colors[ImGuiCol_DockingPreview] = blenderOrangePreview;
	colors[ImGuiCol_DockingEmptyBg] = bgDark;

	// ==============================
	// Plots
	// ==============================
	colors[ImGuiCol_PlotLines] = blenderOrange;
	colors[ImGuiCol_PlotLinesHovered] = blenderOrangeMid;
	colors[ImGuiCol_PlotHistogram] = orangeActive;
	colors[ImGuiCol_PlotHistogramHovered] = blenderOrangeMid;

	// ==============================
	// Tables
	// ==============================
	colors[ImGuiCol_TableHeaderBg] = bgMid;
	colors[ImGuiCol_TableBorderStrong] = bgLight;
	colors[ImGuiCol_TableBorderLight] = bgDark;
	colors[ImGuiCol_TableRowBg] = ImVec4(0, 0, 0, 0);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.03f);

	// ==============================
	// Selection / Nav
	// ==============================
	colors[ImGuiCol_TextSelectedBg] = blenderOrangeMid;
	colors[ImGuiCol_DragDropTarget] = blenderOrangeMid;
	colors[ImGuiCol_NavCursor] = blenderOrangeMid;
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(blenderOrange.x, blenderOrange.y, blenderOrange.z, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.60f);

	// ==============================
	// Modal
	// ==============================
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.65f);

	// ==============================
	// Shape
	// ==============================
	style.WindowRounding = 4.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 4.0f;
	style.FrameBorderSize = 1.0f;
#endif
}