#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"
#include "JsonBinder.h"
#include "CollisionManager/Collider/SphereCollider.h"

/// <summary>
/// テスト用オブジェクト（Sphere コライダー）
/// </summary>
class TestObject : public SphereCollider, public GameObject
{
public:
	TestObject() = default;
	~TestObject() override = default;

	void Initialize();
	void Update();
	void DrawOffscreen();
	void ImGui();
	void Finalize();

	// --- ICollider ---
	Vector3 GetWorldPosition() override;
	void OnCollisionEnter(ICollider* other) override {}
	void OnCollisionStay(ICollider* other) override {}
	void OnCollisionExit(ICollider* other) override {}

	ObjectTag GetTag() const { return tag_; }

private:
	std::unique_ptr<Sphere> model_;
	std::unique_ptr<JsonBinder> binder_;

	DirectXCommon* dxCommon_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	ObjectTag tag_ = ObjectTag::TestObject;
};
