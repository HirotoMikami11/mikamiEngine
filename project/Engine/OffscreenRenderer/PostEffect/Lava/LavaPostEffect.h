#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// 流れる溶岩・マグマポストエフェクト（OffscreenTriangle使用版）
/// ノイズベースで流れるようなマグマを表現
/// </summary>
class LavaPostEffect : public PostEffect {
public:
	/// <summary>
	/// 溶岩エフェクト専用のパラメータ
	/// </summary>
	struct LavaParameters {
		float time = 0.0f;					// 時間（アニメーション用）
		float speed = 1.8f;					// 流れる速度（0.1～3.0）
		float scale = 8.0f;					// ノイズスケール（大きい=細かい模様）
		float distortionStrength = 0.3f;	// 歪みの強度（0.0～1.0）
		
		float brightnessMultiplier = 1.6f;	// 明るさ（0.5～2.0）
		float octaves = 2.0f;				// ノイズのオクターブ数（1～8）
		float blendMode = 0.0f;				// ブレンドモード（0=上書き, 1=加算, 2=乗算）
		float mixRatio = 1.0f;				// 元画像との混合比（0=元画像, 1=溶岩のみ）
		
		float colorHot[4] = { 0.96f, 0.2f, 0.0f, 1.0f };		// 高温部の色（濃いオレンジ）
		float colorMid[4] = { 1.0f, 0.36f, 0.04f, 1.0f };		// 中温部の色（明るめのオレンジ）
		float colorCool[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// 低温部の色（暗い赤）
		float padding[3] = { 0.0f, 0.0f, 0.0f };			// 16バイト境界調整用
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		CALM_LAVA,			// 穏やかな溶岩
		FLOWING_MAGMA,		// 流れるマグマ
		INTENSE_LAVA,		// 激しい溶岩
		FIERY_FLOW,			// 炎のような流れ
		MOLTEN_METAL,		// 溶けた金属風
		BLUE_LAVA,			// 青い溶岩（ファンタジー）
	};

public:
	LavaPostEffect() { name_ = "Lava Effect"; }
	~LavaPostEffect() = default;

	// PostEffect インターフェース実装
	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) override;
	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }

	// 固有メソッド
	void ApplyPreset(EffectPreset preset);
	void SetSpeed(float speed);
	void SetScale(float scale);
	void SetDistortionStrength(float strength);
	void SetBrightness(float brightness);
	void SetMixRatio(float ratio);
	void SetColorHot(float r, float g, float b);
	void SetColorMid(float r, float g, float b);
	void SetColorCool(float r, float g, float b);

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// 溶岩パラメータ
	LavaParameters parameters_;

	// 溶岩エフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	LavaParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};
