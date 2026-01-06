#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"

class ModelFont
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	ModelFont();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~ModelFont();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const Vector3& position);

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

	//座標Setter
	void SetPosition(const Vector3& position) {
		if (gameObject_) {
			gameObject_->SetPosition(position);
		}
	};
	//座標Getter
	Vector3 GetPosition() const {
		if (gameObject_) {
			return gameObject_->GetPosition();
		}
		return { 0.0f, 0.0f, 0.0f };
	};


private:
	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;
	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};

