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
	/// <param name="subdivision">分割数（デフォルト10）</param>
	void DrawSphere(const Vector3& center, float radius, const Vector4& color = { 0.0f, 1.0f, 0.0f, 1.0f }, uint32_t subdivision = 3);
	void DrawSphere(const Vector3& center, float radius, const uint32_t& color = 0x00FF00FF, uint32_t subdivision = 3);

	/// <summary>
	/// 十字線を描画（位置マーカー）
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="size">サイズ</param>
	/// <param name="color">色</param>
	void DrawCross(const Vector3& position, float size = 0.5f, const Vector4& color = { 1.0f, 1.0f, 0.0f, 1.0f });

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
	bool isUse_;//使用するか
};