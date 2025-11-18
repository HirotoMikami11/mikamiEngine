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
#include "State/BossStateManager.h"
#include "BossSplineTrack.h"
#include "BossSplineMovement.h"
#include "BossSplineDebugger.h"
#include "BossMoveEditor.h"
#include "BossUI.h"
#include "BossBullet.h"


/// <summary>
/// Phase（フェーズ管理）
/// </summary>
enum class BossPhase {
	Phase1,		// フェーズ1: 頭と体がアクティブ
	Phase2,		// フェーズ2: 頭と尻尾がアクティブ
	Death,		// 死亡フェーズ: すべて非アクティブ
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
	void Update(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatirxSprite);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// オフスクリーン外に描画
	/// </summary>
	void DrawUI();

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
	void SetHeadPosition(const Vector3& position);  // 追加：直接位置を設定
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

	// スプライントラックを取得
	BossSplineTrack* GetSplineTrack() const { return splineTrack_.get(); }

	// スプライン移動システムを取得
	BossSplineMovement* GetSplineMovement() const { return splineMovement_.get(); }

	/// <summary>
	/// 位置履歴をクリア
	/// </summary>
	void ClearPositionHistory();

	/// <summary>
	/// 頭の向きに合わせて、全パーツを一直線に整列させる
	/// 頭の位置と向きを基準に、体と尻尾を後ろ方向に配置し、位置履歴も更新
	/// </summary>
	void AlignAllPartsInLine();


	// 弾発射機構
	/// <summary>
	/// 弾を発射
	/// </summary>
	/// <param name="position">発射位置</param>
	/// <param name="velocity">弾の速度ベクトル</param>
	void FireBullet(const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 全ての弾を更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void UpdateBullets(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 全ての弾を描画
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void DrawBullets(const Light& directionalLight);

	/// <summary>
	/// 弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<BossBullet>>& GetBullets() const { return bossBullets_; }

	/// <summary>
	/// Activeな全パーツのリストを取得（State から使用）
	/// </summary>
	std::vector<BaseParts*> GetActiveBodyParts();

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
	/// 2つのパーツ間の距離を計算（スケールを考慮）
	/// </summary>
	/// <param name="part1">前のパーツ</param>
	/// <param name="part2">次のパーツ</param>
	/// <returns>2つのパーツ間の距離</returns>
	float CalculateDistanceBetweenParts(BaseParts* part1, BaseParts* part2);

	/// <summary>
	/// Phase毎のパーツ状態を更新（衝突属性とアクティブ状態）
	/// </summary>
	void UpdatePartsState();

	// パーツ（ポインタは生ポインタで管理）
	std::unique_ptr<HeadParts> head_;
	std::vector<std::unique_ptr<BodyParts>> bodies_;
	std::unique_ptr<TailParts> tail_;

	// 位置履歴（頭の移動履歴を保存）
	std::deque<Vector3> positionHistory_;

	// State管理
	std::unique_ptr<BossState> currentState_;
	std::unique_ptr<BossStateManager> stateManager_;

	// Phase管理
	BossPhase currentPhase_ = BossPhase::Phase1;
	BossPhase previousPhase_ = BossPhase::Phase1;

	//UI
	std::unique_ptr<BossUI> bossUI_;

	// 移動用スプラインシステム
	std::unique_ptr<BossSplineTrack> splineTrack_;
	std::unique_ptr<BossSplineMovement> splineMovement_;
	std::unique_ptr<BossSplineDebugger> splineDebugger_;
	std::unique_ptr<BossMoveEditor> moveEditor_;

	// HP管理
	float maxBossHP_ = 500.0f;	// Boss全体の最大HP
	float bossHP_ = 500.0f;		// Boss全体の現在HP

	// 弾管理
	std::list<std::unique_ptr<BossBullet>> bossBullets_;

	// パラメータ
	const size_t kBodyCount = 5;				// 体のパーツ数
	float partsOffset_ = 0.0f;					// パーツ間のオフセット（隙間）（ImGuiで変更可能）
	float moveSpeed_ = 10.0f;					// 移動速度
	const float kHistoryUpdateThreshold = 0.001f;	// 履歴更新の閾値（ガタガタ防止）
	const size_t kMaxHistorySize = 2048;		// 履歴の最大サイズ
	const float kBasePartSize = 1.0f;			// 基本パーツサイズ（キューブのデフォルトサイズ）

	// 前回の頭の位置（履歴更新判定用）
	Vector3 previousHeadPosition_;

	// デバッグ表示フラグ
	bool showColliders_ = true;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;

};