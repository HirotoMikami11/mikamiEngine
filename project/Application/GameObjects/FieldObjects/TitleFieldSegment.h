#pragma once
#include <memory>
#include "TitleWall.h"
#include "TitleGround.h"

class TitleFieldSegment
{

public:
	TitleFieldSegment();
	~TitleFieldSegment();

	void Initialize(DirectXCommon* dxCommon);
	void Update(const Matrix4x4& viewProjectionMatrix);
	void Draw();
	void ImGui();

	TitleGround* GetGround() { return ground_.get(); }
	TitleWall* GetWall() { return titleWall_.get(); }


private:
	std::unique_ptr<TitleGround> ground_;
	std::unique_ptr<TitleWall> titleWall_;
};
