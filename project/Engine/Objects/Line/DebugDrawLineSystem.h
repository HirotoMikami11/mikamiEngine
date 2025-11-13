#pragma once
#include <memory>
#include <d3d12.h>
#include "DirectXCommon.h"
#include "LineRenderer.h"
#include "MyFunction.h"

/// <summary>
/// デバッグ描画管理クラス（シングルトン）
/// <para>全てのデバッグ線分描画を一元管理</para>
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
	/// フレーム開始時の処理（全ての線分をクリア）
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

	/// <summary>
	/// AABBを描画
	/// </summary>
	/// <param name="aabb">AABB（ワールド座標）</param>
	/// <param name="color">色</param>
	void DrawAABB(const AABB& aabb, const Vector4& color = { 1.0f, 0.0f, 0.0f, 1.0f });

	/// <summary>
	/// 球体を描画
	/// </summary>
	/// <param name="center">中心座標</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	/// <param name="subdivision">分割数（デフォルト10）</param>
	void DrawSphere(const Vector3& center, float radius, const Vector4& color = { 0.0f, 1.0f, 0.0f, 1.0f }, uint32_t subdivision = 5);

	/// <summary>
	/// グリッド線を描画（XZ平面）
	/// </summary>
	/// <param name="center">グリッドの中心</param>
	/// <param name="size">グリッドサイズ</param>
	/// <param name="interval">間隔</param>
	/// <param name="majorInterval">主要線の間隔</param>
	/// <param name="normalColor">通常線の色</param>
	/// <param name="majorColor">主要線の色</param>
	void DrawGrid(
		const Vector3& center,
		float size,
		float interval,
		float majorInterval,
		const Vector4& normalColor = { 0.5f, 0.5f, 0.5f, 1.0f },
		const Vector4& majorColor = { 0.0f, 0.0f, 0.0f, 1.0f }
	);

	/// <summary>
	/// 十字線を描画（位置マーカー）
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	void DrawCross(const Vector3& position, float size = 0.5f, const Vector4& color = { 1.0f, 1.0f, 0.0f, 1.0f });

	/// <summary>
	/// カプセルを描画
	/// </summary>
	/// <param name="start">カプセルの開始点</param>
	/// <param name="end">カプセルの終了点</param>
	/// <param name="radius">半径</param>
	/// <param name="color">色</param>
	/// <param name="segments">分割数</param>
	void DrawCapsule(const Vector3& start, const Vector3& end, float radius, const Vector4& color = { 0.0f, 1.0f, 1.0f, 1.0f }, int segments = 16);


	bool IsInitialized() const { return isInitialized_; }
	uint32_t GetLineCount() const;
	bool IsFull() const;

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

	// LineRendererインスタンス（シングルトンで管理）
	std::unique_ptr<LineRenderer> lineRenderer_;

	// DirectXCommon参照
	DirectXCommon* directXCommon_ = nullptr;

	// 状態フラグ
	bool isInitialized_ = false;
};