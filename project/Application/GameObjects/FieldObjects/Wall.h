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
	void Draw();
	void ImGui();

	void SetAreaSize(const Vector2& areaSize);

	// JsonSettings関連
	void SaveToJson();
	void ApplyParameters();

private:
	struct WallObject {
		std::unique_ptr<Model3D> obj;
		Vector3Transform transform;
	};


	void UpdateTransforms();

	//システム参照
	DirectXCommon* dxCommon_;

	// 壁モデルサイズ（scale が 1 のときの実寸法）
	Vector3 modelSize;

	// 4方向の壁（前, 右, 後, 左）
	std::array<WallObject, 4> walls_{};

	// 囲みたい領域（フルサイズ)
	Vector2 areaSize_;

	// JsonSettingsのグループパス
	const std::vector<std::string> kGroupPath_ = { "Wall" };
	
};