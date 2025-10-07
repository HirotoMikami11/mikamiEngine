#include "OffscreenRenderer.h"
#include "Managers/ImGui/ImGuiManager.h" 

void OffscreenRenderer::Initialize(DirectXCommon* dxCommon, uint32_t width, uint32_t height) {
	dxCommon_ = dxCommon;
	width_ = width;
	height_ = height;

	// リソース作成
	CreateRenderTargetTexture();
	CreateDepthStencilTexture();
	CreateRTV();
	CreateDSV();
	CreateSRV();
	CreateDepthSRV();  // 深度用SRV作成を追加
	CreatePSO();

	// ビューポート設定
	viewport_.Width = static_cast<float>(width_);
	viewport_.Height = static_cast<float>(height_);
	viewport_.TopLeftX = 0.0f;
	viewport_.TopLeftY = 0.0f;
	viewport_.MinDepth = 0.0f;
	viewport_.MaxDepth = 1.0f;

	// シザー矩形設定
	scissorRect_.left = 0;
	scissorRect_.right = width_;
	scissorRect_.top = 0;
	scissorRect_.bottom = height_;

	// オフスクリーン描画用OffscreenTriangleを初期化（Sprite置き換え）
	InitializeOffscreenTriangle();

	// ポストプロセスチェーンを初期化
	postProcessChain_ = std::make_unique<PostProcessChain>();
	postProcessChain_->Initialize(dxCommon_, width_, height_);

	///
	///ここにエフェクトを追加していく
	///

	// 深度フォグエフェクトを追加
	depthFogEffect_ = postProcessChain_->AddEffect<DepthFogPostEffect>();
	// 深度ぼかしエフェクトを追加
	depthOfFieldEffect_ = postProcessChain_->AddEffect<DepthOfFieldPostEffect>();


	// グリッチエフェクトを追加
	RGBShiftEffect_ = postProcessChain_->AddEffect<RGBShiftPostEffect>();
	// ライングリッチエフェクトを追加
	lineGlitchEffect_ = postProcessChain_->AddEffect<LineGlitchPostEffect>();
	// グレースケールエフェクトを追加
	grayscaleEffect_ = postProcessChain_->AddEffect<GrayscalePostEffect>();
	// ビネットエフェクトを追加
	vignetteEffect_ = postProcessChain_->AddEffect<VignettePostEffect>();
	// ダメージエフェクトを追加
	damageEffect_ = postProcessChain_->AddEffect<VignettePostEffect>();



	// 初期化完了のログを出す
	Logger::Log(Logger::GetStream(), "Complete OffscreenRenderer initialized (PostProcess Chain with OffscreenTriangle)!!\n");
}

void OffscreenRenderer::Finalize() {
	// ポストプロセスチェーンの終了処理
	if (postProcessChain_) {
		postProcessChain_->Finalize();
		postProcessChain_.reset();
	}

	///
	///ここで追加したエフェクトをnullptrにしておく
	///



	grayscaleEffect_ = nullptr;
	depthFogEffect_ = nullptr;
	RGBShiftEffect_ = nullptr;
	vignetteEffect_ = nullptr;
	damageEffect_ = nullptr;
	lineGlitchEffect_ = nullptr;
	depthOfFieldEffect_ = nullptr;

	// オフスクリーンOffscreenTriangle削除（Sprite置き換え）
	if (offscreenTriangle_) {
		offscreenTriangle_->Finalize();
		offscreenTriangle_.reset();
	}

	// DescriptorHeapManagerからディスクリプタを解放
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (descriptorManager) {
		if (rtvHandle_.isValid) {
			descriptorManager->ReleaseRTV(rtvHandle_.index);
		}
		if (dsvHandle_.isValid) {
			descriptorManager->ReleaseDSV(dsvHandle_.index);
		}
		if (srvHandle_.isValid) {
			descriptorManager->ReleaseSRV(srvHandle_.index);
		}
		if (depthSrvHandle_.isValid) {  // 深度SRVも解放
			descriptorManager->ReleaseSRV(depthSrvHandle_.index);
		}
	}

	Logger::Log(Logger::GetStream(), "OffscreenRenderer finalized (PostProcess Chain with OffscreenTriangle).\n");
}

void OffscreenRenderer::Update(float deltaTime) {
	// ポストプロセスチェーンの更新
	if (postProcessChain_) {
		postProcessChain_->Update(deltaTime);
	}
}

