#pragma once
#include <string>
#include <memory>
#include "DirectXCommon.h"
#include "Transform3D.h"
#include "Object3DCommon.h"
#include "Light.h"
#include "Texture/TextureManager.h"
#include "Model/ModelManager.h"
#include "ObjectID/ObjectIDManager.h"

/// <summary>
/// ゲームオブジェクト - 共有モデル（メッシュ）と個別Transform、個別マテリアル
/// </summary>
class Object3D
{
public:
	Object3D() = default;
	virtual ~Object3D() = default;

	/// <summary>
	/// 初期化（共有モデルを使用）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="modelTag">共有モデルのタグ名</param>
	/// <param name="textureName">テクスチャ名（プリミティブ用、空文字列の場合はテクスチャなし）</param>
	virtual void Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName = "");

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight);

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	virtual void ImGui();

	// Transform関連
	Vector3 GetPosition() const { return transform_.GetPosition(); }
	Vector3 GetRotation() const { return transform_.GetRotation(); }
	Vector3 GetScale() const { return transform_.GetScale(); }
	Transform3D& GetTransform() { return transform_; }
	const Transform3D& GetTransform() const { return transform_; }

	void SetTransform(const Vector3Transform& newTransform) { transform_.SetTransform(newTransform); }
	void SetPosition(const Vector3& position) { transform_.SetPosition(position); }
	void SetRotation(const Vector3& rotation) { transform_.SetRotation(rotation); }
	void SetScale(const Vector3& scale) { transform_.SetScale(scale); }

	void AddPosition(const Vector3& deltaPosition) { transform_.AddPosition(deltaPosition); }
	void AddRotation(const Vector3& deltaRotation) { transform_.AddRotation(deltaRotation); }
	void AddScale(const Vector3& deltaScale) { transform_.AddScale(deltaScale); }

	// Model関連
	Model* GetModel() { return sharedModel_; }
	const Model* GetModel() const { return sharedModel_; }
	void SetModel(const std::string& modelTag, const std::string& textureName = "");


	// マテリアル操作
	Material& GetMaterial(size_t index = 0) { return materials_.GetMaterial(index); }
	const Material& GetMaterial(size_t index = 0) const { return materials_.GetMaterial(index); }
	size_t GetMaterialCount() const { return materials_.GetMaterialCount(); }
	Vector4 GetColor(size_t index = 0) const { return GetMaterial(index).GetColor(); }
	LightingMode GetLightingMode(size_t index = 0) const { return GetMaterial(index).GetLightingMode(); }


	void SetColor(const Vector4& color, size_t index = 0) { materials_.GetMaterial(index).SetColor(color); }
	void SetColor(const uint32_t& color, size_t index = 0) { materials_.GetMaterial(index).SetColor(Uint32ToColorVector(color)); }
	void SetLightingMode(LightingMode mode, size_t index = 0) { materials_.GetMaterial(index).SetLightingMode(mode); }
	// 全マテリアルに同じ設定を適用
	void SetAllMaterialsColor(const Vector4& color, LightingMode mode = LightingMode::HalfLambert) { materials_.SetAllMaterials(color, mode); }

	// UV操作
	void SetUVTransformScale(const Vector2& scale, size_t index = 0) { materials_.GetMaterial(index).SetUVTransformScale(scale); }
	void SetUVTransformRotateZ(float rotate, size_t index = 0) { materials_.GetMaterial(index).SetUVTransformRotateZ(rotate); }
	void SetUVTransformTranslate(const Vector2& translate, size_t index = 0) { materials_.GetMaterial(index).SetUVTransformTranslate(translate); }

	// オブジェクト状態
	const std::string& GetName() const { return name_; }
	const std::string& GetModelTag() const { return modelTag_; }
	void SetName(const std::string& name) { name_ = name; }

	// テクスチャ操作
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }
	const std::string& GetTextureName() const { return textureName_; }
	bool HasCustomTexture() const { return !textureName_.empty(); }

protected:
	Transform3D transform_;					// 個別のトランスフォーム
	Model* sharedModel_ = nullptr;			// 共有モデル（メッシュとテクスチャ）Materialは別途用意することで同じモデルでMaterial情報が変わらないようにする。
	MaterialGroup materials_;				// 個別マテリアル

	std::string name_ = "Object3D";
	std::string modelTag_ = "";
	std::string textureName_ = "";

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
	ModelManager* modelManager_ = ModelManager::GetInstance();
	Object3DCommon* object3DCommon_ = Object3DCommon::GetInstance();

private:

};


class Triangle : public Object3D {
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "triangle", const std::string& textureName = "white") {
		Object3D::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName("Triangle");
		SetLightingMode(LightingMode::HalfLambert);
	}
};

class Sphere : public Object3D {
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "sphere", const std::string& textureName = "white") {
		Object3D::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName("Sphere");
		SetLightingMode(LightingMode::HalfLambert);
	}
};

class Plane : public Object3D {
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag = "plane", const std::string& textureName = "white") {
		Object3D::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateName("Plane");
		SetLightingMode(LightingMode::HalfLambert);
	}
};

class Model3D : public Object3D {
public:
	void Initialize(DirectXCommon* dxCommon, const std::string& modelTag, const std::string& textureName = "") {
		Object3D::Initialize(dxCommon, modelTag, textureName);
		name_ = ObjectIDManager::GetInstance()->GenerateNameModel(std::format("{}", modelTag), "Model3D");
		SetLightingMode(LightingMode::HalfLambert);
	}
};