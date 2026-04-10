#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"
#include "JsonBinder.h"
#include "CollisionManager/Collider/AABBCollider.h"

/// <summary>
/// テスト用壁オブジェクト（AABB コライダー）
/// </summary>
class TestWall : public AABBCollider, public GameObject
{
public:
	TestWall() = default;
	~TestWall() override = default;

	void Initialize();
	void Update();
	void Draw();
	void ImGui();
	void Finalize();

	// --- ICollider ---
	Vector3 GetWorldPosition() override;
	void OnCollisionEnter(ICollider* other) override {}
	void OnCollisionStay(ICollider* other) override {}
	void OnCollisionExit(ICollider* other) override {}

	ObjectTag GetTag() const { return tag_; }

private:
	std::unique_ptr<Model3D> model_;
	std::unique_ptr<JsonBinder> binder_;

	Vector3 aabbSize_ = { 2.0f, 2.0f, 2.0f };

	DirectXCommon* dxCommon_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	ObjectTag tag_ = ObjectTag::TestObject;
};
