#pragma once
#include <memory>
#include <vector>
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Structures.h"
#include "Object3D.h"

/// <summary>
///　動作確認用に球体を大量に表示するためのクラス
/// </summary>
class DebugSphere {
public:
    DebugSphere();
    ~DebugSphere() = default;

    void Initialize(DirectXCommon* dxCommon);

    // 更新・描画処理
    void Update(const Matrix4x4& viewProj);
    void Draw();
    void ImGui();

    // Sphere のリストを再生成
    void Regenerate();

private:
    void GenerateSpheres();

private:
    DirectXCommon* directXCommon_;

    // スフィア
    std::vector<std::unique_ptr<Sphere>> spheres_;

    // パラメータ
    int sphereCount_ = 10;
    int sphereCountPerRow_ = 10;
    float sphereSpacing_ = 3.0f;
    bool autoRotate_ = true;
};
