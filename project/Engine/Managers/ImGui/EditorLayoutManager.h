//#pragma once
//#include <d3d12.h>
//#include <string>
//#include <functional>
//#include "DirectXCommon.h"
//
///// <summary>
///// ImGuiのドッキングレイアウトを管理するクラス
///// エディタライクなUIを提供
///// </summary>
//class EditorLayoutManager {
//public:
//	EditorLayoutManager() = default;
//	~EditorLayoutManager() = default;
//
//	/// <summary>
//	/// 初期化
//	/// </summary>
//	/// <param name="dxCommon">DirectXCommonへのポインタ</param>
//	void Initialize(DirectXCommon* dxCommon);
//
//	/// <summary>
//	/// 終了処理
//	/// </summary>
//	void Finalize();
//
//	/// <summary>
//	/// フレーム開始（ドッキングスペース作成）
//	/// </summary>
//	void BeginFrame();
//
//	/// <summary>
//	/// フレーム終了
//	/// </summary>
//	void EndFrame();
//
//	/// <summary>
//	/// シーンビューポート開始
//	/// </summary>
//	/// <returns>シーンビューポートが表示されているか</returns>
//	bool BeginSceneViewport();
//
//	/// <summary>
//	/// シーンビューポート終了
//	/// </summary>
//	void EndSceneViewport();
//
//	/// <summary>
//	/// シーンビューポートにテクスチャを描画
//	/// </summary>
//	/// <param name="textureHandle">描画するテクスチャのSRVハンドル</param>
//	void DrawSceneTexture(D3D12_GPU_DESCRIPTOR_HANDLE textureHandle);
//
//	/// <summary>
//	/// ヒエラルキーウィンドウ開始
//	/// </summary>
//	bool BeginHierarchyWindow();
//
//	/// <summary>
//	/// ヒエラルキーウィンドウ終了
//	/// </summary>
//	void EndHierarchyWindow();
//
//	/// <summary>
//	/// インスペクターウィンドウ開始
//	/// </summary>
//	bool BeginInspectorWindow();
//
//	/// <summary>
//	/// インスペクターウィンドウ終了
//	/// </summary>
//	void EndInspectorWindow();
//
//	/// <summary>
//	/// コンソールウィンドウ開始
//	/// </summary>
//	bool BeginConsoleWindow();
//
//	/// <summary>
//	/// コンソールウィンドウ終了
//	/// </summary>
//	void EndConsoleWindow();
//
//	/// <summary>
//	/// プロジェクトウィンドウ開始
//	/// </summary>
//	bool BeginProjectWindow();
//
//	/// <summary>
//	/// プロジェクトウィンドウ終了
//	/// </summary>
//	void EndProjectWindow();
//
//	/// <summary>
//	/// エディタモードが有効か
//	/// </summary>
//	bool IsEditorMode() const { return isEditorMode_; }
//
//	/// <summary>
//	/// エディタモードの切り替え
//	/// </summary>
//	void SetEditorMode(bool enabled) { isEditorMode_ = enabled; }
//
//	/// <summary>
//	/// シーンビューポートのサイズを取得
//	/// </summary>
//	void GetSceneViewportSize(float& width, float& height) const;
//
//	/// <summary>
//	/// シーンビューポートにフォーカスがあるか
//	/// </summary>
//	bool IsSceneViewportFocused() const { return isSceneViewportFocused_; }
//
//	/// <summary>
//	/// シーンビューポートにマウスホバーがあるか
//	/// </summary>
//	bool IsSceneViewportHovered() const { return isSceneViewportHovered_; }
//
//private:
//	/// <summary>
//	/// デフォルトのドッキングレイアウトをセットアップ
//	/// </summary>
//	void SetupDefaultLayout();
//
//private:
//	// DirectXCommon参照
//	DirectXCommon* dxCommon_ = nullptr;
//
//	// エディタモードフラグ
//	bool isEditorMode_ = false;
//
//	// ドッキングスペースID
//	unsigned int dockspaceID_ = 0;
//
//	// 初回レイアウトセットアップフラグ
//	bool isFirstTime_ = true;
//
//	// シーンビューポートの状態
//	float sceneViewportWidth_ = 1280.0f;
//	float sceneViewportHeight_ = 720.0f;
//	bool isSceneViewportFocused_ = false;
//	bool isSceneViewportHovered_ = false;
//
//	// ウィンドウの表示フラグ
//	bool showHierarchy_ = true;
//	bool showInspector_ = true;
//	bool showConsole_ = true;
//	bool showProject_ = true;
//};