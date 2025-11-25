#pragma once
#include "Engine.h"
#include "MyFunction.h"

/// <summary>
/// ダメージエフェクト演出クラス
/// VignettePostEffectを使用して被ダメージ時の赤いビネット演出を行う
/// </summary>
class DamageVignette {
public:
	DamageVignette() = default;
	~DamageVignette() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	void Update(float deltaTime);

	/// <summary>
	/// ダメージエフェクトを開始
	/// </summary>
	void TriggerDamageEffect();

	/// <summary>
	/// イージング速度を設定
	/// </summary>
	/// <param name="speed">イージング速度（大きいほど早い）</param>
	void SetEasingSpeed(float speed) { easingSpeed_ = speed; }

	/// <summary>
	/// エフェクトが実行中かどうか
	/// </summary>
	/// <returns>実行中ならtrue</returns>
	bool IsPlaying() const { return isPlaying_; }

	/// <summary>
	/// ImGuiでのデバッグ表示
	/// </summary>
	void ImGui();

private:
	/// <summary>
	/// ビネットパラメータを設定
	/// </summary>
	void SetupVignetteParameters();

private:
	// VignettePostEffectの参照
	VignettePostEffect* vignetteEffect_ = nullptr;

	// エフェクト状態
	bool isPlaying_ = false;
	float currentTime_ = 0.0f;
	float duration_ = 1.0f;         // エフェクト全体の時間
	float easingSpeed_ = 2.0f;      // イージング速度

	// ビネットパラメータ
	struct DamageVignetteParams {
		Vector4 damageColor = { 0.255f, 0.0f, 0.0f, 1.0f };  // 赤色
		float targetStrength = 1.0f;    // 最大強度
		float radius = 0.486f;          // 半径
		float softness = 0.220f;        // 柔らかさ
	} params_;
};
