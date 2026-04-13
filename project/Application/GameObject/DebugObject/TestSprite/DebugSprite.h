#pragma once
#include <memory>
#include <vector>
#include "GameObject.h"
#include "DirectXCommon.h"
#include "Sprite.h"

/// <summary>
///　動作確認用にスプライトを大量に表示するためのクラス
/// </summary>
class DebugSprite : public GameObject
{
public:
	DebugSprite() = default;
	~DebugSprite() override = default;

	void Initialize() override;

	void Update() override;
	void Draw() override;
	void ImGui() override;

private:
	void GenerateSprites();

private:
	DirectXCommon* dxCommon_ = nullptr;
	Matrix4x4 spriteViewProjectionMatrix_{};

	std::vector<std::unique_ptr<Sprite>> sprites_;

	Vector2 leftTopPosition_{ 0.0f, 0.0f };
	int columns_ = 30;
	int rows_ = 10;
	Vector2 spriteSize_{ 30.0f, 30.0f };
	Vector2 spacing_{ 2.0f, 2.0f };

	bool needsRegenerate_ = true;
};
