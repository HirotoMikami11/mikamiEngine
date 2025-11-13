#pragma once
#include <memory>
#include <vector>
#include <deque>
#include "Engine.h"
#include "Parts/BaseParts.h"
#include "Parts/HeadParts.h"
#include "Parts/BodyParts.h"
#include "Parts/TailParts.h"
#include "State/BossState.h"

/// <summary>
/// Phase（フェーズ管理）
/// </summary>
enum class BossPhase {
	Phase1,		// フェーズ1: 体パーツがEnemy、尻尾がObjects
	Phase2,		// フェーズ2: 体パーツがObjects、尻尾がEnemy
	Death,		// 死亡フェーズ
};

/// <summary>
/// 蛇のような動きをするボスクラス
/// </summary>
class Boss {
public:
	Boss();
	~Boss();

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
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	// State操作
	void ChangeState(std::unique_ptr<BossState> newState);
	BossState* GetCurrentState() const { return currentState_.get(); }

	// 頭の位置操作（State から呼ばれる）
	Vector3 GetHeadPosition() const;
	void MoveHead(const Vector3& movement);
	void SetHeadRotationY(float rotationY);

	// 移動速度の取得
	float GetMoveSpeed() const { return moveSpeed_; }

	// HP管理
	float GetHP() const { return bossHP_; }
	float GetMaxHP() const { return maxBossHP_; }
	void SetHP(float hp);
	void TakeDamageFromPart(float damage);  // パーツからのダメージを受ける

	// Phase管理
	BossPhase GetCurrentPhase() const { return currentPhase_; }
	void TransitionToPhase2();
	void TransitionToDeathPhase();

	// コライダーリストを取得（CollisionManagerに登録するため）
	std::vector<Collider*> GetColliders();

private:
	/// <summary>
	/// パーツの初期化
	/// </summary>
	void InitializeParts();

	/// <summary>
	/// 位置履歴の更新
	/// </summary>
	void UpdatePositionHistory();

	/// <summary>
	/// 各パーツの位置を更新（履歴から参照）
	/// </summary>
	void UpdatePartsPositions();

	/// <summary>
	/// 各パーツの向きを更新
	/// </summary>
	void UpdatePartsRotations();

	/// <summary>
	/// Phase遷移チェック
	/// </summary>
	void CheckPhaseTransition();

	/// <summary>
	/// パーツのHPを設定
	/// </summary>
	void SetPartsHP();

	/// <summary>
	/// Phase毎の衝突属性を設定
	/// </summary>
	void UpdateCollisionAttributes();

	// パーツ（ポインタは生ポインタで管理）
	std::unique_ptr<HeadParts> head_;
	std::vector<std::unique_ptr<BodyParts>> bodies_;
	std::unique_ptr<TailParts> tail_;

	// 位置履歴（頭の移動履歴を保存）
	std::deque<Vector3> positionHistory_;

	// State管理
	std::unique_ptr<BossState> currentState_;

	// Phase管理
	BossPhase currentPhase_ = BossPhase::Phase1;
	BossPhase previousPhase_ = BossPhase::Phase1;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	Matrix4x4 viewProjectionMatrix_;

	// HP管理
	float maxBossHP_ = 500.0f;	// Boss全体の最大HP
	float bossHP_ = 500.0f;		// Boss全体の現在HP

	// パラメータ
	const size_t kBodyCount = 5;				// 体のパーツ数
	float partsDistance_ = 1.5f;				// パーツ間の距離（ImGuiで変更可能）
	float moveSpeed_ = 1.0f;					// 移動速度
	const float kHistoryUpdateThreshold = 0.001f;	// 履歴更新の閾値（ガタガタ防止）
	const size_t kMaxHistorySize = 1000;		// 履歴の最大サイズ

	// 前回の頭の位置（履歴更新判定用）
	Vector3 previousHeadPosition_;

	// デバッグ表示フラグ
	bool showColliders_ = true;
};