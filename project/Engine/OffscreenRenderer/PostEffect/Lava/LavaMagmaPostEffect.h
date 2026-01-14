#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// 溶岩マグマポストエフェクト（ShaderToy完全再現版）
/// ShaderToyのコードを1:1で再現し、すべてのパラメータを調整可能に
/// </summary>
class LavaMagmaPostEffect : public PostEffect {
public:
	/// <summary>
	/// 溶岩マグマ専用のパラメータ
	/// </summary>
	struct LavaMagmaParameters {
		float time = 0.0f;					// 時間（アニメーション用）
		float speedMultiplier = 1.0f;		// 速度倍率（0.1～3.0）
		float scale = 7.5f;					// UVスケール（1.0～15.0）
		float mixRatio = 1.0f;				// 元画像との混合比（0=元画像, 1=溶岩のみ）

		// ShaderToyのspeed（vec2(0.35, 0.35)）
		float speedX = 0.35f;				// X方向の速度
		float speedY = 0.35f;				// Y方向の速度

		// ShaderToyのrand引数
		float randSeed1X = 1.0f;			// rand(vec2(1., 5.))のX
		float randSeed1Y = 5.0f;			// rand(vec2(1., 5.))のY

		float randSeed2X = 100.0f;			// rand(vec2(100., 100.))のX
		float randSeed2Y = 100.0f;			// rand(vec2(100., 100.))のY
		float padding1 = 0.0f;
		float padding2 = 0.0f;

		// ShaderToyの6色（col1～col6）
		float color1[4] = { 0.5f, 0.0f, 0.1f, 1.0f };	// col1
		float color2[4] = { 0.9f, 0.3f, 0.1f, 1.0f };	// col2
		float color3[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// col3
		float color4[4] = { 1.0f, 0.9f, 0.6f, 1.0f };	// col4
		float color5[4] = { 0.1f, 0.4f, 0.8f, 1.0f };	// col5
		float color6[4] = { 1.155f, 1.155f, 1.155f, 1.0f };	// col6

		float brightnessMultiplier = 1.0f;	// 明るさ調整
		float padding3[3] = { 0.0f, 0.0f, 0.0f };  // 16バイト境界調整用
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		SHADERTOY_ORIGINAL,		// ShaderToyオリジナル（完全再現）
		LAVA_ORANGE,			// オレンジ溶岩
		MY_LAVA,			// 自分で設定した奴
		LARGE_SCALE,			// 大きなスケール（粗い）
		SMALL_SCALE,			// 小さなスケール（細かい）
	};

public:
	LavaMagmaPostEffect() { name_ = "Lava Magma Effect"; }
	~LavaMagmaPostEffect() = default;

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
	void SetSpeedMultiplier(float multiplier);
	void SetScale(float scale);
	void SetSpeedX(float speedX);
	void SetSpeedY(float speedY);
	void SetMixRatio(float ratio);
	void SetBrightness(float brightness);

	// 6色の設定
	void SetColor1(float r, float g, float b);
	void SetColor2(float r, float g, float b);
	void SetColor3(float r, float g, float b);
	void SetColor4(float r, float g, float b);
	void SetColor5(float r, float g, float b);
	void SetColor6(float r, float g, float b);

	// rand引数の設定
	void SetRandSeed1(float x, float y);
	void SetRandSeed2(float x, float y);

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// 溶岩マグマパラメータ
	LavaMagmaParameters parameters_;

	// 溶岩マグマエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	LavaMagmaParameters* mappedParameters_ = nullptr;
};