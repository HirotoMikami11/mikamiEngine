#pragma once
#include <memory>
#include <string>
#include "DirectXCommon.h"
#include "DebugDrawLineSystem.h"
#include "MyFunction.h"

/// <summary>
/// グリッド平面の種類
/// </summary>
enum class GridLineType {
	XZ,  // XZ平面上のグリッド
	XY,  // XY平面上のグリッド
	YZ   // YZ平面上のグリッド
};

/// <summary>
/// グリッド描画専用クラス
/// <para>DebugDrawLineSystemを使用してグリッド線を描画</para>
/// <para>Transformは持たず、固定位置にグリッドを表示</para>
/// </summary>
class GridLine
{
public:
	GridLine() = default;
	~GridLine() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="gridType">グリッドタイプ（XZ, XY, YZ）</param>
	/// <param name="size">グリッドサイズ（一辺の長さ）</param>
	/// <param name="interval">間隔</param>
	/// <param name="majorInterval">主要線の間隔</param>
	/// <param name="normalColor">通常線の色</param>
	/// <param name="majorColor">主要線の色</param>
	void Initialize(
		DirectXCommon* dxCommon,
		const GridLineType& gridType = GridLineType::XZ,
		float size = 100.0f,
		float interval = 1.0f,
		float majorInterval = 10.0f,
		const Vector4& normalColor = { 0.5f, 0.5f, 0.5f, 1.0f },
		const Vector4& majorColor = { 0.0f, 0.0f, 0.0f, 1.0f }
	);

	/// <summary>
	/// グリッド設定を変更して再生成
	/// </summary>
	void SetGridSettings(
		const GridLineType& gridType,
		float size,
		float interval,
		float majorInterval,
		const Vector4& normalColor,
		const Vector4& majorColor
	);

	/// <summary>
	/// グリッドの中心位置を設定
	/// </summary>
	void SetCenter(const Vector3& center) { center_ = center; }
	const Vector3& GetCenter() const { return center_; }

	/// <summary>
	/// 描画処理（DebugDrawLineSystemに線を追加）
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui用デバッグ表示
	/// </summary>
	void ImGui();

	// ===== Getter =====
	bool IsVisible() const { return isVisible_; }
	const std::string& GetName() const { return name_; }
	GridLineType GetGridType() const { return gridType_; }
	float GetSize() const { return gridSize_; }
	float GetInterval() const { return gridInterval_; }
	float GetMajorInterval() const { return gridMajorInterval_; }

	// ===== Setter =====
	void SetVisible(bool visible) { isVisible_ = visible; }
	void SetName(const std::string& name) { name_ = name; }

private:
	/// <summary>
	/// 各平面のグリッドを描画
	/// </summary>
	void DrawXZGrid();
	void DrawXYGrid();
	void DrawYZGrid();

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	DebugDrawLineSystem* debugDrawLineSystem_ = nullptr;

	// 基本情報
	bool isVisible_ = true;
	std::string name_ = "GridLine";

	// グリッド設定
	Vector3 center_ = { 0.0f, 0.0f, 0.0f };     // グリッドの中心位置
	GridLineType gridType_ = GridLineType::XZ;
	float gridSize_ = 100.0f;
	float gridInterval_ = 1.0f;
	float gridMajorInterval_ = 10.0f;
	Vector4 gridNormalColor_ = { 0.5f, 0.5f, 0.5f, 1.0f };
	Vector4 gridMajorColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};