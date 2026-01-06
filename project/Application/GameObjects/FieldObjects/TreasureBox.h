#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"

class TreasureBox
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TreasureBox();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TreasureBox();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// 座標設定
	/// </summary>
	/// <param name="translation"></param>
	void SetPosition(const Vector3& translation) {
		if (gameObject_) {
			gameObject_->SetPosition(translation);
		}
	}

private:
	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;
	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};

