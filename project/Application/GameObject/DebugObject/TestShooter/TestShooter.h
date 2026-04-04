#pragma once
#include <memory>
#include "GameObject.h"
#include "Object3D.h"
#include "JsonBinder.h"

class GameObjectManager;
class TestPlayer;

/// <summary>
/// デバッグシューター（赤いキューブ）
/// <summary>
class TestShooter : public GameObject
{
public:
	/// <summary>
	/// コンストラクタで依存オブジェクトを注入する
	/// </summary>
	TestShooter(GameObjectManager* objectManager, TestPlayer* target);
	~TestShooter() override = default;

	void Initialize()	override;
	void Update()		override;
	void DrawOffscreen()override;
	void ImGui()		override;
	void Finalize()		override;

	ObjectTag GetTag() const { return tag_; }

private:
	/// <summary>プレイヤーへ向けて弾を1発生成して ObjectManager に追加</summary>
	void Shoot();

	std::unique_ptr<Model3D>    model_;
	std::unique_ptr<JsonBinder> binder_;

	GameObjectManager* objectManager_ = nullptr;
	TestPlayer* target_ = nullptr;

	float bulletLifetime_ = 3.0f;	// 弾の生存時間（秒）
	float bulletSpeed_ = 10.0f;		// 弾の速度
	float bulletRadius_ = 0.3f;		// 弾のコライダー半径

	DirectXCommon* dxCommon_ = nullptr;
	Matrix4x4 viewProjectionMatrix_{};

	ObjectTag tag_ = ObjectTag::TestObject;
};
