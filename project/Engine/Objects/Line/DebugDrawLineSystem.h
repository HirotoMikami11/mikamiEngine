#pragma once
#include <memory>
#include <d3d12.h>
#include "DirectXCommon.h"
#include "LineRenderer.h"
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
/// デバッグ描画管理クラス（シングルトン）
/// <para>全てのデバッグ線分描画を一元管理</para>
/// <para>グリッド機能を内包</para>
/// </summary>
class DebugDrawLineSystem
{
public:
	// シングルトンインスタンス取得
	static DebugDrawLineSystem* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// フレーム開始時の処理（全ての線分をクリア、グリッドを自動生成）
	/// </summary>
	void Reset();

	/// <summary>
	/// 一括描画（フレーム終了時に呼ぶ）
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Draw(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	// ===== 基本図形描画API =====

	/// <summary>
	/// 単一線分を追加
	/// </summary>
	void AddLine(const Vector3& start, const Vector3& end, const Vector4& color);
	void AddLine(const Vector3& start, const Vector3& end, const uint32_t& color);

	/// <summary>
	/// AABBを描画
	/// </summary>
	/// <param name="aabb">AABB（ワールド座標）</param>
	/// <param name="color">色</param>
	void DrawAABB(const AABB& aabb, const Vector4& color = { 0.0f, 1.0f, 0.0f, 1.0f });
	void DrawAABB(const AABB& aabb, const uint32_t& color = 0x00FF00FF);

	/// <summary>
	/// 球体を描画
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	/// <param name="subdivision">分割数（デフォルト16）</param>
	void DrawSphere(const Vector3& center, float radius, const Vector4& color = { 0.0f, 1.0f, 0.0f, 1.0f }, uint32_t subdivision = 16);
	void DrawSphere(const Vector3& center, float radius, const uint32_t& color = 0x00FF00FF, uint32_t subdivision = 16);

	/// <summary>
	/// 十字線を描画（位置マーカー）
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	void DrawCross(const Vector3& position, float size = 0.5f, const Vector4& color = { 1.0f, 1.0f, 0.0f, 1.0f });

	// ===== グリッド設定API =====

	/// <summary>
	/// グリッド設定を変更
	/// </summary>
	/// <param name="gridType">グリッドタイプ（XZ, XY, YZ）</param>
	/// <param name="size">グリッドサイズ（一辺の長さ）</param>
	/// <param name="interval">間隔</param>
	/// <param name="majorInterval">主要線の間隔</param>
	void ConfigureGrid(
		const GridLineType& gridType = GridLineType::XZ,
		float size = 100.0f,
		float interval = 1.0f,
		float majorInterval = 10.0f
	);

	/// <summary>
	/// グリッドの色を設定
	/// </summary>
	/// <param name="normalColor">通常線の色</param>
	/// <param name="majorColor">主要線の色</param>
	void SetGridColors(
		const Vector4& normalColor = { 0.5f, 0.5f, 0.5f, 1.0f },
		const Vector4& majorColor = { 0.0f, 0.0f, 0.0f, 1.0f }
	);

	/// <summary>
	/// グリッドの中心位置を設定
	/// </summary>
	void SetGridCenter(const Vector3& center) { gridCenter_ = center; }

	/// <summary>
	/// グリッドの表示/非表示を設定
	/// </summary>
	void SetGridVisible(bool visible) { isGridVisible_ = visible; }

	// ===== Getter =====
	bool IsInitialized() const { return isInitialized_; }
	uint32_t GetLineCount() const;
	bool IsFull() const;
	bool IsGridVisible() const { return isGridVisible_; }
	const Vector3& GetGridCenter() const { return gridCenter_; }

private:
	// シングルトンパターン
	DebugDrawLineSystem() = default;
	~DebugDrawLineSystem() = default;
	DebugDrawLineSystem(const DebugDrawLineSystem&) = delete;
	DebugDrawLineSystem& operator=(const DebugDrawLineSystem&) = delete;

	/// <summary>
	/// AABBの8頂点を計算
	/// </summary>
	void CalculateAABBVertices(const AABB& aabb, Vector3 vertices[8]) const;

	/// <summary>
	/// グリッド線を生成
	/// </summary>
	void GenerateGridLines();

	/// <summary>
	/// 各平面のグリッドを描画
	/// </summary>
	void DrawXZGrid();
	void DrawXYGrid();
	void DrawYZGrid();

	// LineRendererインスタンス(別々にしてないため、時間あるときに別途分ける)
	std::unique_ptr<LineRenderer> lineRenderer_;

	// DirectXCommon参照
	DirectXCommon* dxCommon_ = nullptr;

	// 状態フラグ
	bool isInitialized_ = false;
	bool isUse_ = false;  // 使用するか

	// グリッド設定
	bool isGridVisible_ = false;  // デフォルトで表示
	GridLineType gridType_ = GridLineType::XZ;
	Vector3 gridCenter_ = { 0.0f, 0.0f, 0.0f };
	float gridSize_ = 100.0f;
	float gridInterval_ = 1.0f;
	float gridMajorInterval_ = 10.0f;
	Vector4 gridNormalColor_ = { 0.5f, 0.5f, 0.5f, 1.0f };
	Vector4 gridMajorColor_ = { 0.0f, 0.0f, 0.0f, 1.0f };
};