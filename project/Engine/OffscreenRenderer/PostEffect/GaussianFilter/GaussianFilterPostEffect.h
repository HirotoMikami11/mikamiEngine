#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// ガウシアンフィルタポストエフェクト
/// </summary>
class GaussianFilterPostEffect : public PostEffect {
public:
	struct GaussianFilterParameters {
		int32_t kernelSize = 3;
		float sigma = 1.0f;
		float padding[2] = { 0.0f, 0.0f };
	};

	enum class EffectPreset {
		DEFAULT_3X3,
		SOFT_5X5,
	};

public:
	GaussianFilterPostEffect() { name_ = "Gaussian Filter Effect"; }
	~GaussianFilterPostEffect() = default;

	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) override;
	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }
	PostEffectId GetId() const override;
	std::type_index GetParameterType() const override { return typeid(GaussianFilterParameters); }
	void* GetMutableParametersRaw() override { return &parameters_; }
	const void* GetParametersRaw() const override { return &parameters_; }

	void ApplyPreset(EffectPreset preset);
	void SetKernelSize(int32_t kernelSize);
	void SetSigma(float sigma);
	int32_t GetKernelSize() const { return parameters_.kernelSize; }
	float GetSigma() const { return parameters_.sigma; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	bool isInitialized_ = false;

	GaussianFilterParameters parameters_;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;

	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	GaussianFilterParameters* mappedParameters_ = nullptr;
};
