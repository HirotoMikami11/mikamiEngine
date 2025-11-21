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

	// コライダーの球体を描画（現在のデバッグカラーを使用）
	debugDrawLineSystem->DrawSphere(center, radius_, currentColliderColor_,3);
}