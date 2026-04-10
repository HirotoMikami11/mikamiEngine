#include "Sprite.h"
#include <cassert>
#include <cstring>
#include "ImGui/ImGuiManager.h"
#include "SpriteRenderer.h"

void Sprite::Initialize(DirectXCommon* dxCommon, const std::string& textureName, const Vector2& center, const Vector2& size, const Vector2& anchor)
{
	dxCommon_ = dxCommon;
	textureName_ = textureName;
	anchor_ = anchor;

	// ID生成
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
	name_ = idManager->GenerateName("Sprite");

	// アンカーポイントを考慮したメッシュを作成
	CreateSpriteMesh();

	// Transform2Dクラスを初期化（GPU バッファなし）
	transform_.Initialize();

	// centerとsizeをTransform2Dに設定
	Vector2Transform initialTransform{
		size,   // scale = size
		0.0f,   // rotateZ
		center  // translate = center
	};
	transform_.SetTransform(initialTransform);

	// 頂点・インデックスバッファを作成
	CreateBuffers();

	// テクスチャのサイズに切り取りを合わせる
	ApplyMetadataToTexSize();
}

void Sprite::Initialize(DirectXCommon* dxCommon, const Vector2& center, const Vector2& size, const Vector2& anchor)
{
	dxCommon_ = dxCommon;
	textureName_ = "white";
	anchor_ = anchor;

	// ID生成
	ObjectIDManager* idManager = ObjectIDManager::GetInstance();
	name_ = idManager->GenerateName("Sprite");

	// アンカーポイントを考慮したメッシュを作成
	CreateSpriteMesh();

	// Transform2Dクラスを初期化（GPU バッファなし）
	transform_.Initialize();

	// centerとsizeをTransform2Dに設定
	Vector2Transform initialTransform{
		size,   // scale = size
		0.0f,   // rotateZ
		center  // translate = center
	};
	transform_.SetTransform(initialTransform);

	// 頂点・インデックスバッファを作成
	CreateBuffers();

	// テクスチャのサイズに切り取りを合わせる
	ApplyMetadataToTexSize();
}

void Sprite::Update(const Matrix4x4& viewProjectionMatrix)
{
	// Transform2Dクラスを使用して行列更新
	transform_.UpdateMatrix(viewProjectionMatrix);
}

void Sprite::Draw()
{
	SpriteRenderer* renderer = SpriteRenderer::GetInstance();

	// TransformationMatrix スロットを確保して CPU データを書き込む
	auto transformAlloc = renderer->AllocateTransform();
	*transformAlloc.cpuPtr = transform_.BuildTransformData();

	// SpriteMaterialData スロットを確保して CPU データを書き込む
	auto materialAlloc = renderer->AllocateMaterial();
	*materialAlloc.cpuPtr = BuildMaterialData();

	// SpriteSubmission を構築して SpriteRenderer のキューに積む
	SpriteSubmission submission{};
	submission.vbv = &vertexBufferView_;
	submission.ibv = &indexBufferView_;
	submission.indexCount = static_cast<uint32_t>(indices_.size());
	submission.transformGpuAddr = transformAlloc.gpuAddr;
	submission.materialGpuAddr = materialAlloc.gpuAddr;
	if (!textureName_.empty()) {
		submission.textureHandle = textureManager_->GetTextureHandle(textureName_);
	}
	submission.layerOrder = layerOrder_;
	submission.group = RenderGroup::UI;

	renderer->Submit(submission);
}