void OffscreenRenderer::PreDraw() {
	auto commandList = dxCommon_->GetCommandList();

	// カラーバリア構造体を毎回新しく作成（メンバ変数の使い回しを避ける）
	D3D12_RESOURCE_BARRIER preDrawBarrier{};
	preDrawBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	preDrawBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	preDrawBarrier.Transition.pResource = renderTargetTexture_.Get();
	preDrawBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	preDrawBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	preDrawBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// 深度テクスチャは既にDEPTH_WRITE状態で作成されているので、最初のフレームではバリア不要
	// 2フレーム目以降のためにバリア設定
	D3D12_RESOURCE_BARRIER depthPreDrawBarrier{};
	depthPreDrawBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthPreDrawBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	depthPreDrawBarrier.Transition.pResource = depthStencilTexture_.Get();
	depthPreDrawBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	depthPreDrawBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	depthPreDrawBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// 初回フレームはバリア不要、2フレーム目以降はバリア実行
	static bool isFirstFrame = true;
	if (isFirstFrame) {
		commandList->ResourceBarrier(1, &preDrawBarrier);  // カラーのみ
		isFirstFrame = false;
	} else {
		D3D12_RESOURCE_BARRIER barriers[] = { preDrawBarrier, depthPreDrawBarrier };
		commandList->ResourceBarrier(2, barriers);
	}

	// レンダーターゲットとデプスステンシルを設定
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvHandle_.cpuHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvHandle_.cpuHandle;
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	// クリア
	commandList->ClearRenderTargetView(rtvHandle, clearColor_, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// ビューポートとシザー矩形を設定
	commandList->RSSetViewports(1, &viewport_);
	commandList->RSSetScissorRects(1, &scissorRect_);

	// 描画用のデスクリプタヒープを設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	// 通常の描画設定を適用（既存のPSOを使用）
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void OffscreenRenderer::PostDraw() {
	auto commandList = dxCommon_->GetCommandList();

	// カラーバリア構造体を毎回新しく作成（メンバ変数の使い回しを避ける）
	D3D12_RESOURCE_BARRIER postDrawBarrier{};
	postDrawBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	postDrawBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	postDrawBarrier.Transition.pResource = renderTargetTexture_.Get();
	postDrawBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	postDrawBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	postDrawBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// 深度バリア構造体（シェーダーリソース用に遷移）
	D3D12_RESOURCE_BARRIER depthPostDrawBarrier{};
	depthPostDrawBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	depthPostDrawBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	depthPostDrawBarrier.Transition.pResource = depthStencilTexture_.Get();
	depthPostDrawBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
	depthPostDrawBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	depthPostDrawBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// バリアを同時実行
	D3D12_RESOURCE_BARRIER barriers[] = { postDrawBarrier, depthPostDrawBarrier };
	commandList->ResourceBarrier(2, barriers);
}



void OffscreenRenderer::DisableAllEffects() {
	SetAllEffectsEnabled(false);
}

void OffscreenRenderer::EnableAllEffects() {
	SetAllEffectsEnabled(true);
}

void OffscreenRenderer::SetAllEffectsEnabled(bool enabled) {
	//ポストプロセスチェーンのエフェクトをまとめて有効/無効にする
	postProcessChain_->SetAllEffectsEnabled(enabled);
}


void OffscreenRenderer::DrawOffscreenTexture() {
	if (!offscreenTriangle_) {
		return;
	}

	auto commandList = dxCommon_->GetCommandList();

	// ポストプロセスチェーンを適用（深度テクスチャも渡す）
	D3D12_GPU_DESCRIPTOR_HANDLE finalTexture = srvHandle_.gpuHandle;
	if (postProcessChain_) {
		// 深度テクスチャ対応版のApplyEffectsを使用
		finalTexture = postProcessChain_->ApplyEffectsWithDepth(srvHandle_.gpuHandle, depthSrvHandle_.gpuHandle);
	}

	// PostProcessChain実行後に描画状態を完全にリセット
	// バックバッファのレンダーターゲットを再設定
	UINT backBufferIndex = dxCommon_->GetSwapChain()->GetCurrentBackBufferIndex();
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = dxCommon_->GetRTVHandle(backBufferIndex);
	/// オフスクリーンの外にも3DObjectが描画できるようにレンダーターゲットに深度も入れておく
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxCommon_->GetDescriptorManager()->GetCPUHandle(
		DescriptorHeapManager::HeapType::DSV, GraphicsConfig::kMainDSVIndex);
	commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	// ディスクリプタヒープを再設定
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = {
		dxCommon_->GetDescriptorManager()->GetSRVHeapComPtr()
	};
	commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());

	// 最終結果を描画（OffscreenTriangle使用）
	offscreenTriangle_->DrawWithCustomPSO(
		offscreenRootSignature_.Get(),
		offscreenPipelineState_.Get(),
		finalTexture
	);

	// オフスクリーン用PSOから通常描画用PSOに戻す
	commandList->SetGraphicsRootSignature(dxCommon_->GetRootSignature());
	commandList->SetPipelineState(dxCommon_->GetPipelineState());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void OffscreenRenderer::InitializeOffscreenTriangle() {
	// オフスクリーン描画用OffscreenTriangleを作成（Sprite置き換え）
	offscreenTriangle_ = std::make_unique<OffscreenTriangle>();
	offscreenTriangle_->Initialize(dxCommon_);

	Logger::Log(Logger::GetStream(), "Complete initialize offscreen triangle (replacing sprite)!!\n");
}

void OffscreenRenderer::CreateRenderTargetTexture() {
	// リソース設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(width_);
	resourceDesc.Height = UINT(height_);
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	// ヒープ設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	// オフスクリーンレンダリング用のクリアカラーを設定
	D3D12_CLEAR_VALUE clearValue{};
	clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	clearValue.Color[0] = clearColor_[0];
	clearValue.Color[1] = clearColor_[1];
	clearValue.Color[2] = clearColor_[2];
	clearValue.Color[3] = clearColor_[3];

	// 初期状態をPIXEL_SHADER_RESOURCEに変更（バリアとの整合性のため）
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		&clearValue,
		IID_PPV_ARGS(&renderTargetTexture_));

	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create offscreen render target texture (PIXEL_SHADER_RESOURCE initial state)!!\n");
}

void OffscreenRenderer::CreateDepthStencilTexture() {
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width_;
	resourceDesc.Height = height_;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;  // Typelessフォーマットで作成
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // DSV用フォーマット

	// 初期状態をDEPTH_WRITEに変更（書き込み準備状態で作成）
	HRESULT hr = dxCommon_->GetDevice()->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,  // 初期状態を書き込み用に変更
		&depthClearValue,
		IID_PPV_ARGS(&depthStencilTexture_));

	assert(SUCCEEDED(hr));
	Logger::Log(Logger::GetStream(), "Complete create offscreen depth stencil texture (Typeless format)!!\n");
}

