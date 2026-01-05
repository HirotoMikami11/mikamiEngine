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

	/// <summary>
	/// Z座標オフセットを設定
	/// </summary>
	/// <param name="zOffset">Z座標のオフセット値</param>
	void SetZOffset(float zOffset);

	/// <summary>
	/// 現在のZ座標オフセットを取得
	/// </summary>
	float GetZOffset() const { return zOffset_; }

private:
	std::unique_ptr<TitleGround> ground_;
	std::unique_ptr<TitleGround> skyGround_;
	std::unique_ptr<TitleWall> titleWall_;

	// Z座標オフセット（セグメント全体の位置調整用）
	float zOffset_ = 0.0f;
};