void Sprite::ImGui()
{
#ifdef USEIMGUI
	if (ImGui::TreeNode(name_.c_str())) {

		// Transform（Transform2Dクラスを使用、2D用UIに最適化）
		if (ImGui::CollapsingHeader("Transform")) {

			// トランスフォームのimgui
			transform_.ImGui();

			// アンカーポイント設定
			Vector2 imguiAnchor_ = anchor_;
			if (ImGui::DragFloat2("Anchor", &imguiAnchor_.x, 0.01f, 0.0f, 1.0f)) {
				SetAnchor(imguiAnchor_);
			}

			// 上下左右反転
			ImGuiChangeFlipButtonX();
			ImGuiChangeFlipButtonY();
		}

		// Color & UVTransform
		if (ImGui::CollapsingHeader("Material")) {

			// 色の変更
			Vector4 color = cpuData_.color;
			if (ImGui::ColorEdit4("Color", reinterpret_cast<float*>(&color.x))) {
				SetColor(color);
			}

			// UVTransform
			ImGui::Text("UVTransform");

			if (ImGui::DragFloat2("UVtranslate", &uvTranslate_.x, 0.01f, -10.0f, 10.0f)) {
				UpdateUVTransform();
			}
			if (ImGui::DragFloat2("UVscale", &uvScale_.x, 0.01f, -10.0f, 10.0f)) {
				UpdateUVTransform();
			}
			if (ImGui::SliderAngle("UVrotate", &uvRotateZ_)) {
				UpdateUVTransform();
			}

			// 範囲を切り取るとき
			ImGui::Text("Crop texture");
			DirectX::TexMetadata metadata = textureManager_->GetTextureMetadata(textureName_);
			Vector2 maxTextureSize = { static_cast<float>(metadata.width), static_cast<float>(metadata.height) };
			if (ImGui::DragFloat("TexLeftTop.x", &texLeftTop_.x, 1.0f, 0, maxTextureSize.x)) {
				SetTextureRect(texLeftTop_, texSize_);
			}
			if (ImGui::DragFloat("TexLeftTop.y", &texLeftTop_.y, 1.0f, 0, maxTextureSize.y)) {
				SetTextureRect(texLeftTop_, texSize_);
			}
			if (ImGui::DragFloat("TexSize.x", &texSize_.x, 1.0f, 0, maxTextureSize.x)) {
				SetTextureRect(texLeftTop_, texSize_);
			}
			if (ImGui::DragFloat("TexSize.y", &texSize_.y, 1.0f, 0, maxTextureSize.y)) {
				SetTextureRect(texLeftTop_, texSize_);
			}
		}

		// テクスチャ設定
		if (ImGui::CollapsingHeader("Texture")) {
			ImGui::Text("Current Texture: %s", textureName_.c_str());

			std::vector<std::string> textureList = textureManager_->GetTextureTagList();
			if (!textureList.empty()) {
				std::vector<const char*> textureNames;
				textureNames.push_back("Default");
				for (const auto& texture : textureList) {
					textureNames.push_back(texture.c_str());
				}
				int currentIndex = 0;
				if (!textureName_.empty()) {
					for (size_t i = 0; i < textureList.size(); ++i) {
						if (textureList[i] == textureName_) {
							currentIndex = static_cast<int>(i + 1);
							break;
						}
					}
				}
				if (ImGui::Combo("Custom Texture", &currentIndex, textureNames.data(), static_cast<int>(textureNames.size()))) {
					if (currentIndex == 0) {
						SetTexture("");
					} else {
						SetTexture(textureList[currentIndex - 1]);
					}
				}
			} else {
				ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No textures loaded");
			}
		}

		ImGui::TreePop();
	}
#endif
}

void Sprite::SetColor(const Vector4& color)
{
	cpuData_.color = color;
}

void Sprite::SetAnchor(const Vector2& anchor)
{
	anchor_ = anchor;
	// アンカーが変更されたらメッシュを再生成
	CreateSpriteMesh();

	// 頂点バッファを更新
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
}

void Sprite::SetFlipX(const bool& flipX)
{
	isFlipX_ = flipX;
	// メッシュを再生成
	CreateSpriteMesh();

	// 頂点バッファを更新
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
}

void Sprite::SetFlipY(const bool& flipY)
{
	isFlipY_ = flipY;
	// メッシュを再生成
	CreateSpriteMesh();

	// 頂点バッファを更新
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());

}

void Sprite::SetUVTransformScale(const Vector2& uvScale)
{
	uvScale_ = uvScale;
	UpdateUVTransform();
}

void Sprite::SetUVTransformRotateZ(float uvRotateZ)
{
	uvRotateZ_ = uvRotateZ;
	UpdateUVTransform();
}

void Sprite::SetUVTransformTranslate(const Vector2& uvTranslate)
{
	uvTranslate_ = uvTranslate;
	UpdateUVTransform();
}

