#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// 白黒2値化ポストエフェクト
/// 輝度を基準に白と黒に分ける
/// </summary>
class BinarizationPostEffect : public PostEffect {
public:
	/// <summary>
	/// 2値化専用のパラメータ
	/// </summary>
	struct BinarizationParameters {
		float threshold = 0.023f;					// 閾値（0.0～1.0）
		float ditherStrength = 0.037f;			// ディザの強度（0.0=OFF, 1.0=MAX）
		float smoothness = 0.009f;				// 境界のぼかし（0.0=カクカク, 0.1=滑らか）
		float padding = 0.0f;					// 16バイト境界調整用

		float color1[4] = { 0.0f, 0.0f, 0.0f, 1.0f };	// 暗い側の色（デフォルト：黒）
		float color2[4] = { 1.0f, 1.0f, 1.0f, 1.0f };	// 明るい側の色（デフォルト：白）
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		BLACK_WHITE,		// 純粋な白黒
		GRAY_WHITE,			// グレー＆白
		BLACK_GRAY,			// 黒＆グレー
		HIGH_CONTRAST,		// ハイコントラスト（くっきり）
		SOFT,				// ソフト（境界ぼかし）
		DITHERED,			// ディザリング風
		NEWSPAPER,			// 新聞印刷風
	};

public:
	BinarizationPostEffect() { name_ = "Binarization Effect"; }
	~BinarizationPostEffect() = default;

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
	void SetThreshold(float threshold);
	void SetDitherStrength(float strength);
	void SetSmoothness(float smoothness);
	void SetColor1(float r, float g, float b);
	void SetColor2(float r, float g, float b);

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// 2値化パラメータ
	BinarizationParameters parameters_;

	// 2値化エフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	BinarizationParameters* mappedParameters_ = nullptr;
};