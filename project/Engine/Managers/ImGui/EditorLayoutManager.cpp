//#include "EditorLayoutManager.h"
//#include "ImGui/ImGuiManager.h"
//#include "Logger.h"
//#include <cassert>
//
//void EditorLayoutManager::Initialize(DirectXCommon* dxCommon) {
//	assert(dxCommon);
//	dxCommon_ = dxCommon;
//
//#ifdef USEIMGUI
//	// エディタモードはRELEASEビルドでは無効化
//	isEditorMode_ = true;
//#endif
//
//	isEditorMode_ = false;
//}
//
//void EditorLayoutManager::Finalize() {
//	dxCommon_ = nullptr;
//}
//
//void EditorLayoutManager::BeginFrame() {
//#ifdef USEIMGUI
//	if (!isEditorMode_) {
//		return;
//	}
//
//	// フルスクリーンのドッキングスペースを作成
//	ImGuiViewport* viewport = ImGui::GetMainViewport();
//	ImGui::SetNextWindowPos(viewport->Pos);
//	ImGui::SetNextWindowSize(viewport->Size);
//	ImGui::SetNextWindowViewport(viewport->ID);
//
//	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
//	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
//	window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
//	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
//
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
//
//	ImGui::Begin("DockSpace", nullptr, window_flags);
//	ImGui::PopStyleVar(3);
//
//	// ドッキングスペースを作成
//	ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
//	dockspaceID_ = dockspace_id;
//	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
//
//	// 初回のみデフォルトレイアウトをセットアップ
//	if (isFirstTime_) {
//		SetupDefaultLayout();
//		isFirstTime_ = false;
//	}
//
//	// メニューバー
//	if (ImGui::BeginMenuBar()) {
//		if (ImGui::BeginMenu("Window")) {
//			ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy_);
//			ImGui::MenuItem("Inspector", nullptr, &showInspector_);
//			ImGui::MenuItem("Console", nullptr, &showConsole_);
//			ImGui::MenuItem("Project", nullptr, &showProject_);
//			ImGui::EndMenu();
//		}
//		ImGui::EndMenuBar();
//	}
//
//	ImGui::End();
//#endif
//}
//
//void EditorLayoutManager::EndFrame() {
//	// 特に処理なし
//}
//
//bool EditorLayoutManager::BeginSceneViewport() {
//#ifdef USEIMGUI
//	if (!isEditorMode_) {
//		return false;
//	}
//
//	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
//	bool open = ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
//
//	if (open) {
//		// ビューポートのサイズを取得
//		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
//		sceneViewportWidth_ = viewportPanelSize.x;
//		sceneViewportHeight_ = viewportPanelSize.y;
//
//		// フォーカスとホバー状態を更新
//		isSceneViewportFocused_ = ImGui::IsWindowFocused();
//		isSceneViewportHovered_ = ImGui::IsWindowHovered();
//	}
//
//	ImGui::PopStyleVar();
//	return open;
//#else
//	return false;
//#endif
//}
//
//void EditorLayoutManager::EndSceneViewport() {
//#ifdef USEIMGUI
//	if (isEditorMode_) {
//		ImGui::End();
//	}
//#endif
//}
//
//void EditorLayoutManager::DrawSceneTexture(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle) {
//#ifdef USEIMGUI
//	if (!isEditorMode_) {
//		return;
//	}
//
//	// ImGuiのImageウィジェットでテクスチャを描画
//	ImVec2 viewportSize(sceneViewportWidth_, sceneViewportHeight_);
//	ImGui::Image(
//		(ImTextureID)textureHandle.ptr,
//		viewportSize,
//		ImVec2(0, 0),  // UV開始
//		ImVec2(1, 1)   // UV終了
//	);
//#endif
//}
//
//bool EditorLayoutManager::BeginHierarchyWindow() {
//#ifdef USEIMGUI
//	if (!isEditorMode_ || !showHierarchy_) {
//		return false;
//	}
//	return ImGui::Begin("Hierarchy", &showHierarchy_);
//#else
//	return false;
//#endif
//}
//
//void EditorLayoutManager::EndHierarchyWindow() {
//#ifdef USEIMGUI
//	if (isEditorMode_) {
//		ImGui::End();
//	}
//#endif
//}
//
//bool EditorLayoutManager::BeginInspectorWindow() {
//#ifdef USEIMGUI
//	if (!isEditorMode_ || !showInspector_) {
//		return false;
//	}
//	return ImGui::Begin("Inspector", &showInspector_);
//#else
//	return false;
//#endif
//}
//
//void EditorLayoutManager::EndInspectorWindow() {
//#ifdef USEIMGUI
//	if (isEditorMode_) {
//		ImGui::End();
//	}
//#endif
//}
//
//bool EditorLayoutManager::BeginConsoleWindow() {
//#ifdef USEIMGUI
//	if (!isEditorMode_ || !showConsole_) {
//		return false;
//	}
//	return ImGui::Begin("Console", &showConsole_);
//#else
//	return false;
//#endif
//}
//
//void EditorLayoutManager::EndConsoleWindow() {
//#ifdef USEIMGUI
//	if (isEditorMode_) {
//		ImGui::End();
//	}
//#endif
//}
//
//bool EditorLayoutManager::BeginProjectWindow() {
//#ifdef USEIMGUI
//	if (!isEditorMode_ || !showProject_) {
//		return false;
//	}
//	return ImGui::Begin("Project", &showProject_);
//#else
//	return false;
//#endif
//}
//
//void EditorLayoutManager::EndProjectWindow() {
//#ifdef USEIMGUI
//	if (isEditorMode_) {
//		ImGui::End();
//	}
//#endif
//}
//
//void EditorLayoutManager::GetSceneViewportSize(float& width, float& height) const {
//	width = sceneViewportWidth_;
//	height = sceneViewportHeight_;
//}
//
//void EditorLayoutManager::SetupDefaultLayout() {
//#ifdef USEIMGUI
//	ImGui::DockBuilderRemoveNode(dockspaceID_);
//	ImGui::DockBuilderAddNode(dockspaceID_, ImGuiDockNodeFlags_DockSpace);
//	ImGui::DockBuilderSetNodeSize(dockspaceID_, ImGui::GetMainViewport()->Size);
//
//	// ドッキングエリアを分割
//	ImGuiID dock_main_id = dockspaceID_;
//	ImGuiID dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.2f, nullptr, &dock_main_id);
//	ImGuiID dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.25f, nullptr, &dock_main_id);
//	ImGuiID dock_id_bottom = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Down, 0.25f, nullptr, &dock_main_id);
//	
//	// 各ウィンドウをドッキング
//	ImGui::DockBuilderDockWindow("Hierarchy", dock_id_left);
//	ImGui::DockBuilderDockWindow("Inspector", dock_id_right);
//	ImGui::DockBuilderDockWindow("Console", dock_id_bottom);
//	ImGui::DockBuilderDockWindow("Project", dock_id_bottom);
//	ImGui::DockBuilderDockWindow("Scene", dock_main_id);
//
//	ImGui::DockBuilderFinish(dockspaceID_);
//
//	Logger::Log(Logger::GetStream(), "EditorLayoutManager: Default layout setup completed\n");
//#endif
//}