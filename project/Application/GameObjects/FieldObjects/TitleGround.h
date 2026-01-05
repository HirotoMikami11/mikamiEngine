#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"

class TitleGround
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TitleGround();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleGround();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position);

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

private:
	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;
	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
};

