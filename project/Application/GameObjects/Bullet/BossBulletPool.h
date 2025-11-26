#pragma once
#include <vector>
#include <memory>
#include "BossBullet.h"
#include "Engine.h"

/// <summary>
/// ボス弾のオブジェクトプールクラス
/// 事前に弾を生成しておき、使い回すことでパフォーマンスを向上
/// </summary>
class BossBulletPool {
public:
	BossBulletPool() = default;
	~BossBulletPool() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="poolSize">プールサイズ（事前生成する弾の数）</param>
	void Initialize(DirectXCommon* dxCommon, size_t poolSize = 500);

	/// <summary>
	/// プールから非アクティブな弾を取得して発射
	/// </summary>
	/// <param name="position">発射位置</param>
	/// <param name="velocity">速度ベクトル</param>
	/// <returns>発射成功時true、プールが満杯の場合false</returns>
	bool FireBullet(const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 全ての弾を更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 全ての弾を描画
	/// </summary>
	void Draw();

	/// <summary>
	/// アクティブな弾のリストを取得（当たり判定用）
	/// </summary>
	/// <returns>アクティブな弾のポインタのベクター</returns>
	std::vector<BossBullet*> GetActiveBullets();

	/// <summary>
	/// 全ての弾を非アクティブ化（リセット）
	/// </summary>
	void ResetAll();

	/// <summary>
	/// 現在のアクティブな弾の数を取得
	/// </summary>
	size_t GetActiveBulletCount() const { return activeBulletCount_; }

	/// <summary>
	/// プールサイズを取得
	/// </summary>
	size_t GetPoolSize() const { return bulletPool_.size(); }

private:
	// 弾のプール（全てuniquePtrで管理）
	std::vector<std::unique_ptr<BossBullet>> bulletPool_;

	// アクティブ状態の管理（インデックスで対応）
	std::vector<bool> isActive_;

	// アクティブな弾の数
	size_t activeBulletCount_ = 0;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};