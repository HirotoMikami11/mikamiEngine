#pragma once
#include <memory>
#include "Ground.h"
#include "Wall.h"
#include "Torch.h"
#include "GroundLight.h"

class GameField {
public:
	GameField();
	~GameField();

	void Initialize(DirectXCommon* dxCommon);
	void Update(const Matrix4x4& viewProjectionMatrix);
	void Draw();
	void ImGui();

	Ground* GetGround() { return ground_.get(); }
	Wall* GetWall() { return wall_.get(); }
	Torch* GetTorch() { return torch_.get(); }
	GroundLight* GetGroundLight() { return groundLight_.get(); }

private:
	std::unique_ptr<Ground> ground_;
	std::unique_ptr<Wall> wall_;
	std::unique_ptr<Torch> torch_;
	std::unique_ptr<GroundLight> groundLight_;
};
