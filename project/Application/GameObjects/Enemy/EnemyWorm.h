#pragma once
#include <memory>
#include <vector>
#include "Engine.h"
#include "Enemy/Parts/BaseEnemyParts.h"
#include "Enemy/Parts/WormHeadParts.h"
#include "Enemy/Parts/WormBodyParts.h"

/// <summary>
/// うねうね動く雑魚敵（8パーツ構成：頭1 + 体7）
/// チンアナゴのように上方向に伸び、傘の持ち手のように頭がプレイヤー方向を向く
/// 一番下の体パーツを固定点とし、下部は直立、上部のみが動く
/// </summary>
class EnemyWorm {
public:
	EnemyWorm();
	~EnemyWorm();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置（一番下の体パーツの位置）</param>
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
	/// プレイヤーの位置を設定
	/// </summary>
	/// <param name="position">プレイヤーの位置</param>
	void SetPlayerPosition(const Vector3& position) { playerPosition_ = position; }

	/// <summary>
	/// コライダーリストを取得（CollisionManagerに登録するため）
	/// </summary>
	const std::vector<Collider*>& GetColliders();

	// HP取得
	float GetHP() const { return hp_; }
	float GetMaxHP() const { return maxHP_; }

	// 位置取得（一番下の体パーツの位置）
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
	/// 直立状態に更新（初期姿勢）
	/// </summary>
	void UpdateStraightPose();

	/// <summary>
	/// プレイヤー追尾状態に更新（傘の持ち手のように曲がる）
	/// </summary>
	void UpdateLookAtPlayerPose();

	/// <summary>
	/// 頭の目標回転角度を計算
	/// </summary>
	float CalculateHeadTargetRotation() const;

	// パーツ
	std::unique_ptr<WormHeadParts> head_;
	std::vector<std::unique_ptr<WormBodyParts>> bodies_;

	// コライダーキャッシュ
	std::vector<Collider*> collidersCache_;

	// パーツキャッシュ（順序付き：頭→体1→...→体7）
	std::vector<BaseEnemyParts*> allPartsCache_;

	// HP管理
	float maxHP_ = 100.0f;
	float hp_ = 100.0f;

	// 位置・動作パラメータ
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };		// 敵全体の位置（一番下の体パーツ）
	float partsDistance_ = 1.2f;					// パーツ間の距離

	// プレイヤー追尾関連
	bool lookAtPlayer_ = false;						// プレイヤー方向を向くかどうか
	Vector3 playerPosition_ = { 0.0f, 0.0f, 0.0f };	// プレイヤーの位置
	float curveSmoothness_ = 0.3f;					// カーブの滑らかさ（0.0 ~ 1.0）
	size_t fixedPartsCount_ = 4;					// 下から固定するパーツ数

	// 頭の回転補間
	float currentHeadRotationY_ = 3.14159f;		// 現在の頭の回転角度（初期値：180度）
	float headRotationSpeed_ = 0.1f;			// 回転速度（0.0 ~ 1.0）

	// 状態
	bool isActive_ = true;

	// デバッグ表示フラグ
	bool showColliders_ = true;

	// パーツ数
	const size_t kBodyCount = 7;					// 体のパーツ数

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};
