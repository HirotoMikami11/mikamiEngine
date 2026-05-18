#pragma once
#include <wrl.h>
#include <d3d12.h>

#include "DirectXCommon.h"
#include "PSOFactory.h"
#include "RootSignatureBuilder.h"
#include "MyMath.h"
#include "Structures.h"

/// <summary>
/// Skybox専用の頂点構造体（位置のみ）
/// </summary>
struct SkyboxVertex {
	Vector4 position;
};

/// <summary>
/// SkyboxのWVP定数バッファ（GPU送信用）
/// </summary>
struct SkyboxTransformData {
	Matrix4x4 WVP;
};

/// <summary>
/// Skyboxマテリアルデータ（GPU送信用）
/// </summary>
struct SkyboxMaterialData {
	Vector4 color;
};

/// <summary>
/// Skybox描画を担当するシングルトンレンダラー
/// TextureManagerを経由してDDSキューブマップを管理する
/// </summary>
class SkyboxRenderer {
public:
	static SkyboxRenderer* GetInstance();

	/// <summary>
	/// 初期化（PSO・頂点バッファ・定数バッファ生成）
	/// Engine::InitializeManagers() 内で Object3DRenderer の後に呼ぶこと
	/// </summary>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// フレーム先頭のリセット（現在は何もしないが将来の拡張用）
	/// </summary>
	void BeginFrame() {}

	/// <summary>
	/// Skyboxを描画する
	/// Skybox オブジェクト経由で呼ぶこと（Scene::Draw() 内、BeginOffscreen〜EndOffscreen の間）
	/// cubemapHandle が無効(ptr==0)の場合は何もしない
	/// </summary>
	void Draw(D3D12_GPU_DESCRIPTOR_HANDLE cubemapHandle, const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f });

	/// <summary>
	/// GPUリソースを解放する
	/// Engine::Finalize() の dxCommon_->Finalize() より前に呼ぶこと
	/// </summary>
	void Finalize();


private:
	SkyboxRenderer() = default;
	~SkyboxRenderer() = default;
	SkyboxRenderer(const SkyboxRenderer&) = delete;
	SkyboxRenderer& operator=(const SkyboxRenderer&) = delete;

	void InitializePSO();
	void CreateBoxMesh();
	void CreateConstantBuffers();

	DirectXCommon* dxCommon_ = nullptr;

	// PSO（RootSignatureを所有）
	PSOFactory::PSOInfo pso_;

	// 頂点バッファ（8頂点の±1ボックス）
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};

	// インデックスバッファ（36インデックス = 12三角形）
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};

	// WVP定数バッファ（回転のみのビュープロジェクション）
	Microsoft::WRL::ComPtr<ID3D12Resource> transformBuffer_;
	SkyboxTransformData* transformData_ = nullptr;

	// マテリアル定数バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialBuffer_;
	SkyboxMaterialData* materialData_ = nullptr;

};
