#pragma once
#include "BaseScene.h"
#include "Dollar1Recognizer.h"
#include <vector>

class MojiTestScene : public BaseScene
{
public:
    MojiTestScene();
    ~MojiTestScene() override;

    void ConfigureOffscreenEffects() override;
    void Initialize() override;
    void Update() override;
    void DrawOffscreen() override;
    void DrawBackBuffer() override;
    void Finalize() override;
    void ImGui() override;

private:
    void InitializeGameObjects();
    void UpdateGameObjects();

    std::unique_ptr<Sphere>    sphere_;
    std::unique_ptr<Model3D>   terrain_;
    CameraController* cameraController_;
    Matrix4x4                  viewProjectionMatrix;
    DirectXCommon* dxCommon_;
    OffscreenRenderer* offscreenRenderer_;

    // 図形認識キャンバスの状態
    std::vector<ImVec2> strokePoints_;
    bool                isDrawing_ = false;
    bool                hasResult_ = false;
    DollarResult        lastResult_;
};