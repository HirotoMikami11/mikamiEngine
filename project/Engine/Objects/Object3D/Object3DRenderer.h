#pragma once
#include <vector>
#include "DirectXCommon.h"
#include "GraphicsConfig.h"
#include "PSOFactory.h"
#include "RootSignatureBuilder.h"
#include "RenderSubmission.h"
#include "UploadRingBuffer.h"
#include "Structures.h"
#include "MyFunction.h"

/// <summary>
/// 3Dオブジェクトの描画を一括管理するシングルトンレンダラー
/// </summary>
class Object3DRenderer {
public:
	/// <summary>1フレームに描画できる最大 TransformationMatrix スロット数（GraphicsConfig::kMaxObject3DTransforms に集約）</summary>
	static constexpr uint32_t kMaxTransforms = GraphicsConfig::kMaxObject3DTransforms;
	/// <summary>1フレームに描画できる最大 MaterialData スロット数（GraphicsConfig::kMaxObject3DMaterials に集約）</summary>
	static constexpr uint32_t kMaxMaterials = GraphicsConfig::kMaxObject3DMaterials;

	/// <summary>
	/// シングルトンインスタンスを取得
	/// </summary>
	static Object3DRenderer* GetInstance();

	/// <summary>
	/// 初期化。PSO 生成とリングバッファ確保を行う。
	/// </summary>
	/// <param name="dxCommon">DirectXCommon のポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// GPU リソースを明示的に解放する。
	/// Engine::Finalize() の dxCommon_->Finalize() より前に呼ぶこと。
	/// </summary>
	void Finalize();

	/// <summary>
	/// フレーム先頭で呼ぶ。リングバッファをリセットし submissions_ をクリアする。
	/// Engine::StartDrawOffscreen() から呼ぶこと。
	/// </summary>
	void BeginFrame();

	/// <summary>
	/// TransformationMatrix 用スロットを1つ確保する。
	/// Object3D::Draw() 内でのみ呼ぶこと。
	/// </summary>
	/// <returns>CPU 書き込みポインタと GPU アドレスのペア</returns>
	UploadRingBuffer<TransformationMatrix>::Allocation AllocateTransform() {
		return transformRingBuffer_.Allocate();
	}

	/// <summary>
	/// MaterialData 用スロットを1つ確保する。
	/// Object3D::Draw() 内でのみ呼ぶこと。
	/// </summary>
	/// <returns>CPU 書き込みポインタと GPU アドレスのペア</returns>
	UploadRingBuffer<MaterialData>::Allocation AllocateMaterial() {
		return materialRingBuffer_.Allocate();
	}

	/// <summary>
	/// 描画リクエストをキューに積む。
	/// </summary>
	/// <param name="submission">Object3D::Draw() で構築した描画データ</param>
	void Submit(const ModelSubmission& submission);

	/// <summary>
	/// RenderGroup::UI 以外のオブジェクトを GPU 描画する。
	/// Engine::EndDrawOffscreen() の offscreenRenderer_->PostDraw() より前に呼ぶこと。
	/// </summary>
	void FlushOffscreen();

	/// <summary>
	/// RenderGroup::UI のオブジェクトを GPU 描画する。
	/// Engine::EndDrawBackBuffer() から呼ぶこと。
	/// </summary>
	void FlushUI();

#ifdef USEIMGUI
	/// <summary>
	/// スロット使用量とグループ別描画数をゲージ表示する。Engine::ImGui() から呼ぶこと。
	/// </summary>
	void ImGui();
#endif

private:
	Object3DRenderer() = default;
	~Object3DRenderer() = default;
	Object3DRenderer(const Object3DRenderer&) = delete;
	Object3DRenderer& operator=(const Object3DRenderer&) = delete;

	/// <summary>
	/// PSOFactory を使って RootSignature と 4 種類の PSO を生成する
	/// </summary>
	void InitializePSO();

	/// <summary>
	/// uiOnly フラグに応じてフィルタしながら GPU 描画命令を発行する
	/// </summary>
	/// <param name="uiOnly">true → RenderGroup::UI のみ、false → UI 以外のみ</param>
	void Flush(bool uiOnly);

	// DirectXCommon（コマンドリスト・デバイス・PSOFactory 取得に使用）
	DirectXCommon* dxCommon_ = nullptr;

	// Opaque PSO（RootSignature を所有。他の PSO はこれを共有する）
	PSOFactory::PSOInfo psoOpaque_;

	// AlphaBlend PSO（RootSignature は psoOpaque_ から共有）
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoAlphaBlend_;

	// Add PSO（RootSignature は psoOpaque_ から共有）
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoAdd_;

	// Wireframe PSO（RootSignature は psoOpaque_ から共有）
	Microsoft::WRL::ComPtr<ID3D12PipelineState> psoWireframe_;

	// フレームリセット式 Upload リングバッファ（TransformationMatrix 用）
	UploadRingBuffer<TransformationMatrix> transformRingBuffer_;

	// フレームリセット式 Upload リングバッファ（MaterialData 用）
	UploadRingBuffer<MaterialData> materialRingBuffer_;

	// 1フレーム分の描画リクエストリスト（BeginFrame でクリア）
	std::vector<ModelSubmission> submissions_;
};

/// Object3D は Draw() 内で AllocateTransform/AllocateMaterial でスロットを確保
/// CPU データを書き込んでから Submit() で描画リクエストを登録する
/// 
/// 【フレーム毎の呼び出し順】
/// Engine::StartDrawOffscreen() → BeginFrame()			// リングバッファリセット
/// Scene::Draw() → Object3D::Draw()
///	→ AllocateTransform() / AllocateMaterial()			// スロット確保 & CPU書き込み
///	→ Submit(submission)								// キューに積む
/// Engine::EndDrawOffscreen() → FlushOffscreen()		// UI以外のオブジェクトを GPU 描画
/// Engine::EndDrawBackBuffer() → FlushUI()				// UIオブジェクトを GPU 描画
