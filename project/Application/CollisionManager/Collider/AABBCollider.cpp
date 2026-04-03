#include "AABBCollider.h"
#include "DebugDrawLineSystem.h"

void AABBCollider::DebugLineAdd() {
	if (!isColliderVisible_) return;

	DebugDrawLineSystem* debugDraw = DebugDrawLineSystem::GetInstance();
	if (!debugDraw) return;

	debugDraw->DrawAABB(GetWorldAABB(), currentColliderColor_);
}
