#pragma once
#include <string>
#include "Engine.h"
#include "DirectXCommon.h"

#include "Sprite.h"
#include "LightManager.h"
#include "Object3D.h"
#include "CameraController.h"
#include "ParticleSystem.h"
#include "ParticleEditor.h"
#include "GameObjectManager.h"
#include "CollisionManager/CollisionManager.h"


/// <summary>
/// シーンの基底クラス
/// 派生クラスは Onなんとか関数を override して独自処理を描く
///　オブジェクト生成パターン
/// 派生クラスのコンストラクタでmake_uniqueしてAddObject()する。
/// </summary>
class BaseScene {
public:
	explicit BaseScene(const std::string& sceneName);
	virtual ~BaseScene() = default;

	// SceneManager から呼ばれる public final メソッド

	/// <summary>
	/// オブジェクト初期化
	/// </summary>
	virtual void Initialize() final;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() final;

	/// <summary>
	/// 3D描画処理（オフスクリーン内で描画される）
	/// グリッド、3Dオブジェクト、ライティングなど
	/// </summary>
	virtual void DrawOffscreen() final;

	/// <summary>
	/// UI描画処理（オフスクリーン外で描画される）
	/// スプライト、テキスト、2D要素など
	/// </summary>
	virtual void DrawBackBuffer() final;


	/// 終了処理
	/// </summary>
	virtual void Finalize() final;

	/// <summary>
	/// ImGui描画
	/// </summary>
	virtual void ImGui() final;


	/// <summary>

	/// <summary>
	/// シーンに入った時のオフスクリーン設定
	/// </summary>
	virtual void ConfigureOffscreenEffects() {}


	// オブジェクト初期化状態の管理
	bool IsInitialized() const { return initialized_; }
	void SetInitialized(bool initialized) { initialized_ = initialized; }

	// シーン名の取得
	const std::string& GetSceneName() const { return sceneName_; }

protected:
	// 派生クラスが override する関数

	/// <summary>
	///カメラ・ライト・Manager外オブジェクト生成など
	/// </summary>
	virtual void OnInitialize() {}

	/// <summary>
	///カメラ更新・Manager外オブジェクト更新（VP行列確定）
	/// </summary>
	virtual void OnUpdate() {}

	/// <summary>
	/// Manager外の描画
	/// 3D・UI問わず Submit する
	/// </summary>
	virtual void OnDraw() {}

	/// <summary>
	/// Manager外のUI描画
	/// バックバッファ直接描画が必要な場合のみ
	/// </summary>
	virtual void OnDrawBackBuffer() {}

	/// <summary>
	///Manager外の ImGui
	/// </summary>
	virtual void OnImGui() {}

	/// <summary>
	///rawポインタのnullptr化など（オブジェクトはまだ生存中
	/// ）</summary>
	virtual void OnFinalize() {}

	/// <summary>
	/// Manager外コライダーの追加登録
	/// SyncColliders 直後・CollisionManager::Update() 直前に呼ばれる
	/// Boss の複数コライダー等が必要なシーンのみ override する
	/// </summary>
	virtual void OnHandleCollisions() {}

	std::string        sceneName_;
	GameObjectManager  gameObjectManager_;
	CollisionManager   collisionManager_;

private:
	/// <summary>
	/// コライダーリストをクリア → Manager内を登録 → OnHandleCollisions() → 衝突判定
	/// BaseScene::Update() final から毎フレーム呼ばれる
	/// </summary>
	void HandleCollisions();

	bool initialized_ = false; 	// オブジェクト初期化済みフラグ（切り替え時リセット）
};