#include "SphereCollider.h"
#include "DebugDrawLineSystem.h"

void SphereCollider::DebugLineAdd() {
	if (!isColliderVisible_) return;

	DebugDrawLineSystem* debugDraw = DebugDrawLineSystem::GetInstance();
	if (!debugDraw) return;

	debugDraw->DrawSphere(GetWorldPosition(), radius_, currentColliderColor_);
}