void Sprite::CreateSpriteMesh()
{
	//アンカーを基準とした正方形
	vertices_.resize(4);

	// アンカーに基づいて頂点座標を計算
	float left = -anchor_.x;
	float right = 1.0f - anchor_.x;
	float top = -anchor_.y;
	float bottom = 1.0f - anchor_.y;
	//左右反転
	if (isFlipX_) { left = -left; right = -right; }

	//上下反転
	if (isFlipY_) { top = -top;  bottom = -bottom; }

	// 左下
	vertices_[0].position = { left,  bottom, 0.0f, 1.0f };
	vertices_[0].texcoord = { 0.0f,  1.0f };
	vertices_[0].normal = { 0.0f,  0.0f, -1.0f };

	// 左上
	vertices_[1].position = { left,  top,    0.0f, 1.0f };
	vertices_[1].texcoord = { 0.0f,  0.0f };
	vertices_[1].normal = { 0.0f,  0.0f, -1.0f };

	// 右下
	vertices_[2].position = { right, bottom, 0.0f, 1.0f };
	vertices_[2].texcoord = { 1.0f,  1.0f };
	vertices_[2].normal = { 0.0f,  0.0f, -1.0f };

	// 右上
	vertices_[3].position = { right, top,    0.0f, 1.0f };
	vertices_[3].texcoord = { 1.0f,  0.0f };
	vertices_[3].normal = { 0.0f,  0.0f, -1.0f };

	indices_ = { 0, 1, 2, 1, 3, 2 };
}

void Sprite::CreateBuffers()
{
	// 頂点バッファを作成
	vertexBuffer_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(VertexData) * vertices_.size());
	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());

	// 頂点バッファビューを設定
	vertexBufferView_.BufferLocation = vertexBuffer_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(VertexData) * vertices_.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// インデックスバッファを作成
	indexBuffer_ = CreateBufferResource(dxCommon_->GetDevice(), sizeof(uint32_t) * indices_.size());
	uint32_t* indexData = nullptr;
	indexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	std::memcpy(indexData, indices_.data(), sizeof(uint32_t) * indices_.size());

	indexBufferView_.BufferLocation = indexBuffer_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(uint32_t) * indices_.size());
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// マテリアルは CPU 管理（cpuData_）のみ。GPU バッファは SpriteRenderer が所有する。
	cpuData_.color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 白色
	UpdateUVTransform();							// UVTransformを初期化
}

void Sprite::UpdateUVTransform()
{
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix({ uvScale_.x, uvScale_.y, 1.0f });
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeRotateZMatrix(uvRotateZ_));
	uvTransformMatrix = Matrix4x4Multiply(uvTransformMatrix, MakeTranslateMatrix({ uvTranslate_.x, uvTranslate_.y, 0.0f }));

	cpuData_.uvTransform = uvTransformMatrix;
}

void Sprite::SetTextureRect(const Vector2& texLeftTop, const Vector2& texSize)
{
	texLeftTop_ = texLeftTop;
	texSize_ = texSize;
	// テクスチャが設定されていない場合は何もしない
	if (textureName_.empty()) return;

	// テクスチャのメタデータを取得
	DirectX::TexMetadata metadata = textureManager_->GetTextureMetadata(textureName_);

	// テクスチャサイズ(ピクセル単位)
	float textureWidth = static_cast<float>(metadata.width);
	float textureHeight = static_cast<float>(metadata.height);

	// テクスチャ座標を0.0-1.0の範囲に正規化
	float left = texLeftTop_.x / textureWidth;
	float top = texLeftTop_.y / textureHeight;
	float right = (texLeftTop_.x + texSize_.x) / textureWidth;
	float bottom = (texLeftTop_.y + texSize_.y) / textureHeight;

	vertices_[0].texcoord = { left,  bottom };
	vertices_[1].texcoord = { left,  top };
	vertices_[2].texcoord = { right, bottom };
	vertices_[3].texcoord = { right, top };

	VertexData* vertexData = nullptr;
	vertexBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, vertices_.data(), sizeof(VertexData) * vertices_.size());
	vertexBuffer_->Unmap(0, nullptr);
}

void Sprite::AdjustTextureSize()
{
	// テクスチャが設定されていない場合は何もしない
	if (textureName_.empty()) return;
	ApplyMetadataToTexSize();
	transform_.SetScale(texSize_);
}

void Sprite::ApplyMetadataToTexSize()
{
	DirectX::TexMetadata metadata = textureManager_->GetTextureMetadata(textureName_);
	texSize_ = {
		static_cast<float>(metadata.width),
		static_cast<float>(metadata.height)
	};
}

void Sprite::ImGuiChangeFlipButtonX()
{
#ifdef USEIMGUI
	ImVec2 bottomSize = { 255.0f,0.0f };
	if (ImGui::Button("isFlipX", bottomSize)) {
		isFlipX_ = !isFlipX_;
		SetFlipX(isFlipX_);
	}
#endif
}

void Sprite::ImGuiChangeFlipButtonY()
{
#ifdef USEIMGUI
	ImVec2 bottomSize = { 255.0f,0.0f };
	if (ImGui::Button("isFlipY", bottomSize)) {
		isFlipY_ = !isFlipY_;
		SetFlipY(isFlipY_);
	}
#endif
}
