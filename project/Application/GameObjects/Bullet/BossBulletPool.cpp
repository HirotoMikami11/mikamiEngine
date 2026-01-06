#include "BossBulletPool.h"
#include "BossBulletHitEffectPool.h"

void BossBulletPool::Initialize(DirectXCommon* dxCommon, size_t poolSize) {
	dxCommon_ = dxCommon;

	// メモリ事前確保
	bulletPool_.reserve(poolSize);
	freeIndices_.reserve(poolSize);
	activeIndices_.reserve(poolSize);
	isActive_.reserve(poolSize);

	// プールサイズ分の弾を事前生成
	for (size_t i = 0; i < poolSize; ++i) {
		// Transformバッファ（GPUリソース）を事前作成
		auto bullet = std::make_unique<BossBullet>();
		bullet->Initialize(dxCommon_, Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ 0.0f, 0.0f, 0.0f });

		bulletPool_.push_back(std::move(bullet));
		isActive_.push_back(false);

		// 最初は全て空きスロット
		freeIndices_.push_back(i);
	}
}

bool BossBulletPool::FireBullet(const Vector3& position, const Vector3& velocity) {

	if (freeIndices_.empty()) {
		return false;
	}
	// 空きインデックスを取得 
	size_t index = freeIndices_.back();
	freeIndices_.pop_back();

	// GPUリソース再利用
	bulletPool_[index]->Activate(position, velocity);
	isActive_[index] = true;

	// リストに追加
	activeIndices_.push_back(index);

	return true;
}

void BossBulletPool::Update(const Matrix4x4& viewProjectionMatrix) {
	// アクティブな弾だけループ
	for (size_t i = 0; i < activeIndices_.size(); ) {
		size_t index = activeIndices_[i];

		bulletPool_[index]->Update(viewProjectionMatrix);

		// 死亡チェック
		if (bulletPool_[index]->IsDead()) {
			// リストに戻す
			freeIndices_.push_back(index);
			isActive_[index] = false;

			// Swap-and-Pop で削除 O(1)
			// 末尾と交換してpop_backすることで、eraseの再配置コストを回避
			activeIndices_[i] = activeIndices_.back();
			activeIndices_.pop_back();
			// iを進めない（新しく来た要素を次のループで処理）
		} else {
			++i;
		}
	}
}

void BossBulletPool::Draw() {
	//　アクティブな弾だけ描画
	for (size_t index : activeIndices_) {
		bulletPool_[index]->Draw();
	}
}

std::vector<BossBullet*> BossBulletPool::GetActiveBullets() {
	std::vector<BossBullet*> activeBullets;
	activeBullets.reserve(activeIndices_.size());

	// アクティブインデックスから直接取得
	for (size_t index : activeIndices_) {
		activeBullets.push_back(bulletPool_[index].get());
	}

	return activeBullets;
}

void BossBulletPool::ResetAll() {
	// 全弾を非アクティブ化
	for (size_t index : activeIndices_) {
		bulletPool_[index]->Deactivate();
		isActive_[index] = false;
	}

	//インデックス管理をリセット
	activeIndices_.clear();
	freeIndices_.clear();

	// 全スロットを空きに戻す
	for (size_t i = 0; i < bulletPool_.size(); ++i) {
		freeIndices_.push_back(i);
	}
}

void BossBulletPool::SetHitEffectPool(BossBulletHitEffectPool* effectPool) {
	// 全ての弾丸に対してエフェクトプールを設定
	for (auto& bullet : bulletPool_) {
		if (bullet) {
			bullet->SetHitEffectPool(effectPool);
		}
	}
}