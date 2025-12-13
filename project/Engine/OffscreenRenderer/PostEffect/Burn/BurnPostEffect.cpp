#include "BurnPostEffect.h"
#include "ImGui/ImGuiManager.h" 

void BurnPostEffect::Initialize(DirectXCommon* dxCommon) {
	dxCommon_ = dxCommon;

	// PSOを作成
	CreatePSO();

	// パラメータバッファを作成
	CreateParameterBuffer();

	isInitialized_ = true;

	Logger::Log(Logger::GetStream(), "BurnPostEffect initialized successfully (OffscreenTriangle version)!\n");
}

void BurnPostEffect::Finalize() {
	// パラメータデータのマッピング解除
	if (mappedParameters_) {
		parameterBuffer_->Unmap(0, nullptr);
		mappedParameters_ = nullptr;
	}

	isInitialized_ = false;
	Logger::Log(Logger::GetStream(), "BurnPostEffect finalized (OffscreenTriangle version).\n");
}

void BurnPostEffect::Update(float deltaTime) {
	if (!isEnabled_ || !isInitialized_) {
		return;
	}

	// 時間を更新
	parameters_.time += deltaTime;

	// 自動再生中の場合、progressを増加
	if (isAutoPlaying_) {
		parameters_.progress += deltaTime * parameters_.burnSpeed;
		if (parameters_.progress >= 1.0f) {
			parameters_.progress = 1.0f;
			isAutoPlaying_ = false;	// 完全に燃え尽きたら停止
		}
	}

	// パラメータバッファを更新
	UpdateParameterBuffer();
}

void BurnPostEffect::Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) {
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

void BurnPostEffect::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddCBV(0, D3D12_SHADER_VISIBILITY_PIXEL)		// BurnParameters (b0)
		.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/Burn/Burn.PS.hlsl");

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "BurnPostEffect: Failed to create PSO\n");
		assert(false);
	}

	rootSignature_ = psoInfo.rootSignature;
	pipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create Burn PSO (PSOFactory version)!!\n");
}

void BurnPostEffect::CreateParameterBuffer() {
	// 構造体サイズをデバッグ出力
	size_t structSize = sizeof(BurnParameters);
	Logger::Log(Logger::GetStream(), std::format("BurnParameters size: {} bytes\n", structSize));

	// 256バイト境界に揃える（DirectX12の要件）
	size_t alignedSize = (structSize + 255) & ~255;
	Logger::Log(Logger::GetStream(), std::format("Aligned buffer size: {} bytes\n", alignedSize));

	// パラメータバッファ作成
	parameterBuffer_ = CreateBufferResource(dxCommon_->GetDeviceComPtr(), alignedSize);
	parameterBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedParameters_));

	// 初期データを設定
	UpdateParameterBuffer();

	Logger::Log(Logger::GetStream(), "Complete create Burn parameter buffer (OffscreenTriangle version)!!\n");
}

Microsoft::WRL::ComPtr<IDxcBlob> BurnPostEffect::CompileShader(
	const std::wstring& filePath, const wchar_t* profile) {

	//DirectXCommonのCompileShader関数を使用
	return DirectXCommon::CompileShader(
		filePath,
		profile,
		dxCommon_->GetDxcUtils(),
		dxCommon_->GetDxcCompiler(),
		dxCommon_->GetIncludeHandler());
}

void BurnPostEffect::UpdateParameterBuffer() {
	if (mappedParameters_) {
		*mappedParameters_ = parameters_;
	}
}

void BurnPostEffect::ApplyPreset(EffectPreset preset) {
	switch (preset) {
	case EffectPreset::OFF:
		SetEnabled(false);
		Reset();
		break;

	case EffectPreset::SLOW_BURN:
		parameters_.burnSpeed = 0.2f;
		parameters_.edgeWidth = 0.08f;
		parameters_.noiseScale = 5.0f;
		parameters_.noiseStrength = 0.2f;
		SetEnabled(true);
		break;

	case EffectPreset::NORMAL_BURN:
		parameters_.burnSpeed = 0.5f;
		parameters_.edgeWidth = 0.05f;
		parameters_.noiseScale = 5.0f;
		parameters_.noiseStrength = 0.2f;
		SetEnabled(true);
		break;

	case EffectPreset::FAST_BURN:
		parameters_.burnSpeed = 1.0f;
		parameters_.edgeWidth = 0.03f;
		parameters_.noiseScale = 6.0f;
		parameters_.noiseStrength = 0.25f;
		SetEnabled(true);
		break;

	case EffectPreset::INTENSE_BURN:
		parameters_.burnSpeed = 0.7f;
		parameters_.edgeWidth = 0.06f;
		parameters_.noiseScale = 8.0f;
		parameters_.noiseStrength = 0.4f;
		SetEnabled(true);
		break;

	case EffectPreset::GENTLE_BURN:
		parameters_.burnSpeed = 0.3f;
		parameters_.edgeWidth = 0.1f;
		parameters_.noiseScale = 3.0f;
		parameters_.noiseStrength = 0.1f;
		SetEnabled(true);
		break;
	}

	UpdateParameterBuffer();
}

