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
/// パーティクル
/// <para>構造上トランスフォームを独立させない</para>
/// <para>3Dobjectより後ろに描画する</para>
/// </summary>
class Particle
{
public:
	Particle() = default;
	virtual ~Particle() = default;

	/// <summary>
	/// 初期化（共有モデルを使用）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="modelTag">共有モデルのタグ名</param>
	/// <param name="numParticles">生成するパーティクル数</param>
	/// <param name="textureName">テクスチャ名（空文字列の場合はテクスチャなし）</param>
	virtual void Initialize(DirectXCommon* dxCommon, const std::string& modelTag,
		uint32_t numParticles, const std::string& textureName = "");

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="deltaTime">デルタタイム</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix, float deltaTime);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	virtual void ImGui();

	// パーティクル制御
	uint32_t GetParticleCount() const { return static_cast<uint32_t>(particles_.size()); }
	void SetEnableUpdate(bool enable) { enableUpdate_ = enable; }
	bool IsUpdateEnabled() const { return enableUpdate_; }

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
	/// 各パーティクルの初期状態を設定する
	/// </summary>
	ParticleState MakeNewParticle();

	/// <summary>
	/// ビルボード行列を作成
	/// </summary>
	/// <param name="viewMatrix">カメラ行列</param>
	/// <returns></returns>
	void MakeBillboardMatrix(const Matrix4x4& viewMatrix);

	// パーティクルデータ
	std::vector<ParticleState> particles_;
	uint32_t numMaxParticles_ = 0;		//表示する最大数
	uint32_t numInstance = 0;
	bool enableUpdate_ = false;			// 更新を有効にするか

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

	///ビルボード機能
	bool isBillboard_ = false;		// ビルボードを有効にするか
	Matrix4x4 billboardMatrix_;		// ビルボード行列


	// システム参照
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
	ModelManager* modelManager_ = ModelManager::GetInstance();
	ParticleCommon* particleCommon_ = ParticleCommon::GetInstance();
	CameraController* cameraController_ = CameraController::GetInstance();
};