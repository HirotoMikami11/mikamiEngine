#pragma once
/// @file Object3DRenderer.h
/// @brief 3Dオブジェクトの描画リクエストを一括管理するレンダラー
/// @details
///   Object3D は Draw(renderer) 内で AllocateTransform/AllocateMaterial でスロットを確保し、
///   CPU データを書き込んでから Submit() で描画リクエストを登録する。
///   実際の GPU コマンド発行は Draw3D() に集約されており、
///   PSO は自身で PSOFactory を通じて生成するため DirectXCommon の PSO に依存しない。
///
///   【フレーム毎の呼び出し順】
///   Engine::StartDrawOffscreen() → BeginFrame()          // リングバッファリセット
///   Scene::Draw() → Object3D::Draw(renderer)
///     → AllocateTransform() / AllocateMaterial()         // スロット確保 & CPU書き込み
///     → Submit(submission)                               // キューに積む
///   Engine::EndDrawOffscreen() → Draw3D()                // GPU コマンド一括発行
///   Engine::EndDrawOffscreen() → PostDraw()              // オフスクリーン終了

#include <vector>
#include "DirectXCommon.h"
#include "PSOFactory.h"
#include "RootSignatureBuilder.h"
#include "RenderSubmission.h"
#include "UploadRingBuffer.h"
#include "Structures.h"
#include "MyFunction.h"  // TransformationMatrix

/// @brief 3Dオブジェクトの描画を一括管理するシングルトンレンダラー
class Object3DRenderer {
public:
    /// @brief 1フレームに描画できる最大 TransformationMatrix スロット数
    static constexpr uint32_t kMaxTransforms = 2048;
    /// @brief 1フレームに描画できる最大 MaterialData スロット数
    static constexpr uint32_t kMaxMaterials  = 2048;

    /// @brief シングルトンインスタンスを取得
    static Object3DRenderer* GetInstance();

    /// @brief 初期化。PSO 生成とリングバッファ確保を行う。
    /// @param dxCommon DirectXCommon のポインタ（デバイス・コマンドリスト・PSOFactory 取得に使用）
    void Initialize(DirectXCommon* dxCommon);

    /// @brief GPU リソースを明示的に解放する。
    /// @note Engine::Finalize() の dxCommon_->Finalize() より前に呼ぶこと
    void Finalize();

    /// @brief フレーム先頭で呼ぶ。リングバッファをリセットし submissions_ をクリアする。
    /// @note Engine::StartDrawOffscreen() から呼ぶこと
    void BeginFrame();

    /// @brief TransformationMatrix 用スロットを1つ確保する。
    /// @return CPU 書き込みポインタと GPU アドレスのペア
    /// @note Object3D::Draw(renderer) 内でのみ呼ぶこと
    UploadRingBuffer<TransformationMatrix>::Allocation AllocateTransform() {
        return transformRingBuffer_.Allocate();
    }

    /// @brief MaterialData 用スロットを1つ確保する。
    /// @return CPU 書き込みポインタと GPU アドレスのペア
    /// @note Object3D::Draw(renderer) 内でのみ呼ぶこと
    UploadRingBuffer<MaterialData>::Allocation AllocateMaterial() {
        return materialRingBuffer_.Allocate();
    }

    /// @brief 描画リクエストをキューに積む。
    /// @param submission Object3D::Draw() で構築した描画データ（Mesh*, GPU アドレス, テクスチャ等）
    void Submit(const ModelSubmission& submission);

    /// @brief 積まれた全リクエストをソートして GPU 描画命令を発行する。
    /// @note Engine::EndDrawOffscreen() の offscreenRenderer_->PostDraw() 前から呼ぶこと
    void Draw3D();

private:
    Object3DRenderer()  = default;
    ~Object3DRenderer() = default;
    Object3DRenderer(const Object3DRenderer&)            = delete;
    Object3DRenderer& operator=(const Object3DRenderer&) = delete;

    /// @brief PSOFactory を使って RootSignature と PSO を生成する
    void InitializePSO();

    // DirectXCommon（コマンドリスト・デバイス・PSOFactory 取得に使用）
    DirectXCommon* dxCommon_ = nullptr;

    // 自前で生成した PSO（RootSignature + PipelineState）
    // DirectXCommon の PSO は使用しない
    PSOFactory::PSOInfo pso_;

    // フレームリセット式 Upload リングバッファ（TransformationMatrix 用）
    UploadRingBuffer<TransformationMatrix> transformRingBuffer_;

    // フレームリセット式 Upload リングバッファ（MaterialData 用）
    UploadRingBuffer<MaterialData>         materialRingBuffer_;

    // 1フレーム分の描画リクエストリスト（BeginFrame でクリア）
    std::vector<ModelSubmission> submissions_;
};
