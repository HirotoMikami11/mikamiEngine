#pragma once
#include <memory>
#include <vector>
#include <deque>

#include "Engine.h"

//パーツ関連
#include "Parts/BaseParts.h"
#include "Parts/HeadParts.h"
#include "Parts/BodyParts.h"
#include "Parts/TailParts.h"
//State Phase関連
#include "State/BossState.h"
#include "State/BossStateManager.h"
#include "Phase/BossPhaseManager.h"
//spline移動
#include "BossSplineTrack.h"
#include "BossSplineMovement.h"
#include "BossSplineDebugger.h"
#include "BossMoveEditor.h"
//UI関連
#include "BossUI.h"
//弾関連
#include "BossBulletPool.h "
//エミッター関連
#include "BossSmokeEmitter.h"
#include "BossBreakSmokeEmitter.h"
#include "BossExplosionEmitter.h"

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
	void Draw();

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
	void SetHeadPosition(const Vector3& position);//直接位置を設定
	void SetHeadRotationY(float rotationY);

	// 移動速度の取得（Phase別倍率を適用）
	float GetMoveSpeed() const;

	// HP管理
	float GetHP() const { return bossHP_; }
	float GetMaxHP() const { return maxBossHP_; }
	void SetHP(float hp);
	void TakeDamageFromPart(float damage);  // パーツからのダメージを受ける

	// Phase管理
	BossPhase GetCurrentPhase() const;
	DeathSubPhase GetDeathSubPhase() const;
	BossPhaseManager* GetPhaseManager() const { return phaseManager_.get(); }

	// コライダーリストを取得（CollisionManagerに登録するため）
	// const参照で返す（コピー不要）
	const std::vector<Collider*>& GetColliders();

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

	/// <summary>
	/// 弾を発射
	/// </summary>
	/// <param name="position">発射位置</param>
	/// <param name="velocity">弾の速度ベクトル</param>
	/// <returns>発射成功時true、プールが満杯の場合false</returns>
	bool FireBullet(const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 全ての弾を更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void UpdateBullets(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 全ての弾を描画
	/// </summary>
	void DrawBullets();

	/// <summary>
	/// アクティブな弾のリストを取得（当たり判定用）
	/// </summary>
	std::vector<BossBullet*> GetActiveBullets() const;

	/// <summary>
	/// Activeな全パーツのリストを取得（State使用）
	/// const参照で返す
	/// </summary>
	const std::vector<BaseParts*>& GetActiveBodyParts() const;

	/// <summary>
	/// 全パーツのリストを取得
	/// const参照で返す
	/// </summary>
	const std::vector<BaseParts*>& GetAllBodyParts() const;

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
	/// 全パーツの更新を統合処理（位置・回転・行列）
	/// 元の UpdatePartsPositions(), UpdatePartsRotations(), 個別Update() を統合
	/// </summary>
	void UpdateAllParts(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 位置履歴から各パーツの位置を更新
	/// </summary>
	void UpdatePartsPositionsFromHistory();

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

	/// <summary>
	/// パーツキャッシュを再構築
	/// </summary>
	void RebuildPartsCache();

	/// <summary>
	/// アクティブパーツキャッシュを無効化
	/// </summary>
	void InvalidateActivePartsCache();

	// パーツ（ポインタは生ポインタで管理）
	std::unique_ptr<HeadParts> head_;
	std::vector<std::unique_ptr<BodyParts>> bodies_;
	std::unique_ptr<TailParts> tail_;

	// パーツキャッシュ
	std::vector<Collider*> collidersCache_;				// 全コライダーのキャッシュ（固定）
	std::vector<BaseParts*> allPartsCache_;				// 全パーツのキャッシュ（固定）
	mutable std::vector<BaseParts*> activePartsCache_;	// アクティブパーツのキャッシュ（Phase依存）
	mutable bool activePartsCacheDirty_ = true;			// アクティブパーツキャッシュの更新フラグ

	// 位置履歴（頭の移動履歴を保存）
	std::deque<Vector3> positionHistory_;

	// State管理
	std::unique_ptr<BossState> currentState_;
	std::unique_ptr<BossStateManager> stateManager_;

	// Phase管理
	std::unique_ptr<BossPhaseManager> phaseManager_;

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
	std::unique_ptr<BossBulletPool> bulletPool_;
	std::unique_ptr<BossSmokeEmitter> smokeEmitter_;
	std::unique_ptr<BossBreakSmokeEmitter> smokeBreakEmitter_;
	std::unique_ptr<BossExplosionEmitter> explosionEmitter_;

	// パラメータ
	const size_t kBodyCount = 6;				// 体のパーツ数
	float partsOffset_ = 0.0f;					// パーツ間のオフセット（隙間）（ImGuiで変更可能）
	float baseMoveSpeed_ = 10.0f;				// 基本移動速度
	const float kHistoryUpdateThreshold = 0.001f;	// 履歴更新の閾値（ガタガタ防止）
	const size_t kMaxHistorySize = 2048;		// 履歴の最大サイズ
	const float kBasePartSize = 1.0f;			// 基本パーツサイズ（モデルのデフォルトサイズ）
	const size_t kBulletPoolSize = 500;			// 弾プールサイズ

	// 前回の頭の位置（履歴更新判定用）
	Vector3 previousHeadPosition_;

	// デバッグ表示フラグ
	bool showColliders_ = true;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;

};