void BurnPostEffect::SetProgress(float progress) {
	parameters_.progress = std::clamp(progress, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void BurnPostEffect::SetBurnColor(const Vector4& color) {
	parameters_.burnColor = color;
	UpdateParameterBuffer();
}

void BurnPostEffect::SetEdgeColor(const Vector4& color) {
	parameters_.edgeColor = color;
	UpdateParameterBuffer();
}

void BurnPostEffect::SetEdgeWidth(float width) {
	parameters_.edgeWidth = std::clamp(width, 0.001f, 0.3f);
	UpdateParameterBuffer();
}

void BurnPostEffect::SetNoiseScale(float scale) {
	parameters_.noiseScale = std::clamp(scale, 0.1f, 20.0f);
	UpdateParameterBuffer();
}

void BurnPostEffect::SetNoiseStrength(float strength) {
	parameters_.noiseStrength = std::clamp(strength, 0.0f, 1.0f);
	UpdateParameterBuffer();
}

void BurnPostEffect::SetBurnCenter(const Vector2& center) {
	parameters_.burnCenter = center;
	UpdateParameterBuffer();
}

void BurnPostEffect::SetBurnSpeed(float speed) {
	parameters_.burnSpeed = std::clamp(speed, 0.1f, 5.0f);
	UpdateParameterBuffer();
}

void BurnPostEffect::Reset() {
	parameters_.progress = 0.0f;
	parameters_.time = 0.0f;
	isAutoPlaying_ = false;
	UpdateParameterBuffer();
}

void BurnPostEffect::StartBurn() {
	isAutoPlaying_ = true;
	if (parameters_.progress >= 1.0f) {
		Reset();
	}
}

void BurnPostEffect::StopBurn() {
	isAutoPlaying_ = false;
}

void BurnPostEffect::ImGui() {
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {
		// エフェクトの状態表示
		ImGui::Text("Effect Status: %s", isEnabled_ ? "ENABLED" : "DISABLED");
		ImGui::Text("Initialized: %s", isInitialized_ ? "YES" : "NO");
		ImGui::Text("Auto Playing: %s", isAutoPlaying_ ? "YES" : "NO");
		ImGui::Text("Render Method: OffscreenTriangle");

		if (isEnabled_) {
			// コントロールボタン
			if (ImGui::TreeNode("Playback Controls")) {
				if (ImGui::Button("Start Burn")) {
					StartBurn();
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop")) {
					StopBurn();
				}
				ImGui::SameLine();
				if (ImGui::Button("Reset")) {
					Reset();
				}

				ImGui::TreePop();
			}

			// プリセット選択
			if (ImGui::TreeNode("Presets")) {
				if (ImGui::Button("Slow Burn")) ApplyPreset(EffectPreset::SLOW_BURN);
				ImGui::SameLine();
				if (ImGui::Button("Normal Burn")) ApplyPreset(EffectPreset::NORMAL_BURN);
				ImGui::SameLine();
				if (ImGui::Button("Fast Burn")) ApplyPreset(EffectPreset::FAST_BURN);

				if (ImGui::Button("Intense Burn")) ApplyPreset(EffectPreset::INTENSE_BURN);
				ImGui::SameLine();
				if (ImGui::Button("Gentle Burn")) ApplyPreset(EffectPreset::GENTLE_BURN);

				ImGui::TreePop();
			}

			// 個別パラメータ調整
			if (ImGui::TreeNode("Manual Settings")) {
				float progress = parameters_.progress;
				if (ImGui::SliderFloat("Progress", &progress, 0.0f, 1.0f)) {
					SetProgress(progress);
					isAutoPlaying_ = false;	// 手動で変更した場合は自動再生を停止
				}

				float burnSpeed = parameters_.burnSpeed;
				if (ImGui::SliderFloat("Burn Speed", &burnSpeed, 0.1f, 5.0f)) {
					SetBurnSpeed(burnSpeed);
				}

				Vector4 burnColor = parameters_.burnColor;
				if (ImGui::ColorEdit4("Burn Color", reinterpret_cast<float*>(&burnColor.x))) {
					SetBurnColor(burnColor);
				}

				Vector4 edgeColor = parameters_.edgeColor;
				if (ImGui::ColorEdit4("Edge Color", reinterpret_cast<float*>(&edgeColor.x))) {
					SetEdgeColor(edgeColor);
				}

				float edgeWidth = parameters_.edgeWidth;
				if (ImGui::SliderFloat("Edge Width", &edgeWidth, 0.001f, 0.3f)) {
					SetEdgeWidth(edgeWidth);
				}

				float noiseScale = parameters_.noiseScale;
				if (ImGui::SliderFloat("Noise Scale", &noiseScale, 0.1f, 20.0f)) {
					SetNoiseScale(noiseScale);
				}

				float noiseStrength = parameters_.noiseStrength;
				if (ImGui::SliderFloat("Noise Strength", &noiseStrength, 0.0f, 1.0f)) {
					SetNoiseStrength(noiseStrength);
				}

				Vector2 burnCenter = parameters_.burnCenter;
				if (ImGui::SliderFloat2("Burn Center", reinterpret_cast<float*>(&burnCenter.x), 0.0f, 1.0f)) {
					SetBurnCenter(burnCenter);
				}

				ImGui::TreePop();
			}

			// 情報表示
			ImGui::Separator();
			ImGui::Text("Current Progress: %.2f", parameters_.progress);
			ImGui::Text("Current Time: %.2f", parameters_.time);
			ImGui::Text("Burn Speed: %.2f", parameters_.burnSpeed);
			ImGui::Text("Edge Width: %.3f", parameters_.edgeWidth);
			ImGui::Text("Noise Scale: %.2f", parameters_.noiseScale);
		}

		ImGui::TreePop();
	}
#endif
}