#include "LavaMagmaPostEffect.h"
#include "ImGui/ImGuiManager.h"

void LavaMagmaPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "LavaMagmaPostEffect initialized successfully!\n");
}

void LavaMagmaPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "LavaMagmaPostEffect finalized.\n");
}

void LavaMagmaPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime * parameters_.speedMultiplier;

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

void LavaMagmaPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// LavaMagmaParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/Lava/LavaMagma.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "LavaMagmaPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create LavaMagma PSO!!\n");
}

void LavaMagmaPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(LavaMagmaParameters);
	Logger::Log(Logger::GetStream(), std::format("LavaMagmaParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create LavaMagma parameter buffer!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> LavaMagmaPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	//DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void LavaMagmaPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void LavaMagmaPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::SHADERTOY_ORIGINAL:
		// ShaderToyのオリジナル設定（完全再現）
		parameters_.speedMultiplier = 1.0f;
		parameters_.scale = 7.5f;
		parameters_.speedX = 0.35f;
		parameters_.speedY = 0.35f;
		parameters_.randSeed1X = 1.0f;
		parameters_.randSeed1Y = 5.0f;
		parameters_.randSeed2X = 100.0f;
		parameters_.randSeed2Y = 100.0f;
		parameters_.mixRatio = 1.0f;
		parameters_.brightnessMultiplier = 1.0f;
		// ShaderToyのオリジナル色
		SetColor1(0.5f, 0.0f, 0.1f);
		SetColor2(0.9f, 0.3f, 0.1f);
		SetColor3(0.0f, 0.0f, 0.0f);
		SetColor4(1.0f, 0.9f, 0.6f);
		SetColor5(0.1f, 0.4f, 0.8f);
		SetColor6(1.155f, 1.155f, 1.155f);
		SetEnabled(true);
		break;

	case EffectPreset::LAVA_ORANGE:
		// オレンジ溶岩
		parameters_.speedMultiplier = 1.0f;
		parameters_.scale = 7.5f;
		parameters_.speedX = 0.35f;
		parameters_.speedY = 0.35f;
		parameters_.randSeed1X = 1.0f;
		parameters_.randSeed1Y = 5.0f;
		parameters_.randSeed2X = 100.0f;
		parameters_.randSeed2Y = 100.0f;
		parameters_.mixRatio = 1.0f;
		parameters_.brightnessMultiplier = 1.2f;
		SetColor1(0.6f, 0.2f, 0.0f);
		SetColor2(1.0f, 0.5f, 0.1f);
		SetColor3(0.0f, 0.0f, 0.0f);
		SetColor4(1.0f, 1.0f, 0.7f);
		SetColor5(0.0f, 0.0f, 0.0f);
		SetColor6(1.0f, 1.0f, 0.9f);
		SetEnabled(true);
		break;

	case EffectPreset::MY_LAVA:
		// 緑の毒液
		parameters_.speedMultiplier = 1.9f;
		parameters_.scale = 15.0f;
		parameters_.speedX = 0.23f;
		parameters_.speedY = 0.95f;
		parameters_.randSeed1X = 1.0f;
		parameters_.randSeed1Y = 5.0f;
		parameters_.randSeed2X = 100.0f;
		parameters_.randSeed2Y = 100.0f;
		parameters_.mixRatio = 1.0f;
		parameters_.brightnessMultiplier = 1.16f;
		SetColor1(1.0f, 0.38f, 0.38f);
		SetColor2(1.0f, 0.43f, 0.23f);
		SetColor3(1.0f, 1.0f, 1.0f);
		SetColor4(0.98f, 0.735f, 0.0f);
		SetColor5(0.975f, 0.5f, 0.5f);
		SetColor6(0.81f, 0.81f, 0.81f);
		SetEnabled(true);
		break;

	case EffectPreset::LARGE_SCALE:
		// 大きなスケール（粗い）
		parameters_.speedMultiplier = 1.0f;
		parameters_.scale = 3.0f;
		parameters_.speedX = 0.35f;
		parameters_.speedY = 0.35f;
		parameters_.randSeed1X = 1.0f;
		parameters_.randSeed1Y = 5.0f;
		parameters_.randSeed2X = 100.0f;
		parameters_.randSeed2Y = 100.0f;
		parameters_.mixRatio = 1.0f;
		parameters_.brightnessMultiplier = 1.0f;
		SetColor1(0.5f, 0.0f, 0.1f);
		SetColor2(0.9f, 0.3f, 0.1f);
		SetColor3(0.0f, 0.0f, 0.0f);
		SetColor4(1.0f, 0.9f, 0.6f);
		SetColor5(0.0f, 0.0f, 0.0f);
		SetColor6(1.0f, 1.0f, 1.0f);
		SetEnabled(true);
		break;

	case EffectPreset::SMALL_SCALE:
		// 小さなスケール（細かい）
		parameters_.speedMultiplier = 1.0f;
		parameters_.scale = 12.0f;
		parameters_.speedX = 0.35f;
		parameters_.speedY = 0.35f;
		parameters_.randSeed1X = 1.0f;
		parameters_.randSeed1Y = 5.0f;
		parameters_.randSeed2X = 100.0f;
		parameters_.randSeed2Y = 100.0f;
		parameters_.mixRatio = 1.0f;
		parameters_.brightnessMultiplier = 1.0f;
		SetColor1(0.5f, 0.0f, 0.1f);
		SetColor2(0.9f, 0.3f, 0.1f);
		SetColor3(0.0f, 0.0f, 0.0f);
		SetColor4(1.0f, 0.9f, 0.6f);
		SetColor5(0.0f, 0.0f, 0.0f);
		SetColor6(1.0f, 1.0f, 1.0f);
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetSpeedMultiplier(float multiplier) {
	parameters_.speedMultiplier = std::clamp(multiplier, 0.0f, 5.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetScale(float scale) {
	parameters_.scale = std::clamp(scale, 0.5f, 20.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetSpeedX(float speedX) {
	parameters_.speedX = std::clamp(speedX, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetSpeedY(float speedY) {
	parameters_.speedY = std::clamp(speedY, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetMixRatio(float ratio) {
	parameters_.mixRatio = std::clamp(ratio, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetBrightness(float brightness) {
	parameters_.brightnessMultiplier = std::clamp(brightness, 0.1f, 3.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetColor1(float r, float g, float b) {
	parameters_.color1[0] = std::clamp(r, 0.0f, 2.0f);
	parameters_.color1[1] = std::clamp(g, 0.0f, 2.0f);
	parameters_.color1[2] = std::clamp(b, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetColor2(float r, float g, float b) {
	parameters_.color2[0] = std::clamp(r, 0.0f, 2.0f);
	parameters_.color2[1] = std::clamp(g, 0.0f, 2.0f);
	parameters_.color2[2] = std::clamp(b, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetColor3(float r, float g, float b) {
	parameters_.color3[0] = std::clamp(r, 0.0f, 2.0f);
	parameters_.color3[1] = std::clamp(g, 0.0f, 2.0f);
	parameters_.color3[2] = std::clamp(b, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetColor4(float r, float g, float b) {
	parameters_.color4[0] = std::clamp(r, 0.0f, 2.0f);
	parameters_.color4[1] = std::clamp(g, 0.0f, 2.0f);
	parameters_.color4[2] = std::clamp(b, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetColor5(float r, float g, float b) {
	parameters_.color5[0] = std::clamp(r, 0.0f, 2.0f);
	parameters_.color5[1] = std::clamp(g, 0.0f, 2.0f);
	parameters_.color5[2] = std::clamp(b, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetColor6(float r, float g, float b) {
	parameters_.color6[0] = std::clamp(r, 0.0f, 2.0f);
	parameters_.color6[1] = std::clamp(g, 0.0f, 2.0f);
	parameters_.color6[2] = std::clamp(b, 0.0f, 2.0f);
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetRandSeed1(float x, float y) {
	parameters_.randSeed1X = x;
	parameters_.randSeed1Y = y;
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::SetRandSeed2(float x, float y) {
	parameters_.randSeed2X = x;
	parameters_.randSeed2Y = y;
	UpdateParameterBuffer();
}

void LavaMagmaPostEffect::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle (ShaderToy 1:1)");

		if (isEnabled_) {
			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("ShaderToy Original")) ApplyPreset(EffectPreset::SHADERTOY_ORIGINAL);
				ImGui::SameLine();
				if (ImGui::Button("Lava Orange")) ApplyPreset(EffectPreset::LAVA_ORANGE);
				if (ImGui::Button("MY LAVA")) ApplyPreset(EffectPreset::MY_LAVA);

					ImGui::SameLine();

				if (ImGui::Button("Large Scale")) ApplyPreset(EffectPreset::LARGE_SCALE);
				if (ImGui::Button("Small Scale")) ApplyPreset(EffectPreset::SMALL_SCALE);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				// 速度倍率
				float speedMult = parameters_.speedMultiplier;
				if (ImGui::SliderFloat("Speed Multiplier", &speedMult, 0.0f, 3.0f)) {
					SetSpeedMultiplier(speedMult);
				}

				// スケール
				float scale = parameters_.scale;
				if (ImGui::SliderFloat("Scale", &scale, 1.0f, 15.0f)) {
					SetScale(scale);
				}

				// 明るさ
				float brightness = parameters_.brightnessMultiplier;
				if (ImGui::SliderFloat("Brightness", &brightness, 0.5f, 2.0f)) {
					SetBrightness(brightness);
				}

				// 混合比
				float mixRatio = parameters_.mixRatio;
				if (ImGui::SliderFloat("Mix Ratio", &mixRatio, 0.0f, 1.0f)) {
					SetMixRatio(mixRatio);
				}

				ImGui::TreePop();
			}

			// ShaderToyパラメータ（上級者向け）
			if (ImGui::TreeNode("Advanced (ShaderToy Parameters)")) {
				// speedX, speedY
				float speedX = parameters_.speedX;
				if (ImGui::SliderFloat("Speed X", &speedX, 0.0f, 1.0f)) {
					SetSpeedX(speedX);
				}

				float speedY = parameters_.speedY;
				if (ImGui::SliderFloat("Speed Y", &speedY, 0.0f, 1.0f)) {
					SetSpeedY(speedY);
				}

				// rand seed1
				float seed1[2] = { parameters_.randSeed1X, parameters_.randSeed1Y };
				if (ImGui::SliderFloat2("Rand Seed 1", seed1, 0.0f, 10.0f)) {
					SetRandSeed1(seed1[0], seed1[1]);
				}

				// rand seed2
				float seed2[2] = { parameters_.randSeed2X, parameters_.randSeed2Y };
				if (ImGui::SliderFloat2("Rand Seed 2", seed2, 0.0f, 200.0f)) {
					SetRandSeed2(seed2[0], seed2[1]);
				}

				ImGui::TreePop();
			}

			// カラー設定（ShaderToyの6色）
			if (ImGui::TreeNode("Color Settings (6 Colors)")) {
				// Color 1
				float color1[3] = { parameters_.color1[0], parameters_.color1[1], parameters_.color1[2] };
				if (ImGui::ColorEdit3("Color 1 (col1)", color1)) {
					SetColor1(color1[0], color1[1], color1[2]);
				}

				// Color 2
				float color2[3] = { parameters_.color2[0], parameters_.color2[1], parameters_.color2[2] };
				if (ImGui::ColorEdit3("Color 2 (col2)", color2)) {
					SetColor2(color2[0], color2[1], color2[2]);
				}

				// Color 3
				float color3[3] = { parameters_.color3[0], parameters_.color3[1], parameters_.color3[2] };
				if (ImGui::ColorEdit3("Color 3 (col3)", color3)) {
					SetColor3(color3[0], color3[1], color3[2]);
				}

				// Color 4
				float color4[3] = { parameters_.color4[0], parameters_.color4[1], parameters_.color4[2] };
				if (ImGui::ColorEdit3("Color 4 (col4)", color4)) {
					SetColor4(color4[0], color4[1], color4[2]);
				}

				// Color 5
				float color5[3] = { parameters_.color5[0], parameters_.color5[1], parameters_.color5[2] };
				if (ImGui::ColorEdit3("Color 5 (col5)", color5)) {
					SetColor5(color5[0], color5[1], color5[2]);
				}

				// Color 6
				float color6[3] = { parameters_.color6[0], parameters_.color6[1], parameters_.color6[2] };
				if (ImGui::ColorEdit3("Color 6 (col6)", color6)) {
					SetColor6(color6[0], color6[1], color6[2]);
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Time: %.2f", parameters_.time);
			ImGui::Text("Speed Multiplier: %.2f", parameters_.speedMultiplier);
			ImGui::Text("Scale: %.2f", parameters_.scale);
			ImGui::Text("Speed: (%.2f, %.2f)", parameters_.speedX, parameters_.speedY);
		}

		ImGui::TreePop();
	}
#endif
}