void OffscreenRenderer::CreateRTV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// RTVを割り当て
	rtvHandle_ = descriptorManager->AllocateRTV();
	if (!rtvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate RTV for offscreen renderer\n");
		return;
	}

	// RTV作成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	dxCommon_->GetDevice()->CreateRenderTargetView(
		renderTargetTexture_.Get(),
		&rtvDesc,
		rtvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen RTV (Index: {})!!\n", rtvHandle_.index));
}

void OffscreenRenderer::CreateDSV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// DSVを割り当て
	dsvHandle_ = descriptorManager->AllocateDSV();
	if (!dsvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate DSV for offscreen renderer\n");
		return;
	}

	// DSV作成
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // DSV用フォーマット
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;

	dxCommon_->GetDevice()->CreateDepthStencilView(
		depthStencilTexture_.Get(),
		&dsvDesc,
		dsvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen DSV (Index: {})!!\n", dsvHandle_.index));
}

void OffscreenRenderer::CreateSRV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// SRVを割り当て
	srvHandle_ = descriptorManager->AllocateSRV();
	if (!srvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate SRV for offscreen renderer\n");
		return;
	}

	// SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	dxCommon_->GetDevice()->CreateShaderResourceView(
		renderTargetTexture_.Get(),
		&srvDesc,
		srvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen SRV (Index: {})!!\n", srvHandle_.index));
}

void OffscreenRenderer::CreateDepthSRV() {
	auto descriptorManager = dxCommon_->GetDescriptorManager();
	if (!descriptorManager) {
		Logger::Log(Logger::GetStream(), "DescriptorManager is null\n");
		return;
	}

	// 深度用SRVを割り当て
	depthSrvHandle_ = descriptorManager->AllocateSRV();
	if (!depthSrvHandle_.isValid) {
		Logger::Log(Logger::GetStream(), "Failed to allocate depth SRV for offscreen renderer\n");
		return;
	}

	// 深度用SRV作成
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;  // 深度読み取り用フォーマット
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;

	dxCommon_->GetDevice()->CreateShaderResourceView(
		depthStencilTexture_.Get(),
		&srvDesc,
		depthSrvHandle_.cpuHandle);

	Logger::Log(Logger::GetStream(), std::format("Complete create offscreen depth SRV (Index: {})!!\n", depthSrvHandle_.index));
}
void OffscreenRenderer::CreatePSO() {
	// RootSignatureを構築
	RootSignatureBuilder rsBuilder;
	rsBuilder.AddSRV(0, 1, D3D12_SHADER_VISIBILITY_PIXEL)		// Texture (t0)
		.AddStaticSampler(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	// PSO設定を構築
	auto psoDesc = PSODescriptor::CreatePostEffectColorOnly()
		.SetPixelShader(L"resources/Shader/FullscreenTriangle/FullscreenTriangle.PS.hlsl")
		.SetBlendMode(BlendMode::AlphaBlend);// アルファブレンドを有効化

	// PSOFactory経由で生成
	auto psoInfo = dxCommon_->GetPSOFactory()->CreatePSO(psoDesc, rsBuilder);
	if (!psoInfo.IsValid()) {
		Logger::Log(Logger::GetStream(), "OffscreenRenderer: Failed to create PSO\n");
		assert(false);
	}

	offscreenRootSignature_ = psoInfo.rootSignature;
	offscreenPipelineState_ = psoInfo.pipelineState;

	Logger::Log(Logger::GetStream(), "Complete create offscreen PipelineState (PSOFactory version)!!\n");
}

void OffscreenRenderer::ImGui() {
#ifdef USEIMGUI
	if (ImGui::CollapsingHeader("Offscreen Renderer ")) {

		// ポストプロセスチェーンのImGui
		if (postProcessChain_) {
			postProcessChain_->ImGui();
		}
	}
#endif
}