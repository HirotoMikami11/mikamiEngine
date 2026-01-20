#include "BinarizationPostEffect.h"
#include "ImGui/ImGuiManager.h"

void BinarizationPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "BinarizationPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void BinarizationPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "BinarizationPostEffect finalized.\n");
}

void BinarizationPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 2値化は基本的に静的なので、特に更新処理は不要
	// 必要に応じてアニメーション処理を追加可能
}

void BinarizationPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
	if (!isEnabled_ || !isInitialized_ || !renderTriangle) {
		return;
	}

	auto commandList = dxCommon_->GetCommandList();

	// レンダーターゲットを設定
	commandList->OMSetRenderTargets(1, &outputRTV, false, nullptr);

	// クリア
	float clearColor[] = { 0.0f, 0.0f, 0.0f, 1.0f };
	commandList->ClearRenderTargetView(outputRTV, clearColor, 0, nullptr);

	// ディスクリプタヒープを設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {
		dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr()
	};
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	// OffscreenTriangleを使用してカスタムPSOで描画
	renderTriangle->DrawWithCustomPSO(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,
		parameterBuffer_->GetGPUVirtualAddress()
	);
}

void BinarizationPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// BinarizationParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/Binarization/Binarization.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "BinarizationPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create Binarization PSO (PSOFactory version)!!\n");
}

void BinarizationPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(BinarizationParameters);
	Logger::Log(Logger::GetStream(), std::format("BinarizationParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create Binarization parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> BinarizationPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	//DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void BinarizationPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void BinarizationPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::BLACK_WHITE:
		parameters_.threshold = 0.023f;
		parameters_.ditherStrength = 0.037f;
		parameters_.smoothness = 0.009f;
		parameters_.color1[0] = 0.0f; parameters_.color1[1] = 0.0f; parameters_.color1[2] = 0.0f;
		parameters_.color2[0] = 1.0f; parameters_.color2[1] = 1.0f; parameters_.color2[2] = 1.0f;

		SetEnabled(true);
		break;

	case EffectPreset::GRAY_WHITE:
		parameters_.threshold = 0.5f;
		parameters_.ditherStrength = 0.0f;
		parameters_.smoothness = 0.0f;
		parameters_.color1[0] = 0.3f; parameters_.color1[1] = 0.3f; parameters_.color1[2] = 0.3f;
		parameters_.color2[0] = 1.0f; parameters_.color2[1] = 1.0f; parameters_.color2[2] = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::BLACK_GRAY:
		parameters_.threshold = 0.5f;
		parameters_.ditherStrength = 0.0f;
		parameters_.smoothness = 0.0f;
		parameters_.color1[0] = 0.0f; parameters_.color1[1] = 0.0f; parameters_.color1[2] = 0.0f;
		parameters_.color2[0] = 0.5f; parameters_.color2[1] = 0.5f; parameters_.color2[2] = 0.5f;
		SetEnabled(true);
		break;

	case EffectPreset::HIGH_CONTRAST:
		parameters_.threshold = 0.4f;
		parameters_.ditherStrength = 0.0f;
		parameters_.smoothness = 0.0f;
		parameters_.color1[0] = 0.0f; parameters_.color1[1] = 0.0f; parameters_.color1[2] = 0.0f;
		parameters_.color2[0] = 1.0f; parameters_.color2[1] = 1.0f; parameters_.color2[2] = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::SOFT:
		parameters_.threshold = 0.5f;
		parameters_.ditherStrength = 0.0f;
		parameters_.smoothness = 0.05f;
		parameters_.color1[0] = 0.0f; parameters_.color1[1] = 0.0f; parameters_.color1[2] = 0.0f;
		parameters_.color2[0] = 1.0f; parameters_.color2[1] = 1.0f; parameters_.color2[2] = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::DITHERED:
		parameters_.threshold = 0.5f;
		parameters_.ditherStrength = 0.5f;
		parameters_.smoothness = 0.0f;
		parameters_.color1[0] = 0.0f; parameters_.color1[1] = 0.0f; parameters_.color1[2] = 0.0f;
		parameters_.color2[0] = 1.0f; parameters_.color2[1] = 1.0f; parameters_.color2[2] = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::NEWSPAPER:
		parameters_.threshold = 0.5f;
		parameters_.ditherStrength = 0.8f;
		parameters_.smoothness = 0.0f;
		parameters_.color1[0] = 0.0f; parameters_.color1[1] = 0.0f; parameters_.color1[2] = 0.0f;
		parameters_.color2[0] = 1.0f; parameters_.color2[1] = 1.0f; parameters_.color2[2] = 1.0f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void BinarizationPostEffect::SetThreshold(float threshold) {
	parameters_.threshold = std::clamp(threshold, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void BinarizationPostEffect::SetDitherStrength(float strength) {
	parameters_.ditherStrength = std::clamp(strength, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void BinarizationPostEffect::SetSmoothness(float smoothness) {
	parameters_.smoothness = std::clamp(smoothness, 0.0f, 0.2f);
	UpdateParameterBuffer();
}

void BinarizationPostEffect::SetColor1(float r, float g, float b) {
	parameters_.color1[0] = std::clamp(r, 0.0f, 1.0f);
	parameters_.color1[1] = std::clamp(g, 0.0f, 1.0f);
	parameters_.color1[2] = std::clamp(b, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void BinarizationPostEffect::SetColor2(float r, float g, float b) {
	parameters_.color2[0] = std::clamp(r, 0.0f, 1.0f);
	parameters_.color2[1] = std::clamp(g, 0.0f, 1.0f);
	parameters_.color2[2] = std::clamp(b, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void BinarizationPostEffect::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("Black & White")) ApplyPreset(EffectPreset::BLACK_WHITE);
				ImGui::SameLine();
				if (ImGui::Button("Gray & White")) ApplyPreset(EffectPreset::GRAY_WHITE);
				ImGui::SameLine();
				if (ImGui::Button("Black & Gray")) ApplyPreset(EffectPreset::BLACK_GRAY);

				if (ImGui::Button("High Contrast")) ApplyPreset(EffectPreset::HIGH_CONTRAST);
				ImGui::SameLine();
				if (ImGui::Button("Soft")) ApplyPreset(EffectPreset::SOFT);

				if (ImGui::Button("Dithered")) ApplyPreset(EffectPreset::DITHERED);
				ImGui::SameLine();
				if (ImGui::Button("Newspaper")) ApplyPreset(EffectPreset::NEWSPAPER);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				// 閾値
				float threshold = parameters_.threshold;
				if (ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f)) {
					SetThreshold(threshold);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Brightness threshold\nLower = More black\nHigher = More white");
				}

				// ディザ強度
				float ditherStrength = parameters_.ditherStrength;
				if (ImGui::SliderFloat("Dither Strength", &ditherStrength, 0.0f, 1.0f)) {
					SetDitherStrength(ditherStrength);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("0 = No dither (sharp)\n1 = Full dither (halftone)");
				}

				// ぼかし
				float smoothness = parameters_.smoothness;
				if (ImGui::SliderFloat("Smoothness", &smoothness, 0.0f, 0.2f)) {
					SetSmoothness(smoothness);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Edge smoothing\n0 = Sharp\n0.1+ = Smooth");
				}

				ImGui::Separator();

				// 色1（暗い側）
				float color1[3] = { parameters_.color1[0], parameters_.color1[1], parameters_.color1[2] };
				if (ImGui::ColorEdit3("Dark Color", color1)) {
					SetColor1(color1[0], color1[1], color1[2]);
				}

				// 色2（明るい側）
				float color2[3] = { parameters_.color2[0], parameters_.color2[1], parameters_.color2[2] };
				if (ImGui::ColorEdit3("Bright Color", color2)) {
					SetColor2(color2[0], color2[1], color2[2]);
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Threshold: %.2f", parameters_.threshold);
			ImGui::Text("Dither: %.2f", parameters_.ditherStrength);
			ImGui::Text("Smoothness: %.3f", parameters_.smoothness);
		}

		ImGui::TreePop();
	}
#endif
}