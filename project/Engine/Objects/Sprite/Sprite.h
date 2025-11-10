#pragma once

#include <string>
#include <memory>
#include <d3d12.h>
#include <wrl.h>

#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Structures.h"
#include "Transform2D.h"  // Transform2D
#include "SpriteCommon.h" //共通設定

#include "Managers/Texture/TextureManager.h"
#include "Managers/ObjectID/ObjectIDManager.h"
using namespace MyMath;



/// <summary>
/// 2Dスプライト専用クラス
/// </summary>
class Sprite
{
	/// <summary>
	/// スプライト専用のマテリアル構造体
	/// </summary>
	struct SpriteMaterial {
		Vector4 color;			// 色
		Matrix4x4 uvTransform;	// UV変換行列
	};


public:
	Sprite() = default;
	~Sprite() = default;

	/// <summary>
	/// 初期化（centerとsizeはTransform2Dで管理）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="textureName">テクスチャ名</param>
	/// <param name="center">中心座標（Transform2Dの初期位置として設定）</param>
	/// <param name="size">サイズ（Transform2Dの初期スケールとして設定）</param>
	/// <param name="anchor">アンカーポイント（0.0-1.0の範囲、デフォルトは中央{0.5f, 0.5f}）</param>
	void Initialize(
		DirectXCommon* dxCommon,
		const std::string& textureName,
		const Vector2& center,
		const Vector2& size,
		const Vector2& anchor = { 0.5f, 0.5f }
	);

	/// <summary>
	/// テクスチャなしで初期化（サイズのみ指定、位置は原点）
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="center">中心座標</param>
	/// <param name="size">サイズ</param>
	/// <param name="anchor">アンカーポイント（0.0-1.0の範囲、デフォルトは中央{0.5f, 0.5f}）</param>
	void Initialize(
		DirectXCommon* dxCommon,
		const Vector2& center = { 50.0f, 50.0f },
		const Vector2& size = { 100.0f, 100.0f },
		const Vector2& anchor = { 0.5f, 0.5f }
	);

	/// <summary>
	/// 更新処理（スプライト専用のビュープロジェクション）
	/// </summary>
	/// <param name="viewProjectionMatrix">スプライト用ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 通常の描画処理（UI用スプライト専用）
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	// Transform関連のGetter
	Vector2 GetPosition() const { return transform_.GetPosition(); }
	float GetRotation() const { return transform_.GetRotation(); }
	Vector2 GetScale() const { return transform_.GetScale(); }
	Transform2D& GetTransform() { return transform_; }
	const Transform2D& GetTransform() const { return transform_; }

	// Sprite固有のGetter
	Vector4 GetColor() const { return materialData_->color; }
	const std::string& GetName() const { return name_; }
	const std::string& GetTextureName() const { return textureName_; }
	Vector2 GetAnchor() const { return anchor_; }
	bool GetFlipX() const { return isFlipX_; }
	bool GetFlipY() const { return isFlipY_; }

	Vector2 GetTextureLeftTop() const { return texLeftTop_; }
	Vector2 GetTextureSize() const { return texSize_; }

	// Transform関連のSetter
	void SetTransform(const Vector2Transform& newTransform) { transform_.SetTransform(newTransform); }
	void SetPosition(const Vector2& position) { transform_.SetPosition(position); }
	void SetRotation(float rotation) { transform_.SetRotation(rotation); }
	void SetScale(const Vector2& scale) { transform_.SetScale(scale); }

	// Sprite固有のSetter
	void SetColor(const Vector4& color);
	void SetName(const std::string& name) { name_ = name; }
	void SetTexture(const std::string& textureName) { textureName_ = textureName; }
	void SetAnchor(const Vector2& anchor);
	void SetFlipX(const bool& flipX);
	void SetFlipY(const bool& flipY);


	// UVTransform関連
	void SetUVTransformScale(const Vector2& uvScale);
	void SetUVTransformRotateZ(float uvRotateZ);
	void SetUVTransformTranslate(const Vector2& uvTranslate);
	Vector2 GetUVTransformScale() const { return uvScale_; }
	float GetUVTransformRotateZ() const { return uvRotateZ_; }
	Vector2 GetUVTransformTranslate() const { return uvTranslate_; }

	//テクスチャの切り出し関連
	void SetTextureRect(const Vector2& texLeftTop, const Vector2& texSize);




private:

	/// <summary>
	/// アンカーからプライトメッシュを作成
	/// </summary>
	void CreateSpriteMesh();

	/// <summary>
	/// バッファを作成
	/// </summary>
	void CreateBuffers();

	/// <summary>
	/// UVトランスフォーム行列を更新
	/// </summary>
	void UpdateUVTransform();


	/// <summary>
	/// サイズをイメージに合わせる
	/// 画像の大きさと同じ大きさにscaleを変える
	/// </summary>
	void AdjustTextureSize();

	/// <summary>
	/// texSizeを、メタデータに合わせる
	/// </summary>
	/// <returns></returns>
	void ApplyMetadataToTexSize();

	/// <summary>
	/// flip関連のimguibuttonを作成する関数
	/// </summary>
	/// <param name="isOn"></param>
	/// <param name="text"></param>
	/// <param name="size"></param>
	void ImGuiChangeFlipButtonX();
	void ImGuiChangeFlipButtonY();


	// 基本情報
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = TextureManager::GetInstance();
	SpriteCommon* spriteCommon_ = SpriteCommon::GetInstance();


	std::string name_ = "Sprite";
	std::string textureName_ = "";

	// アンカーポイント（0.0-1.0の範囲）
	Vector2 anchor_{ 0.5f, 0.5f };

	// Transform2Dクラスを使用
	Transform2D transform_;

	// SpriteMaterial構造体に対応したマテリアルデータ
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
	SpriteMaterial* materialData_ = nullptr;

	// UV変換用のローカル変数
	Vector2 uvTranslate_{ 0.0f, 0.0f };
	Vector2 uvScale_{ 1.0f, 1.0f };
	float uvRotateZ_ = 0.0f;

	//反転フラグ
	bool isFlipX_;//左右反転
	bool isFlipY_;//上下判定

	// テクスチャ切り出し用のパラメータ
	Vector2 texLeftTop_{ 0.0f, 0.0f };		// テクスチャ左上座標
	Vector2 texSize_{ 0.0f, 0.0f };			// テクスチャ切り出しサイズ

	// メッシュデータ
	std::vector<VertexData> vertices_;
	std::vector<uint32_t> indices_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer_;
	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	D3D12_INDEX_BUFFER_VIEW indexBufferView_{};


};