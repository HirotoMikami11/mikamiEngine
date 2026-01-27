#include "Collider.h"
#include "DebugDrawLineSystem.h"

void Collider::DebugLineAdd() {
	// デバッグ表示が無効なら何もしない
	if (!isColliderVisible_) {
		return;
	}

	// DebugDrawLineSystemのインスタンスを取得
	DebugDrawLineSystem* debugDrawLineSystem = Engine::GetInstance()->GetDebugDrawManager();
	if (!debugDrawLineSystem) {
		return;
	}

	// ワールド座標を取得
	Vector3 center = GetWorldPosition();

	// コライダーの種類によって描画を切り替え
	if (colliderType_ == ColliderType::SPHERE) {
		// 球体の場合：球体を描画
		debugDrawLineSystem->DrawSphere(center, radius_, currentColliderColor_, 3);
	}
	else if (colliderType_ == ColliderType::AABB) {
		// AABBの場合：AABBを描画
		// ワールド座標に合わせてAABBを更新
		AABB worldAABB = aabb_;
		Vector3 offset = center;
		worldAABB.min = aabb_.min + offset;
		worldAABB.max = aabb_.max + offset;
		
		debugDrawLineSystem->DrawAABB(worldAABB, currentColliderColor_);
	}
}