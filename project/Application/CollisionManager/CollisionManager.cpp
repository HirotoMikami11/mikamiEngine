#include "CollisionManager.h"
#include "Collision.h"

CollisionManager::CollisionManager() {
	RegisterCollisionHandlers();
}

CollisionManager::~CollisionManager() {}

void CollisionManager::Initialize() {
	colliders_.clear();
	currentPairs_.clear();
	prevPairs_.clear();
}

void CollisionManager::Update() {
	// 1. 前フレームのペアを保存
	prevPairs_ = currentPairs_;
	currentPairs_.clear();

	// 2. 色をリセット
	ResetAllColliderColors();

	// 3. 今フレームの衝突ペアを全検査
	CheckAllCollision();

	// 4. Enter / Stay / Exit を振り分けてコールバック
	// --- Enter: 今フレームにある && 前フレームにない ---
	for (const auto& pair : currentPairs_) {
		ICollider* a = pair.first;
		ICollider* b = pair.second;

		a->SetColliderColor(hitColor_);
		b->SetColliderColor(hitColor_);

		if (prevPairs_.count(pair) == 0) {
			a->OnCollisionEnter(b);
			b->OnCollisionEnter(a);
		} else {
			a->OnCollisionStay(b);
			b->OnCollisionStay(a);
		}
	}

	// --- Exit: 前フレームにある && 今フレームにない ---
	for (const auto& pair : prevPairs_) {
		if (currentPairs_.count(pair) == 0) {
			pair.first->OnCollisionExit(pair.second);
			pair.second->OnCollisionExit(pair.first);
		}
	}
}

void CollisionManager::AddCollider(ICollider* collider) {
	if (collider != nullptr) {
		colliders_.push_back(collider);
	}
}

void CollisionManager::ClearColliderList() {
	colliders_.clear();
}

void CollisionManager::ResetAllColliderColors() {
	for (ICollider* collider : colliders_) {
		if (collider) {
			collider->ResetColliderColor();
		}
	}
}

void CollisionManager::CheckAllCollision() {
	auto itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {
		auto itrB = itrA;
		++itrB;
		for (; itrB != colliders_.end(); ++itrB) {
			if (CheckCollisionPair(*itrA, *itrB)) {
				currentPairs_.insert(MakePair(*itrA, *itrB));
			}
		}
	}
}

bool CollisionManager::CheckCollisionPair(ICollider* colliderA, ICollider* colliderB) {
	// 1. マスクフィルタ（最安、最優先）
	if ((colliderA->GetCollisionAttribute() & colliderB->GetCollisionMask()) == 0 ||
	    (colliderB->GetCollisionAttribute() & colliderA->GetCollisionMask()) == 0) {
		return false;
	}

	// 2. 型正規化（常に小さいインデックス側を a にする）
	ICollider* a = colliderA;
	ICollider* b = colliderB;
	if (static_cast<int>(a->GetColliderType()) > static_cast<int>(b->GetColliderType())) {
		std::swap(a, b);
	}

	int typeA = static_cast<int>(a->GetColliderType());
	int typeB = static_cast<int>(b->GetColliderType());

	// 3. テーブル参照（O(1)）
	CollisionFunc func = dispatchTable_[typeA][typeB];
	if (!func) return false; // 未実装ペアはスキップ

	// 4. 数学計算
	return func(a, b);
}

void CollisionManager::RegisterCollisionHandlers() {
	// --- Sphere vs Sphere ---
	dispatchTable_[static_cast<int>(ColliderType::SPHERE)][static_cast<int>(ColliderType::SPHERE)] =
	    [](ICollider* a, ICollider* b) -> bool {
		    auto* sa = static_cast<SphereCollider*>(a);
		    auto* sb = static_cast<SphereCollider*>(b);
		    return Collision::IsCollision(sa->GetWorldSphere(), sb->GetWorldSphere());
	    };

	// --- Sphere vs AABB ---
	dispatchTable_[static_cast<int>(ColliderType::SPHERE)][static_cast<int>(ColliderType::AABB)] =
	    [](ICollider* a, ICollider* b) -> bool {
		    auto* sphere = static_cast<SphereCollider*>(a);
		    auto* aabb   = static_cast<AABBCollider*>(b);
		    return Collision::IsCollision(sphere->GetWorldSphere(), aabb->GetWorldAABB());
	    };

	// --- AABB vs AABB ---
	dispatchTable_[static_cast<int>(ColliderType::AABB)][static_cast<int>(ColliderType::AABB)] =
	    [](ICollider* a, ICollider* b) -> bool {
		    auto* aabbA = static_cast<AABBCollider*>(a);
		    auto* aabbB = static_cast<AABBCollider*>(b);
		    return Collision::IsCollision(aabbA->GetWorldAABB(), aabbB->GetWorldAABB());
	    };

	// OBB, Sprite は実装時に RegisterCollisionHandlers() へ追加する
}
