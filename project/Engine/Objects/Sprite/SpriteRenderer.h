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
/// スプライトの描画を一括管理するシングルトンレンダラー
/// </summary>
class SpriteRenderer {
public:
	/// <summary>1フレームに描画できる最大スプライト枚数（GraphicsConfig::kMaxSprites に集約）</summary>
	static constexpr uint32_t kMaxSprites = GraphicsConfig::kMaxSprites;

	/// <summary>
	/// シングルトンインスタンスを取得
	/// </summary>
	static SpriteRenderer* GetInstance();

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
	/// Sprite::Draw() 内でのみ呼ぶこと。
	/// </summary>
	UploadRingBuffer<TransformationMatrix>::Allocation AllocateTransform() {
		return transformRingBuffer_.Allocate();
	}

	/// <summary>
	/// SpriteMaterialData 用スロットを1つ確保する。
	/// Sprite::Draw() 内でのみ呼ぶこと。
	/// </summary>
	UploadRingBuffer<SpriteMaterialData>::Allocation AllocateMaterial() {
		return materialRingBuffer_.Allocate();
	}

	/// <summary>
	/// 描画リクエストをキューに積む。
	/// </summary>
	/// <param name="submission">Sprite::Draw() で構築した描画データ</param>
	void Submit(const SpriteSubmission& submission);

#ifdef USEIMGUI
	/// <summary>
	/// スロット使用量をゲージ表示する。Engine::ImGui() から呼ぶこと。
	/// </summary>
	void ImGui();
#endif

	/// <summary>
	/// RenderGroup::UI 以外のスプライトを GPU 描画する。
	/// Engine::EndDrawOffscreen() の PostDraw() より前に呼ぶこと。
	/// </summary>
	void FlushOffscreen();

	/// <summary>
	/// RenderGroup::UI のスプライトを GPU 描画する。
	/// Engine::EndDrawBackBuffer() の ImGui より前に呼ぶこと。
	/// </summary>
	void FlushUI();

private:
	SpriteRenderer() = default;
	~SpriteRenderer() = default;
	SpriteRenderer(const SpriteRenderer&) = delete;
	SpriteRenderer& operator=(const SpriteRenderer&) = delete;

	/// <summary>
	/// PSOFactory を使って RootSignature と PSO を生成する
	/// </summary>
	void InitializePSO();

	/// <summary>
	/// uiOnly フラグに応じてフィルタしながら GPU 描画命令を発行する
	/// </summary>
	/// <param name="uiOnly">true → RenderGroup::UI のみ、false → UI 以外のみ</param>
	void Flush(bool uiOnly);

	// DirectXCommon（コマンドリスト・デバイス・PSOFactory 取得に使用）
	DirectXCommon* dxCommon_ = nullptr;

	// 自前で生成した PSO（RootSignature + PipelineState）
	PSOFactory::PSOInfo pso_;

	// フレームリセット式 Upload リングバッファ（TransformationMatrix 用）
	UploadRingBuffer<TransformationMatrix> transformRingBuffer_;

	// フレームリセット式 Upload リングバッファ（SpriteMaterialData 用）
	UploadRingBuffer<SpriteMaterialData> materialRingBuffer_;

	// 1フレーム分の描画リクエストリスト（BeginFrame でクリア）
	std::vector<SpriteSubmission> submissions_;
};

/// Sprite は Draw() 内で AllocateTransform/AllocateMaterial でスロットを確保
/// CPU データを書き込んでから Submit() で描画リクエストを登録する
/// 【フレーム毎の呼び出し順】
/// Engine::StartDrawOffscreen() → BeginFrame()		// リングバッファリセット
/// Scene::Draw() → Sprite::Draw()
/// → AllocateTransform() / AllocateMaterial()		// スロット確保 & CPU書き込み
/// → Submit(submission)							// キューに積む
/// Engine::EndDrawOffscreen() → FlushOffscreen()	// UI以外のスプライトを GPU 描画
/// Engine::EndDrawBackBuffer() → FlushUI()			// UI スプライトを GPU 描画
