#include "LavaPostEffect.h"
#include "ImGui/ImGuiManager.h"

void LavaPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "LavaPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void LavaPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "LavaPostEffect finalized.\n");
}

void LavaPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * parameters_.speed;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void LavaPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

void LavaPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// LavaParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/Lava/Lava.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "LavaPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create Lava PSO (PSOFactory version)!!\n");
}

void LavaPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(LavaParameters);
	Logger::Log(Logger::GetStream(), std::format("LavaParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create Lava parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> LavaPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	//DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void LavaPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void LavaPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::CALM_LAVA:
		parameters_.speed = 0.5f;
		parameters_.scale = 2.0f;
		parameters_.distortionStrength = 0.2f;
		parameters_.brightnessMultiplier = 1.0f;
		parameters_.octaves = 4.0f;
		parameters_.mixRatio = 1.0f;
		// 穏やかな赤-オレンジ
		SetColorHot(1.0f, 0.9f, 0.6f);
		SetColorMid(1.0f, 0.3f, 0.1f);
		SetColorCool(0.4f, 0.0f, 0.0f);
		SetEnabled(true);
		break;

	case EffectPreset::FLOWING_MAGMA:
		parameters_.speed = 1.2f;
		parameters_.scale = 3.0f;
		parameters_.distortionStrength = 0.4f;
		parameters_.brightnessMultiplier = 1.2f;
		parameters_.octaves = 5.0f;
		parameters_.mixRatio = 1.0f;
		// 明るいオレンジ-黄色
		SetColorHot(1.0f, 1.0f, 0.8f);
		SetColorMid(1.0f, 0.5f, 0.1f);
		SetColorCool(0.6f, 0.1f, 0.0f);
		SetEnabled(true);
		break;

	case EffectPreset::INTENSE_LAVA:
		parameters_.speed = 2.0f;
		parameters_.scale = 4.0f;
		parameters_.distortionStrength = 0.6f;
		parameters_.brightnessMultiplier = 1.5f;
		parameters_.octaves = 6.0f;
		parameters_.mixRatio = 1.0f;
		// 激しい黄-白
		SetColorHot(1.0f, 1.0f, 1.0f);
		SetColorMid(1.0f, 0.6f, 0.2f);
		SetColorCool(0.8f, 0.0f, 0.0f);
		SetEnabled(true);
		break;

	case EffectPreset::FIERY_FLOW:
		parameters_.speed = 1.5f;
		parameters_.scale = 3.5f;
		parameters_.distortionStrength = 0.5f;
		parameters_.brightnessMultiplier = 1.3f;
		parameters_.octaves = 5.0f;
		parameters_.mixRatio = 1.0f;
		// 炎のような赤-オレンジ-黄
		SetColorHot(1.0f, 1.0f, 0.5f);
		SetColorMid(1.0f, 0.4f, 0.0f);
		SetColorCool(0.5f, 0.0f, 0.0f);
		SetEnabled(true);
		break;

	case EffectPreset::MOLTEN_METAL:
		parameters_.speed = 0.8f;
		parameters_.scale = 2.5f;
		parameters_.distortionStrength = 0.3f;
		parameters_.brightnessMultiplier = 1.1f;
		parameters_.octaves = 4.0f;
		parameters_.mixRatio = 1.0f;
		// 金属的なオレンジ-白
		SetColorHot(1.0f, 1.0f, 0.9f);
		SetColorMid(1.0f, 0.6f, 0.3f);
		SetColorCool(0.3f, 0.1f, 0.05f);
		SetEnabled(true);
		break;

	case EffectPreset::BLUE_LAVA:
		parameters_.speed = 1.0f;
		parameters_.scale = 2.5f;
		parameters_.distortionStrength = 0.4f;
		parameters_.brightnessMultiplier = 1.2f;
		parameters_.octaves = 5.0f;
		parameters_.mixRatio = 1.0f;
		// ファンタジー風の青い溶岩
		SetColorHot(0.8f, 0.9f, 1.0f);
		SetColorMid(0.3f, 0.5f, 1.0f);
		SetColorCool(0.0f, 0.1f, 0.5f);
		SetEnabled(true);
		break;

	case EffectPreset::GREEN_TOXIC:
		parameters_.speed = 1.3f;
		parameters_.scale = 3.0f;
		parameters_.distortionStrength = 0.5f;
		parameters_.brightnessMultiplier = 1.1f;
		parameters_.octaves = 5.0f;
		parameters_.mixRatio = 1.0f;
		// 毒々しい緑
		SetColorHot(0.8f, 1.0f, 0.5f);
		SetColorMid(0.3f, 0.8f, 0.1f);
		SetColorCool(0.1f, 0.3f, 0.0f);
		SetEnabled(true);
		break;

	case EffectPreset::OVERLAY_SUBTLE:
		parameters_.speed = 0.7f;
		parameters_.scale = 2.0f;
		parameters_.distortionStrength = 0.2f;
		parameters_.brightnessMultiplier = 0.8f;
		parameters_.octaves = 4.0f;
		parameters_.mixRatio = 0.3f;  // 元画像と混合
		parameters_.blendMode = 1.0f;  // 加算ブレンド
		// 控えめなオレンジ
		SetColorHot(1.0f, 0.8f, 0.5f);
		SetColorMid(1.0f, 0.4f, 0.1f);
		SetColorCool(0.3f, 0.0f, 0.0f);
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void LavaPostEffect::SetSpeed(float speed) {
	parameters_.speed = std::clamp(speed, 0.0f, 5.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetScale(float scale) {
	parameters_.scale = std::clamp(scale, 0.5f, 10.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetDistortionStrength(float strength) {
	parameters_.distortionStrength = std::clamp(strength, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetBrightness(float brightness) {
	parameters_.brightnessMultiplier = std::clamp(brightness, 0.1f, 3.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetMixRatio(float ratio) {
	parameters_.mixRatio = std::clamp(ratio, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetColorHot(float r, float g, float b) {
	parameters_.colorHot[0] = std::clamp(r, 0.0f, 1.0f);
	parameters_.colorHot[1] = std::clamp(g, 0.0f, 1.0f);
	parameters_.colorHot[2] = std::clamp(b, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetColorMid(float r, float g, float b) {
	parameters_.colorMid[0] = std::clamp(r, 0.0f, 1.0f);
	parameters_.colorMid[1] = std::clamp(g, 0.0f, 1.0f);
	parameters_.colorMid[2] = std::clamp(b, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::SetColorCool(float r, float g, float b) {
	parameters_.colorCool[0] = std::clamp(r, 0.0f, 1.0f);
	parameters_.colorCool[1] = std::clamp(g, 0.0f, 1.0f);
	parameters_.colorCool[2] = std::clamp(b, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LavaPostEffect::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("Calm Lava")) ApplyPreset(EffectPreset::CALM_LAVA);
				ImGui::SameLine();
				if (ImGui::Button("Flowing Magma")) ApplyPreset(EffectPreset::FLOWING_MAGMA);
				ImGui::SameLine();
				if (ImGui::Button("Intense Lava")) ApplyPreset(EffectPreset::INTENSE_LAVA);

				if (ImGui::Button("Fiery Flow")) ApplyPreset(EffectPreset::FIERY_FLOW);
				ImGui::SameLine();
				if (ImGui::Button("Molten Metal")) ApplyPreset(EffectPreset::MOLTEN_METAL);

				if (ImGui::Button("Blue Lava")) ApplyPreset(EffectPreset::BLUE_LAVA);
				ImGui::SameLine();
				if (ImGui::Button("Green Toxic")) ApplyPreset(EffectPreset::GREEN_TOXIC);

				if (ImGui::Button("Overlay Subtle")) ApplyPreset(EffectPreset::OVERLAY_SUBTLE);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				// 速度
				float speed = parameters_.speed;
				if (ImGui::SliderFloat("Speed", &speed, 0.0f, 3.0f)) {
					SetSpeed(speed);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Flow speed\n0 = Static\n1 = Normal\n3 = Very fast");
				}

				// スケール
				float scale = parameters_.scale;
				if (ImGui::SliderFloat("Scale", &scale, 0.5f, 8.0f)) {
					SetScale(scale);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Pattern scale\nLower = Larger patterns\nHigher = Finer details");
				}

				// 歪み
				float distortion = parameters_.distortionStrength;
				if (ImGui::SliderFloat("Distortion", &distortion, 0.0f, 1.0f)) {
					SetDistortionStrength(distortion);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Warping strength\n0 = No distortion\n1 = Maximum distortion");
				}

				// 明るさ
				float brightness = parameters_.brightnessMultiplier;
				if (ImGui::SliderFloat("Brightness", &brightness, 0.5f, 2.0f)) {
					SetBrightness(brightness);
				}

				// オクターブ数
				int octaves = static_cast<int>(parameters_.octaves);
				if (ImGui::SliderInt("Octaves", &octaves, 1, 8)) {
					parameters_.octaves = static_cast<float>(octaves);
					UpdateParameterBuffer();
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("Noise detail level\nHigher = More detailed (but slower)");
				}

				// 混合比
				float mixRatio = parameters_.mixRatio;
				if (ImGui::SliderFloat("Mix Ratio", &mixRatio, 0.0f, 1.0f)) {
					SetMixRatio(mixRatio);
				}
				if (ImGui::IsItemHovered()) {
					ImGui::SetTooltip("0 = Original image\n1 = Full lava effect");
				}

				// ブレンドモード
				const char* blendModes[] = { "Replace", "Add", "Multiply" };
				int currentBlend = static_cast<int>(parameters_.blendMode);
				if (ImGui::Combo("Blend Mode", &currentBlend, blendModes, 3)) {
					parameters_.blendMode = static_cast<float>(currentBlend);
					UpdateParameterBuffer();
				}

				ImGui::TreePop();
			}

			// カラー設定
			if (ImGui::TreeNode("Color Settings")) {
				// 高温部
				float colorHot[3] = { parameters_.colorHot[0], parameters_.colorHot[1], parameters_.colorHot[2] };
				if (ImGui::ColorEdit3("Hot Color", colorHot)) {
					SetColorHot(colorHot[0], colorHot[1], colorHot[2]);
				}

				// 中温部
				float colorMid[3] = { parameters_.colorMid[0], parameters_.colorMid[1], parameters_.colorMid[2] };
				if (ImGui::ColorEdit3("Mid Color", colorMid)) {
					SetColorMid(colorMid[0], colorMid[1], colorMid[2]);
				}

				// 低温部
				float colorCool[3] = { parameters_.colorCool[0], parameters_.colorCool[1], parameters_.colorCool[2] };
				if (ImGui::ColorEdit3("Cool Color", colorCool)) {
					SetColorCool(colorCool[0], colorCool[1], colorCool[2]);
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Time: %.2f", parameters_.time);
			ImGui::Text("Speed: %.2f", parameters_.speed);
			ImGui::Text("Scale: %.2f", parameters_.scale);
			ImGui::Text("Distortion: %.2f", parameters_.distortionStrength);
			ImGui::Text("Octaves: %.0f", parameters_.octaves);
		}

		ImGui::TreePop();
	}
#endif
}
