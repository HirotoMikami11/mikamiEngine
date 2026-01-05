#pragma once
#include "BaseBullet.h"
#include "CollisionConfig.h"

// 前方宣言
class BossBulletHitEffectPool;

/// <summary>
/// ボスの弾クラス
/// Activate/Deactivateパターンでリソースを再利用
/// </summary>
class BossBullet : public BaseBullet {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	BossBullet();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~BossBullet();

	/// <summary>
	/// 初期化（プール生成時に1回だけ呼ぶ）
	/// リソースを確保する
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="velocity">速度ベクトル</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& velocity) override;

	/// <summary>
	/// アクティブ化（発射時に呼ぶ）
	/// 位置と速度のみ更新し、リソースは再利用
	/// </summary>
	/// <param name="position">発射位置</param>
	/// <param name="velocity">速度ベクトル</param>
	void Activate(const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 非アクティブ化（死亡時に呼ぶ）
	/// </summary>
	void Deactivate();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix) override;

	/// <summary>
	/// 描画
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision(Collider* other) override;

	/// <summary>
	/// ヒットエフェクトプールを設定
	/// </summary>
	/// <param name="effectPool">エフェクトプールのポインタ</param>
	void SetHitEffectPool(BossBulletHitEffectPool* effectPool) { hitEffectPool_ = effectPool; }

private:
	// 寿命
	static const int32_t kLifeTime = 60 * 8;	// 弾の寿命（8秒）

	// ヒットエフェクトプールへの参照（所有権なし）
	BossBulletHitEffectPool* hitEffectPool_ = nullptr;
};