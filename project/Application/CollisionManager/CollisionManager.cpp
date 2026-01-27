#include "CollisionManager.h"

CollisionManager::CollisionManager() {}

CollisionManager::~CollisionManager() {}

void CollisionManager::Initialize() {
	// 初期化処理
	colliders_.clear();	//一応リストのクリア
}

void CollisionManager::Update() {
	// まず全コライダーの色をデフォルトにリセット
	ResetAllColliderColors();

	// 衝突判定を実行（衝突したものは赤に変更される）
	CheckAllCollision();
}

void CollisionManager::AddCollider(Collider* collider) {
	if (collider != nullptr) {
		colliders_.push_back(collider);
	}
}

void CollisionManager::ClearColliderList() {
	colliders_.clear();
}

void CollisionManager::ResetAllColliderColors() {
	// 登録されている全コライダーの色をデフォルトにリセット
	for (Collider* collider : colliders_) {
		if (collider) {
			collider->ResetColliderColor();
		}
	}
}

void CollisionManager::CheckAllCollision() {
	// リスト内のペアを総当たり
	std::list<Collider*>::iterator itrA = colliders_.begin();
	for (; itrA != colliders_.end(); ++itrA) {
		// イテレータAからコライダーAを取得する
		Collider* colliderA = *itrA;
		// イテレータBはイテレータAの次の要素から回して重複判定を回避
		std::list<Collider*>::iterator itrB = itrA;
		itrB++;

		for (; itrB != colliders_.end(); ++itrB) {
			// イテレータBからコライダーBを取得する
			Collider* colliderB = *itrB;
			// コライダーA,Bの衝突判定と応答
			CheckCollisionPair(colliderA, colliderB);
		}
	}
}

void CollisionManager::CheckCollisionPair(Collider* colliderA, Collider* colliderB) {
	// 衝突フィルタリング
	// コライダーAとコライダーBで衝突属性とマスクを確認
	if ((colliderA->GetCollisionAttribute() & colliderB->GetCollisionMask()) == 0 ||
		(colliderB->GetCollisionAttribute() & colliderA->GetCollisionMask()) == 0) {
		// 衝突しない場合は何もしない（色はResetAllColliderColors()でリセット済み）
		return;
	}

	// コライダーの種類に応じて衝突判定を行う
	bool isColliding = false;

	ColliderType typeA = colliderA->GetColliderType();
	ColliderType typeB = colliderB->GetColliderType();

	// 球体 vs 球体
	if (typeA == ColliderType::SPHERE && typeB == ColliderType::SPHERE) {
		Vector3 posA = colliderA->GetWorldPosition();
		Vector3 posB = colliderB->GetWorldPosition();
		float length = Length(posA - posB);
		
		if (length < colliderA->GetRadius() + colliderB->GetRadius()) {
			isColliding = true;
		}
	}
	// 球体 vs AABB
	else if (typeA == ColliderType::SPHERE && typeB == ColliderType::AABB) {
		SphereMath sphere;
		sphere.center = colliderA->GetWorldPosition();
		sphere.radius = colliderA->GetRadius();

		AABB aabb = colliderB->GetAABB();
		Vector3 posB = colliderB->GetWorldPosition();
		aabb.min = aabb.min + posB;
		aabb.max = aabb.max + posB;

		if (IsCollision(aabb, sphere)) {
			isColliding = true;
		}
	}
	// AABB vs 球体
	else if (typeA == ColliderType::AABB && typeB == ColliderType::SPHERE) {
		AABB aabb = colliderA->GetAABB();
		Vector3 posA = colliderA->GetWorldPosition();
		aabb.min = aabb.min + posA;
		aabb.max = aabb.max + posA;

		SphereMath sphere;
		sphere.center = colliderB->GetWorldPosition();
		sphere.radius = colliderB->GetRadius();

		if (IsCollision(aabb, sphere)) {
			isColliding = true;
		}
	}
	// AABB vs AABB
	else if (typeA == ColliderType::AABB && typeB == ColliderType::AABB) {
		AABB aabbA = colliderA->GetAABB();
		Vector3 posA = colliderA->GetWorldPosition();
		aabbA.min = aabbA.min + posA;
		aabbA.max = aabbA.max + posA;

		AABB aabbB = colliderB->GetAABB();
		Vector3 posB = colliderB->GetWorldPosition();
		aabbB.min = aabbB.min + posB;
		aabbB.max = aabbB.max + posB;

		if (IsCollision(aabbA, aabbB)) {
			isColliding = true;
		}
	}

	// 衝突していた場合の処理
	if (isColliding) {
		// コールバック関数を呼び出す（相手のコライダー情報を渡す）
		colliderA->OnCollision(colliderB);
		colliderB->OnCollision(colliderA);

		// コライダー同士の色を衝突色（赤）に変更
		colliderA->SetColliderColor(hitColor_);
		colliderB->SetColliderColor(hitColor_);
	}
}