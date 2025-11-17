#pragma once
#include <memory>
#include "Engine.h"
#include "Sprite.h"

/// <summary>
/// プレイヤーのUI管理クラス（ゲージなど）
/// </summary>
class PlayerUI {
public:
	PlayerUI() = default;
	~PlayerUI() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrixSprite">スプライト用ビュープロジェクション行列</param>
	void Update(float currentHP, float maxHP, const Matrix4x4& viewProjectionMatrixSprite);

	/// <summary>
	/// UI描画（ゲージ）
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	// UI設定
	void SetGaugePosition(const Vector2& hpPosition);
	void SetGaugeSize(const Vector2& size, const Vector2 frameOffset);
	void SetGaugeColors(const Vector4& hpColor, const Vector4& backgroundColor);

private:
	// システム参照
	DirectXCommon* directXCommon_ = nullptr;

	// HP/ENゲージ用スプライト
	std::unique_ptr<Sprite> hpGaugeBar_;		 // HPゲージの枠
	std::unique_ptr<Sprite> hpGaugeFill_;		  // HPゲージの中身
	std::unique_ptr<Sprite> hpGaugeLight_;		  // HPゲージの中身

	// ゲージ設定
	Vector2 hpGaugePosition_;
	Vector2 gaugeSize_;
	Vector2 gaugeFrameSize_;
	Vector2 frameOffset_;
	float LightScrollSpeed_;
	float originalLightWidth_;
	Vector4 hpNormalColor_{ 0.0f, 1.0f, 0.0f, 1.0f };	//黄色
	Vector4 backgroundColor_{ 0.2f, 0.2f, 0.2f, 1.0f };	 // 暗い灰色

	/// <summary>
	/// HPゲージの初期化
	/// </summary>
	void InitializeGauges();

	/// <summary>
	/// ゲージの更新（色とサイズ）
	/// </summary>
	void UpdateGauges(float currentHP, float maxHP);
};