#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"
#include "JsonBinder.h"
#include "CollisionManager/Collider/SphereCollider.h"

/// <summary>
/// テスト用プレイヤー
/// 衝突判定（Enter/Stay/Exit）確認用
/// </summary>
class TestPlayer : public SphereCollider, public GameObject
{
public:
	TestPlayer() = default;
	~TestPlayer() override = default;

	void Initialize();
	void Update();
	void Draw();
	void ImGui();
	void Finalize();

	//コライダー設定
	Vector3 GetWorldPosition() override;
	void OnCollisionEnter(ICollider* other) override;
	void OnCollisionStay(ICollider* other) override;
	void OnCollisionExit(ICollider* other) override;

	ObjectTag GetTag() const { return tag_; }

private:
	std::unique_ptr<Sphere> model_;
	std::unique_ptr<JsonBinder> binder_;

	float moveSpeed_ = 5.0f;

	DirectXCommon* dxCommon_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	ObjectTag tag_ = ObjectTag::Player;
};
