#include "GaussianFilterPostEffect.h"
#include "ImGui/ImGuiManager.h"

void GaussianFilterPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	CreatePSO();
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "GaussianFilterPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void GaussianFilterPostEffect::Finalize() {
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "GaussianFilterPostEffect finalized.\n");
}

void GaussianFilterPostEffect::Update(float deltaTime) {
	deltaTime;
	if (!isEnabled_ || !isInitialized_) {
		return;
	}
}

void GaussianFilterPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
	if (!isEnabled_ || !isInitialized_ || !renderTriangle) {
		return;
	}

	auto commandList = dxCommon_->GetCommandList();

	commandList->OMSetRenderTargets(1, &outputRTV, false, nullptr);

	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(outputRTV, clearColor, 0, nullptr);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {
		dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr()
	};
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	renderTriangle->DrawWithCustomPSO(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,
		parameterBuffer_->GetGPUVirtualAddress()
	);
}

PostEffectId GaussianFilterPostEffect::GetId() const {
	return PostEffectId::GaussianFilter;
}

void GaussianFilterPostEffect::CreatePSO() {
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/GaussianFilter/GaussianFilter.PS.hlsl");

	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "GaussianFilterPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create GaussianFilter PSO (PSOFactory version)!!\n");
}

void GaussianFilterPostEffect::CreateParameterBuffer() {
	size_t structSize = sizeof(GaussianFilterParameters);
	Logger::Log(Logger::GetStream(), std::format("GaussianFilterParameters size: {} bytes\n", structSize));

	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create GaussianFilter parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> GaussianFilterPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void GaussianFilterPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void GaussianFilterPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::DEFAULT_3X3:
		SetKernelSize(3);
		SetSigma(1.0f);
		SetEnabled(true);
		break;
	case EffectPreset::SOFT_5X5:
		SetKernelSize(5);
		SetSigma(1.4f);
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void GaussianFilterPostEffect::SetKernelSize(int32_t kernelSize) {
	kernelSize = std::clamp(kernelSize, 1, 9);
	if ((kernelSize % 2) == 0) {
		kernelSize += 1;
	}
	parameters_.kernelSize = (std::min)(kernelSize, 9);
	UpdateParameterBuffer();
}

void GaussianFilterPostEffect::SetSigma(float sigma) {
	parameters_.sigma = std::clamp(sigma, 0.1f, 10.0f);
	UpdateParameterBuffer();
}

void GaussianFilterPostEffect::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Default Kernel: 3x3");

		if (isEnabled_) {
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("3x3 Default")) ApplyPreset(EffectPreset::DEFAULT_3X3);
				ImGui::SameLine();
				if (ImGui::Button("5x5 Soft")) ApplyPreset(EffectPreset::SOFT_5X5);
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Manual Settings")) {
				int kernelSize = parameters_.kernelSize;
				if (ImGui::SliderInt("Kernel Size", &kernelSize, 1, 9, "%d x %d")) {
					if ((kernelSize % 2) == 0) {
						kernelSize += 1;
					}
					SetKernelSize(kernelSize);
				}

				float sigma = parameters_.sigma;
				if (ImGui::SliderFloat("Sigma", &sigma, 0.1f, 10.0f, "%.2f")) {
					SetSigma(sigma);
				}

				ImGui::TextDisabled("Odd size only (1,3,5,7,9)");
				ImGui::TreePop();
			}

			ImGui::Separator();
			ImGui::Text("Current Kernel Size: %d x %d", parameters_.kernelSize, parameters_.kernelSize);
			ImGui::Text("Current Sigma: %.2f", parameters_.sigma);
		}

		ImGui::TreePop();
	}
#endif
}
