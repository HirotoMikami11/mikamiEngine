#pragma once
#include "BossSplineTrack.h"
#include "DebugDrawLineSystem.h"

/// <summary>
/// Bossスプラインのデバッグ表示クラス
/// 制御点の十字線表示とスプライン曲線の描画を担当
/// </summary>
class BossSplineDebugger {
public:
	BossSplineDebugger() = default;
	~BossSplineDebugger() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="debugDrawSystem">DebugDrawLineSystemへのポインタ</param>
	/// <param name="track">BossSplineTrackへのポインタ</param>
	void Initialize(DebugDrawLineSystem* debugDrawSystem, BossSplineTrack* track);

	/// <summary>
	/// 描画（毎フレーム呼ぶ）
	/// DebugDrawLineSystemに線分を追加
	/// </summary>
	void Draw();

	/// <summary>
	/// スプライン曲線の線分を生成
	/// </summary>
	void GenerateSplineLines();

	/// <summary>
	/// 制御点の十字線を描画
	/// </summary>
	void DrawControlPoints();

	// === 表示制御 ===

	/// <summary>
	/// スプライン曲線の表示/非表示
	/// </summary>
	bool IsSplineVisible() const { return showSpline_; }
	void SetSplineVisible(bool visible) { showSpline_ = visible; }

	/// <summary>
	/// 制御点の表示/非表示
	/// </summary>
	bool IsControlPointsVisible() const { return showControlPoints_; }
	void SetControlPointsVisible(bool visible) { showControlPoints_ = visible; }

	// === スプライン曲線の設定 ===

	/// <summary>
	/// スプライン曲線の色を設定
	/// </summary>
	void SetSplineColor(const Vector4& color) { splineColor_ = color; }
	Vector4 GetSplineColor() const { return splineColor_; }

	/// <summary>
	/// スプライン曲線の分割数を設定
	/// </summary>
	void SetSplineSegments(int segments) { splineSegments_ = segments; }
	int GetSplineSegments() const { return splineSegments_; }

	// === 制御点の設定 ===

	/// <summary>
	/// 制御点の色を設定
	/// </summary>
	void SetControlPointColor(const Vector4& color) { controlPointColor_ = color; }
	Vector4 GetControlPointColor() const { return controlPointColor_; }

	/// <summary>
	/// 選択中の制御点の色を設定
	/// </summary>
	void SetSelectedPointColor(const Vector4& color) { selectedPointColor_ = color; }
	Vector4 GetSelectedPointColor() const { return selectedPointColor_; }

	/// <summary>
	/// 制御点の十字線のサイズを設定
	/// </summary>
	void SetControlPointSize(float size) { controlPointSize_ = size; }
	float GetControlPointSize() const { return controlPointSize_; }

	/// <summary>
	/// 選択中の制御点インデックスを設定
	/// </summary>
	void SetSelectedPointIndex(int index) { selectedPointIndex_ = index; }
	int GetSelectedPointIndex() const { return selectedPointIndex_; }

private:
	// システム参照
	DebugDrawLineSystem* debugDrawSystem_ = nullptr;
	BossSplineTrack* track_ = nullptr;

	// 表示フラグ
	bool showSpline_ = true;
	bool showControlPoints_ = true;

	// スプライン曲線の描画設定
	Vector4 splineColor_ = { 1.0f, 0.0f, 0.0f, 1.0f };	// 赤
	int splineSegments_ = 100;							// 分割数

	// 制御点の描画設定
	Vector4 controlPointColor_ = { 0.0f, 1.0f, 0.0f, 1.0f };		// 緑
	Vector4 selectedPointColor_ = { 1.0f, 1.0f, 0.0f, 1.0f };		// 黄色
	float controlPointSize_ = 0.5f;									// 十字線のサイズ
	int selectedPointIndex_ = -1;									// 選択中の制御点（-1は未選択）
};