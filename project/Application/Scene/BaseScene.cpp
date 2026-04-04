#include "BaseScene.h"

BaseScene::BaseScene(const std::string& sceneName)
	: sceneName_(sceneName)
{
}

void BaseScene::Initialize()
{
	// カメラ・ライト・Manager外オブジェクト生成など
	OnInitialize();

	// タグ順（GetUpdateOrder 昇順）で全GameObject::Initialize()をする
	gameObjectManager_.InitializeAll();

}

void BaseScene::Update()
{
	// カメラ更新・Manager外オブジェクト更新（VP行列を GameObject より先に確定）
	OnUpdate();

	// GameObjectManager 内オブジェクトをタグ順で更新
	gameObjectManager_.Update();

	// コライダー登録と衝突判定
	HandleCollisions();
}

void BaseScene::DrawOffscreen()
{
	// Manager内オブジェクトの3D描画
	gameObjectManager_.DrawOffscreen();

	// Manager外オブジェクトの3D描画
	OnDrawOffscreen();
}

void BaseScene::DrawBackBuffer()
{
	// Manager内オブジェクトのUI描画
	gameObjectManager_.DrawBackBuffer();

	// Manager外オブジェクトのUI描画
	OnDrawBackBuffer();
}

void BaseScene::ImGui()
{
	// Manager内オブジェクトの ImGui
	gameObjectManager_.ImGui();

	// Manager外オブジェクトの ImGui
	OnImGui();
}

void BaseScene::Finalize()
{
	// 生ポインタのnullptr化など（オブジェクトはまだ生存中）
	OnFinalize();

	// 全GameObject::Finalize()とメモリ解放
	gameObjectManager_.Clear();

	// prevPairs/currentPairs のリセット（シーン使い回し対策）
	collisionManager_.Initialize();
}

void BaseScene::HandleCollisions()
{
	// 毎フレームリストをクリアしてから再登録（解放済みポインタの混入防止）
	collisionManager_.ClearColliderList();

	// Manager内の ICollider を継承するオブジェクトを全登録
	gameObjectManager_.AddAllCollidersToManager(&collisionManager_);

	// Manager外コライダーの追加登録（必要なシーンのみ override）
	OnHandleCollisions();

	// Enter / Stay / Exit コールバックの実行
	collisionManager_.Update();
}
