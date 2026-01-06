#pragma once
#include <memory>
#include <array>
#include "TitleFieldSegment.h"
#include "Camera.h"

/// <summary>
/// タイトルシーンのフィールド管理クラス
/// 3つのTitleFieldSegmentを使って無限ループするフィールドを実現
/// </summary>
class TitleField {
public:
	TitleField();
	~TitleField();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// カメラの位置に応じてセグメントを再配置
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="cameraPosition">カメラのZ座標</param>
	void Update(const Matrix4x4& viewProjectionMatrix, float cameraZ);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

private:
	/// <summary>
	/// セグメントの位置を更新
	/// </summary>
	void UpdateSegmentPositions();

	/// <summary>
	/// カメラが次のセグメントに到達したかチェック
	/// </summary>
	/// <param name="cameraZ">カメラのZ座標</param>
	void CheckSegmentTransition(float cameraZ);

	// 3つのセグメント
	std::array<std::unique_ptr<TitleFieldSegment>, 3> segments_;

	// 各セグメントのZ座標
	std::array<float, 3> segmentPositions_;

	// セグメントの長さ（Z方向）
	const float segmentLength_ = 60.0f;

	// 現在のセグメントインデックス（次に移動するセグメント）
	int nextSegmentIndex_ = 0;

	// 前回のカメラZ座標（遷移判定用）
	float previousCameraZ_ = 0.0f;

	// 次の遷移ポイント（カメラがこのZ座標を超えたらセグメント移動）
	float nextTransitionZ_ = 60.0f;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};