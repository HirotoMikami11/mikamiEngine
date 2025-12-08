#include "OutlinePostEffect.h"
#include "ImGui/ImGuiManager.h" 

void OutlinePostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "OutlinePostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void OutlinePostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "OutlinePostEffect finalized (OffscreenTriangle version).\n");
}

void OutlinePostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 特にアニメーションは必要ないが、将来の拡張用に残しておく
}

void OutlinePostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
	// 基底クラスのApplyは使用しない（深度テクスチャが必要なため）
	// 代わりに専用のApplyを使用することを推奨
	Logger::Log(Logger::GetStream(), "Warning: OutlinePostEffect requires depth texture. Use Apply with depthSRV parameter.\n");
}

void OutlinePostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

	// OffscreenTriangleの深度対応描画関数を使用
	renderTriangle->DrawWithCustomPSOAndDepth(
		rootSignature_.Get(),
		pipelineState_.Get(),
		inputSRV,		// カラーテクスチャ
		depthSRV,		// 深度テクスチャ
		parameterBuffer_->GetGPUVirtualAddress()	// パラメータバッファ
	);
}

void OutlinePostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)	// OutlineParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// ColorTexture (t0)
		.AddSRV(1, 1, D3D12_SHADER_VISIBILITY_PIXEL)	// DepthTexture (t1)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectWithDepth()
		.SetPixelShader(L"resources/Shader/Outline/Outline.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "OutlinePostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create Outline PSO (PSOFactory version)!!\n");
}

void OutlinePostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(OutlineParameters);
	Logger::Log(Logger::GetStream(), std::format("OutlineParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create Outline parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> OutlinePostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	//DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void OutlinePostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void OutlinePostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::OFF:
		SetEnabled(false);
		break;

	case EffectPreset::THIN_BLACK:
		parameters_.outlineColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		parameters_.depthThreshold = 0.05f;
		parameters_.outlineThickness = 1.0f;
		parameters_.outlineStrength = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::THICK_BLACK:
		parameters_.outlineColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		parameters_.depthThreshold = 0.08f;
		parameters_.outlineThickness = 2.0f;
		parameters_.outlineStrength = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::ANIME:
		parameters_.outlineColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		parameters_.depthThreshold = 0.1f;
		parameters_.outlineThickness = 2.5f;
		parameters_.outlineStrength = 1.0f;
		SetEnabled(true);
		break;

	case EffectPreset::COLORED:
		parameters_.outlineColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		parameters_.depthThreshold = 0.06f;
		parameters_.outlineThickness = 1.5f;
		parameters_.outlineStrength = 0.8f;
		SetEnabled(true);
		break;

	case EffectPreset::SUBTLE:
		parameters_.outlineColor = { 0.2f, 0.2f, 0.2f, 1.0f };
		parameters_.depthThreshold = 0.03f;
		parameters_.outlineThickness = 1.0f;
		parameters_.outlineStrength = 0.5f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void OutlinePostEffect::SetOutlineColor(const Vector4& color) {
	parameters_.outlineColor = color;
	UpdateParameterBuffer();
}

void OutlinePostEffect::SetDepthThreshold(float threshold) {
	parameters_.depthThreshold = std::clamp(threshold, 0.001f, 1.0f);
	UpdateParameterBuffer();
}

void OutlinePostEffect::SetOutlineThickness(float thickness) {
	parameters_.outlineThickness = std::clamp(thickness, 0.1f, 5.0f);
	UpdateParameterBuffer();
}

void OutlinePostEffect::SetOutlineStrength(float strength) {
	parameters_.outlineStrength = std::clamp(strength, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void OutlinePostEffect::SetScreenSize(float width, float height) {
	parameters_.screenSize = { width, height };
	UpdateParameterBuffer();
}

void OutlinePostEffect::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("Off")) ApplyPreset(EffectPreset::OFF);
				ImGui::SameLine();
				if (ImGui::Button("Thin Black")) ApplyPreset(EffectPreset::THIN_BLACK);
				ImGui::SameLine();
				if (ImGui::Button("Thick Black")) ApplyPreset(EffectPreset::THICK_BLACK);
				
				if (ImGui::Button("Anime Style")) ApplyPreset(EffectPreset::ANIME);
				ImGui::SameLine();
				if (ImGui::Button("Colored")) ApplyPreset(EffectPreset::COLORED);
				ImGui::SameLine();
				if (ImGui::Button("Subtle")) ApplyPreset(EffectPreset::SUBTLE);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				Vector4 outlineColor = parameters_.outlineColor;
				if (ImGui::ColorEdit4("Outline Color", reinterpret_cast<float*>(&outlineColor.x))) {
					SetOutlineColor(outlineColor);
				}

				float depthThreshold = parameters_.depthThreshold;
				if (ImGui::SliderFloat("Depth Threshold", &depthThreshold, 0.001f, 1.0f)) {
					SetDepthThreshold(depthThreshold);
				}

				float outlineThickness = parameters_.outlineThickness;
				if (ImGui::SliderFloat("Outline Thickness", &outlineThickness, 0.001f, 5.0f)) {
					SetOutlineThickness(outlineThickness);
				}

				float outlineStrength = parameters_.outlineStrength;
				if (ImGui::SliderFloat("Outline Strength", &outlineStrength, 0.0f, 1.0f)) {
					SetOutlineStrength(outlineStrength);
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Screen Size: %.0f x %.0f", parameters_.screenSize.x, parameters_.screenSize.y);
			ImGui::Text("Depth Threshold: %.3f", parameters_.depthThreshold);
			ImGui::Text("Thickness: %.2f", parameters_.outlineThickness);
			ImGui::Text("Strength: %.2f", parameters_.outlineStrength);
		}

		ImGui::TreePop();
	}
#endif
}