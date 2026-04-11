#pragma once
#include "GameObjectTypes.h"

/// <summary>
/// ゲームオブジェクト基底クラス
/// 全てのゲームオブジェクトはこれを継承する
/// </summary>
class GameObject
{
public:
	virtual ~GameObject() = default;

	/// <summary>
	/// 初期化（Scene 側で明示的に呼ぶ）
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画（Submit のみ。3D/UIどちらもここで行う）
	/// RenderGroup で描画先が決まるため DrawOffscreen/DrawBackBuffer は不要
	/// </summary>
	virtual void Draw() {}

	/// <summary>
	/// デバッグ描画（ImGui等）
	/// </summary>
	virtual void ImGui() {}

	/// <summary>
	/// 終了処理（Destroy 直前に呼ばれる）
	/// </summary>
	virtual void Finalize() {}

	/// <summary>
	/// 破棄済みか
	/// </summary>
	bool IsDestroyed() const { return isDestroyed_; }

	/// <summary>
	/// 種類を取得
	/// </summary>
	ObjectTag GetTag() const { return tag_; }

	/// <summary>
	/// 処理順を取得（小さいほど先に処理）
	/// </summary>
	int GetUpdateOrder() const;

protected:
	/// <summary>
	/// 破棄フラグを立てる
	/// 次フレームの Update 後に GameObjectManager から除去される
	/// </summary>
	void Destroy() { isDestroyed_ = true; }

	/// <summary>
	/// 種類を設定
	/// </summary>
	void SetTag(ObjectTag tag) { tag_ = tag; }

private:
	bool isDestroyed_ = false;
	ObjectTag tag_ = ObjectTag::Default;
};
