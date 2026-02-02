#pragma once
#include <memory>
#include <vector>
#include "Engine.h"
#include "Enemy/Parts/BaseEnemyParts.h"
#include "Enemy/Parts/WormHeadParts.h"
#include "Enemy/Parts/WormBodyParts.h"
#include "Enemy/Parts/WormTailParts.h"

/// <summary>
/// うねうね動く雑魚敵（3パーツ構成：頭・体・尻尾）
/// </summary>
class EnemyWorm {
public:
	EnemyWorm();
	~EnemyWorm();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// ダメージを受ける
	/// </summary>
	/// <param name="damage">ダメージ量</param>
	void TakeDamage(float damage);

	/// <summary>
	/// コライダーリストを取得（CollisionManagerに登録するため）
	/// </summary>
	const std::vector<Collider*>& GetColliders();

	// HP取得
	float GetHP() const { return hp_; }
	float GetMaxHP() const { return maxHP_; }

	// 位置取得
	Vector3 GetPosition() const;

	// アクティブ状態
	bool IsActive() const { return isActive_; }
	void SetActive(bool active);

private:
	/// <summary>
	/// パーツの初期化
	/// </summary>
	void InitializeParts();

	/// <summary>
	/// パーツの位置と回転を更新
	/// </summary>
	void UpdatePartsTransform();

	/// <summary>
	/// うねうね動作の更新
	/// </summary>
	void UpdateWaveMotion();

	// パーツ
	std::unique_ptr<WormHeadParts> head_;
	std::unique_ptr<WormBodyParts> body_;
	std::unique_ptr<WormTailParts> tail_;

	// コライダーキャッシュ
	std::vector<Collider*> collidersCache_;

	// パーツキャッシュ（順序付き：頭→体→尻尾）
	std::vector<BaseEnemyParts*> allPartsCache_;

	// HP管理
	float maxHP_ = 100.0f;
	float hp_ = 100.0f;

	// 位置・動作パラメータ
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };		// 敵全体の位置（頭の位置）
	float waveTimer_ = 0.0f;						// うねうね動作用タイマー
	float waveSpeed_ = 2.0f;						// うねうね速度
	float waveAmplitude_ = 0.5f;					// うねうね振幅
	float partsDistance_ = 1.2f;					// パーツ間の距離

	// 状態
	bool isActive_ = true;

	// デバッグ表示フラグ
	bool showColliders_ = true;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};
