#pragma once
/// 3Dオブジェクトの描画リクエストを一括管理するレンダラー
///
/// Object3D は Draw() 内で AllocateTransform/AllocateMaterial でスロットを確保し、
/// CPU データを書き込んでから Submit() で描画リクエストを登録する。
/// 実際の GPU コマンド発行は Draw3D() に集約されており、
/// PSO は自身で PSOFactory を通じて生成するため DirectXCommon の PSO に依存しない。
///
/// 【フレーム毎の呼び出し順】
/// Engine::StartDrawOffscreen() → BeginFrame()          // リングバッファリセット
/// Scene::Draw() → Object3D::Draw()
///   → AllocateTransform() / AllocateMaterial()         // スロット確保 & CPU書き込み
///   → Submit(submission)                               // キューに積む
/// Engine::EndDrawOffscreen() → Draw3D()                // GPU コマンド一括発行

#include <vector>
#include "DirectXCommon.h"
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
	/// <summary>1フレームに描画できる最大 TransformationMatrix スロット数</summary>
	static constexpr uint32_t kMaxTransforms = 2048;
	/// <summary>1フレームに描画できる最大 MaterialData スロット数</summary>
	static constexpr uint32_t kMaxMaterials = 2048;

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
	/// 積まれた全リクエストをソートして GPU 描画命令を発行する。
	/// Engine::EndDrawOffscreen() の offscreenRenderer_->PostDraw() より前に呼ぶこと。
	/// </summary>
	void Draw3D();

private:
	Object3DRenderer() = default;
	~Object3DRenderer() = default;
	Object3DRenderer(const Object3DRenderer&) = delete;
	Object3DRenderer& operator=(const Object3DRenderer&) = delete;

	/// <summary>
	/// PSOFactory を使って RootSignature と PSO を生成する
	/// </summary>
	void InitializePSO();

	// DirectXCommon（コマンドリスト・デバイス・PSOFactory 取得に使用）
	DirectXCommon* dxCommon_ = nullptr;

	// 自前で生成した PSO（RootSignature + PipelineState）
	// DirectXCommon の PSO は使用しない
	PSOFactory::PSOInfo pso_;

	// フレームリセット式 Upload リングバッファ（TransformationMatrix 用）
	UploadRingBuffer<TransformationMatrix> transformRingBuffer_;

	// フレームリセット式 Upload リングバッファ（MaterialData 用）
	UploadRingBuffer<MaterialData> materialRingBuffer_;

	// 1フレーム分の描画リクエストリスト（BeginFrame でクリア）
	std::vector<ModelSubmission> submissions_;
};
