#pragma once
#include <string>
#include <memory>
#include <vector>
#include "DirectXCommon.h"
#include "ParticleState.h"
#include "ParticleCommon.h"
#include "Light.h"
#include "Managers/Texture/TextureManager.h"
#include "Managers/Model/ModelManager.h"
#include "CameraController.h"

/// <summary>
/// パーティクルシステム
/// <para>パーティクルの管理・更新・描画を担当</para>
/// </summary>
class Particle
{
public:
	Particle() = default;
	~Particle() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="modelTag">使用するモデルのタグ名</param>
	/// <param name="maxParticles">最大パーティクル数</param>
	/// <param name="textureName">テクスチャ名</param>
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag,
		uint32_t maxParticles, const std::string& textureName = "");

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="deltaTime">デルタタイム</param>
	void Update(const Matrix4x4& viewProjectionMatrix, float deltaTime);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// パーティクルを追加（エミッターから呼び出される）
	/// </summary>
	/// <param name="particle">追加するパーティクルの状態</param>
	/// <returns>追加に成功したらtrue</returns>
	bool AddParticle(const ParticleState& particle);

	/// <summary>
	/// すべてのパーティクルをクリア
	/// </summary>
	void ClearAllParticles();

	// パーティクル制御
	uint32_t GetActiveParticleCount() const { return activeParticleCount_; }
	uint32_t GetMaxParticleCount() const { return maxParticles_; }

	bool IsFull() const { return particles_.size() >= maxParticles_; }
	bool IsEmpty() const { return particles_.empty(); }

	// Model関連
	Model* GetModel() { return sharedModel_; }
	const Model* GetModel() const { return sharedModel_; }
	void SetModel(const std::string& modelTag, const std::string& textureName = "");

	// マテリアル操作
	Material& GetMaterial(size_t index = 0) { return materials_.GetMaterial(index); }
	const Material& GetMaterial(size_t index = 0) const { return materials_.GetMaterial(index); }
	size_t GetMaterialCount() const { return materials_.GetMaterialCount(); }

	void SetAllMaterialsColor(const Vector4& color, LightingMode mode = LightingMode::HalfLambert) {
		materials_.SetAllMaterials(color, mode);
	}

	// オブジェクト状態
	const std::string& GetName() const { return name_; }
	void SetName(const std::string& name) { name_ = name; }

	// テクスチャ操作
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }
	const std::string& GetTextureName() const { return textureName_; }

	// ビルボード設定
	void SetBillboard(bool enabled) { isBillboard_ = enabled; }
	bool IsBillboard() const { return isBillboard_; }

private:
	/// <summary>
	/// GPU転送用のトランスフォームバッファを作成
	/// </summary>
	void CreateTransformBuffer();

	/// <summary>
	/// GPU転送用のデータを更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void UpdateParticleForGPUBuffer(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// パーティクルの物理更新
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	void UpdateParticles(float deltaTime);

	/// <summary>
	/// ビルボード行列を作成
	/// </summary>
	/// <param name="viewMatrix">カメラ行列</param>
	void MakeBillboardMatrix(const Matrix4x4& viewMatrix);

	// パーティクルデータ
	std::vector<ParticleState> particles_;
	uint32_t maxParticles_ = 0;			// 最大パーティクル数
	uint32_t activeParticleCount_ = 0;	// アクティブなパーティクル数

	// GPU転送用バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> transformResource_;
	ParticleForGPU* instancingData_ = nullptr;
	DescriptorHeapManager::DescriptorHandle srvHandle_;

	// モデルとマテリアル
	Model* sharedModel_ = nullptr;
	MaterialGroup materials_;

	std::string name_ = "Particle";
	std::string modelTag_ = "";
	std::string textureName_ = "";

	// ビルボード機能
	bool isBillboard_ = true;
	Matrix4x4 billboardMatrix_;

	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
	ModelManager* modelManager_ = ModelManager::GetInstance();
	ParticleCommon* particleCommon_ = ParticleCommon::GetInstance();
	CameraController* cameraController_ = CameraController::GetInstance();
};