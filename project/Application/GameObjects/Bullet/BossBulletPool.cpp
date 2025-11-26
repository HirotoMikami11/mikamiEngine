#include "BossBulletPool.h"

void BossBulletPool::Initialize(DirectXCommon* dxCommon, size_t poolSize) {
	dxCommon_ = dxCommon;
	activeBulletCount_ = 0;

	// プールサイズ分の弾を事前生成
	bulletPool_.reserve(poolSize);
	isActive_.reserve(poolSize);

	for (size_t i = 0; i < poolSize; ++i) {
		//球のリソースを確保する
		auto bullet = std::make_unique<BossBullet>();
		bullet->Initialize(dxCommon_, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f });

		// 最初は全て非アクティブ状態
		bulletPool_.push_back(std::move(bullet));
		isActive_.push_back(false);
	}
}

bool BossBulletPool::FireBullet(const Vector3& position, const Vector3& velocity) {
	// プールから非アクティブな弾を探す
	for (size_t i = 0; i < bulletPool_.size(); ++i) {
		if (!isActive_[i]) {
			//座標とアクティブだけ変更して生成
			bulletPool_[i]->Activate(position, velocity);
			isActive_[i] = true;
			activeBulletCount_++;
			return true;
		}
	}

	// プールが満杯の場合は発射できない
	return false;
}

void BossBulletPool::Update(const Matrix4x4& viewProjectionMatrix) {
	// アクティブな弾のみ更新
	for (size_t i = 0; i < bulletPool_.size(); ++i) {
		if (isActive_[i]) {
			bulletPool_[i]->Update(viewProjectionMatrix);

			// 弾が死亡したら非アクティブ化
			if (bulletPool_[i]->IsDead()) {
				isActive_[i] = false;
				activeBulletCount_--;
			}
		}
	}
}

void BossBulletPool::Draw() {
	// アクティブな弾のみ描画
	for (size_t i = 0; i < bulletPool_.size(); ++i) {
		if (isActive_[i]) {
			bulletPool_[i]->Draw();
		}
	}
}

std::vector<BossBullet*> BossBulletPool::GetActiveBullets() {
	std::vector<BossBullet*> activeBullets;
	activeBullets.reserve(activeBulletCount_);

	for (size_t i = 0; i < bulletPool_.size(); ++i) {
		if (isActive_[i]) {
			activeBullets.push_back(bulletPool_[i].get());
		}
	}

	return activeBullets;
}

void BossBulletPool::ResetAll() {
	// 全ての弾を非アクティブ化
	for (size_t i = 0; i < bulletPool_.size(); ++i) {
		if (isActive_[i]) {
			bulletPool_[i]->Deactivate();
			isActive_[i] = false;
		}
	}
	activeBulletCount_ = 0;
}