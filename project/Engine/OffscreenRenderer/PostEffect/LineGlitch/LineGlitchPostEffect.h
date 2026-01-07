#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// ラインをずらすグリッチエフェクト（OffscreenTriangle使用版）
/// lineCount対応：粗い/細かいライングリッチを選択可能
/// </summary>
class LineGlitchPostEffect : public PostEffect {
public:
	/// <summary>
	/// ライングリッチ専用のパラメータ
	/// </summary>
	struct LineGlitchParameters {
		float time = 0.0f;					// 時間（アニメーション用）
		float noiseIntensity = 1.0f;		// グリッチの強度 (0.0f～10.0f)
		float noiseInterval = 0.8f;			// ノイズが起こる頻度(0近ければ近いほど起こりやすく、増やすほど起こりにくくなる)
		float animationSpeed = 1.0f;		// 全体に適応する時間(内部時間にかける値)
		float lineCount = 20.0f;			// 縦の分割数（小さい値=粗いライン、大きい値=細かいライン）
		float padding[3] = { 0.0f, 0.0f, 0.0f };  // 16バイト境界調整用
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,			// エフェクトなし
		COARSE,			// 粗いライングリッチ（5-10本）
		SUBTLE,			// 軽微なライングリッチ（20-30本）
		MEDIUM,			// 中程度のライングリッチ（50-100本）
		INTENSE,		// 強烈なライングリッチ（200-400本）
	};

public:
	LineGlitchPostEffect() { name_ = "LineGlitch Effect"; }
	~LineGlitchPostEffect() = default;

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
	void SetNoiseIntensity(float intensity);
	void SetNoiseInterval(float interval);
	void SetLineCount(float count);

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// ライングリッチパラメータ
	LineGlitchParameters parameters_;

	// グリッチエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	LineGlitchParameters* mappedParameters_ = nullptr;

	// アニメーション用
	float animationSpeed_ = 1.0f;
};