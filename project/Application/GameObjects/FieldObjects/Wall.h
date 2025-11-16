#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"
#include <array>

class Wall
{
public:
	Wall();
	~Wall();


	void Initialize(DirectXCommon* dxCommon);
	void Update(const Matrix4x4& viewProjectionMatrix);
	void Draw(const Light& light);
	void ImGui();

	void SetAreaSize(const Vector2& areaSize);

private:
	struct WallObject {
		std::unique_ptr<Model3D> obj;
		Vector3Transform transform;
	};


	void UpdateTransforms();

	DirectXCommon* dxCommon_ = nullptr;

	// 壁モデルサイズ（scale が 1 のときの実寸法）
	// {長さ（X方向に対応）, 高さ, 厚み（Z方向）}
	Vector3 modelSize = { 60.0f, 30.0f, 1.0f };

	// 4方向の壁（前, 右, 後, 左）
	std::array<WallObject, 4> walls_{};

	// 囲みたい領域（フルサイズ）。デフォルトは modelSize.x を幅・奥行きに使う
	Vector2 areaSize_ = { modelSize.x, modelSize.x };
};
