#include "Wall.h"
#include "ImGui/ImGuiManager.h"
#include "JsonSettings.h"
#include "CollisionConfig.h"
#include <numbers>
#include <filesystem>

void WallCollider::Initialize(const Vector3& position, const Vector3& aabbSize) {
	position_ = position;

	// AABBコライダーとして設定
	SetColliderType(ColliderType::AABB);

	// AABBサイズを設定
	SetAABBSize(aabbSize);

	// 衝突属性を設定（オブジェクト属性）
	SetCollisionAttribute(kCollisionAttributeObjects);
	// プレイヤー弾とボス弾に反応
	SetCollisionMask(kCollisionAttributePlayerBullet | kCollisionAttributeEnemyBullet);

	// デバッグ表示設定
	SetColliderVisible(true);
	SetDefaultColliderColor(0xFFFF00FF);  // 黄色
}

void WallCollider::SetAABBSize(const Vector3& size) {
	aabbSize_ = size;  // サイズを保存
	Vector3 halfSize = { size.x * 0.5f, size.y * 0.5f, size.z * 0.5f };
	AABB aabb;
	aabb.min = -halfSize;
	aabb.max = halfSize;
	SetAABB(aabb);
}

void WallCollider::OnCollision(Collider* other) {
	// 特に何もしない（弾が既にエフェクトを出し、自身を非アクティブ化するから）
}

Wall::Wall()
	: dxCommon_(nullptr) {

	// 壁モデルサイズ（scale が 1 のときの実寸法）
	// {長さ（X方向に対応）, 高さ, 厚み（Z方向）}
	modelSize_ = { 60.0f, 30.0f, 1.0f };

	// 囲みたい領域（フルサイズ）。デフォルトは modelSize_.x を幅・奥行きに使う
	areaSize_ = { modelSize_.x, modelSize_.x };
}

Wall::~Wall() {}

void Wall::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	JsonSettings* json = JsonSettings::GetInstance();

	// グローバル変数グループを作成
	auto groupName = GetGlobalVariableGroupName();
	json->CreateGroup(groupName);
	json->LoadFiles();

	// デフォルト値を追加（既に存在する場合は追加されない）
	json->AddItem(groupName, "modelSize", modelSize_);
	json->AddItem(groupName, "areaSize", areaSize_);

	// 各壁モデルを初期化して transform をセットする
	for (int i = 0; i < 4; ++i) {
		walls_[i].obj = std::make_unique<Model3D>();
		walls_[i].obj->Initialize(dxCommon_, "wall");
		walls_[i].obj->SetColor(0x3F3A38FF);

		// SetName にインデックスを反映
		char nameBuf[32];
		snprintf(nameBuf, sizeof(nameBuf), "Wall[%d]", i);
		walls_[i].obj->SetName(nameBuf);

		// scale は固定 (1,1,1)
		walls_[i].transform.scale = { 1.0f, 1.0f, 1.0f };

		// 各壁に3つのコライダーを作成
		for (int j = 0; j < 3; ++j) {
			walls_[i].colliders[j] = std::make_unique<WallCollider>();

			// デフォルトのコライダーサイズとオフセットを設定
			Vector3 defaultSize;
			Vector3 defaultOffset = { 0.0f, 0.0f, 0.0f };

			if (i == 0 || i == 2) {
				// 前と後ろの壁（X軸方向に長い）
				// 壁を3分割して、左、中央、右に配置
				float sectionWidth = modelSize_.x / 3.0f;
				defaultSize = { sectionWidth, modelSize_.y, modelSize_.z };
				defaultOffset.x = (j - 1) * sectionWidth; // -1, 0, 1 * sectionWidth
			} else {
				// 左と右の壁（Z軸方向に長い）
				// 壁を3分割して、手前、中央、奥に配置
				float sectionDepth = areaSize_.y / 3.0f;
				defaultSize = { modelSize_.z, modelSize_.y, sectionDepth };
				defaultOffset.z = (j - 1) * sectionDepth; // -1, 0, 1 * sectionDepth
			}

			// JsonSettingsからコライダーサイズとオフセットを読み込み（存在すれば）
			std::string colliderSizeKey = "colliderSize_" + std::to_string(i) + "_" + std::to_string(j);
			std::string colliderOffsetKey = "colliderOffset_" + std::to_string(i) + "_" + std::to_string(j);

			walls_[i].colliderSizes[j] = defaultSize;
			walls_[i].colliderOffsets[j] = defaultOffset;

			json->AddItem(groupName, colliderSizeKey, walls_[i].colliderSizes[j]);
			json->AddItem(groupName, colliderOffsetKey, walls_[i].colliderOffsets[j]);
		}
	}

	// グローバル変数から値を適用
	ApplyGlobalVariables();

	// コライダーを初期化
	UpdateColliders();
}

void Wall::ApplyGlobalVariables()
{
	JsonSettings* json = JsonSettings::GetInstance();
	auto groupName = GetGlobalVariableGroupName();

	// 基本パラメータを取得
	modelSize_ = json->GetVector3Value(groupName, "modelSize");
	areaSize_ = json->GetVector2Value(groupName, "areaSize");

	// 各壁の3つのコライダーサイズとオフセットを読み込み
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j) {
			std::string colliderSizeKey = "colliderSize_" + std::to_string(i) + "_" + std::to_string(j);
			std::string colliderOffsetKey = "colliderOffset_" + std::to_string(i) + "_" + std::to_string(j);

			walls_[i].colliderSizes[j] = json->GetVector3Value(groupName, colliderSizeKey);
			walls_[i].colliderOffsets[j] = json->GetVector3Value(groupName, colliderOffsetKey);
		}
	}

	// transformを再計算
	UpdateTransforms();

	// コライダーを更新
	UpdateColliders();
}

void Wall::SetAreaSize(const Vector2& areaSize)
{
	// 0 や負の値は受け付けない（最低値を設定）
	areaSize_.x = (areaSize.x > 0.0001f) ? areaSize.x : modelSize_.x;
	areaSize_.y = (areaSize.y > 0.0001f) ? areaSize.y : modelSize_.x;

	// JsonSettingsに反映
	JsonSettings::GetInstance()->SetValue(GetGlobalVariableGroupName(), "areaSize", areaSize_);

	// 変更があったら transform を再計算して反映
	UpdateTransforms();

	// コライダーを更新
	UpdateColliders();
}

void Wall::UpdateTransforms()
{
	// modelSize_: scale==1 のときの実寸
	const float modelHalfX = modelSize_.x * 0.5f;
	const float modelHalfY = modelSize_.y * 0.5f;
	const float modelHalfZ = modelSize_.z * 0.5f;

	// area の半分
	const float areaHalfX = areaSize_.x * 0.5f;
	const float areaHalfZ = areaSize_.y * 0.5f;

	// Y位置（モデルの中心が原点にある想定 modelHalfY を使う）
	const float yPos = modelHalfY;

	// 各面の回転（前, 右, 後, 左）
	const float rotY[4] = {
		0.0f,
		std::numbers::pi_v<float> / 2.0f,
		std::numbers::pi_v<float>,
		-std::numbers::pi_v<float> / 2.0f
	};

	// 手前 (-Z)
	walls_[0].transform.rotate = { 0.0f, rotY[0], 0.0f };
	walls_[0].transform.translate = { 0.0f, yPos, -(areaHalfZ + modelHalfZ) };
	walls_[0].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[0].obj->SetTransform(walls_[0].transform);

	// 右 (+X)
	walls_[1].transform.rotate = { 0.0f, rotY[1], 0.0f };
	walls_[1].transform.translate = { (areaHalfX + modelHalfZ), yPos, 0.0f };
	walls_[1].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[1].obj->SetTransform(walls_[1].transform);

	// 奥 (+Z)
	walls_[2].transform.rotate = { 0.0f, rotY[2], 0.0f };
	walls_[2].transform.translate = { 0.0f, yPos, (areaHalfZ + modelHalfZ) };
	walls_[2].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[2].obj->SetTransform(walls_[2].transform);

	// 左 (-X)
	walls_[3].transform.rotate = { 0.0f, rotY[3], 0.0f };
	walls_[3].transform.translate = { -(areaHalfX + modelHalfZ), yPos, 0.0f };
	walls_[3].transform.scale = { 1.0f, 1.0f, 1.0f };
	walls_[3].obj->SetTransform(walls_[3].transform);
}

void Wall::UpdateColliders()
{
	// 各壁の3つのコライダーサイズと位置を更新
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (walls_[i].colliders[j]) {
				// 壁のベース位置にオフセットを加えて配置
				Vector3 colliderPosition = walls_[i].transform.translate;

				// オフセットを回転に合わせて適用
				float rotY = walls_[i].transform.rotate.y;
				float cosRot = std::cos(rotY);
				float sinRot = std::sin(rotY);

				Vector3 offset = walls_[i].colliderOffsets[j];
				Vector3 rotatedOffset;
				rotatedOffset.x = offset.x * cosRot - offset.z * sinRot;
				rotatedOffset.y = offset.y;
				rotatedOffset.z = offset.x * sinRot + offset.z * cosRot;

				colliderPosition.x += rotatedOffset.x;
				colliderPosition.y += rotatedOffset.y;
				colliderPosition.z += rotatedOffset.z;

				// コライダーを初期化
				walls_[i].colliders[j]->Initialize(colliderPosition, walls_[i].colliderSizes[j]);
			}
		}
	}
}

std::vector<Collider*> Wall::GetColliders()
{
	std::vector<Collider*> colliders;
	colliders.reserve(12); // 4 walls × 3 colliders each

	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j) {
			if (walls_[i].colliders[j]) {
				colliders.push_back(walls_[i].colliders[j].get());
			}
		}
	}

	return colliders;
}

void Wall::Update(const Matrix4x4& viewProjectionMatrix)
{
	for (int i = 0; i < 4; ++i) {
		if (walls_[i].obj) walls_[i].obj->Update(viewProjectionMatrix);
	}
}

void Wall::Draw()
{
	for (int i = 0; i < 4; ++i) {
		if (i == 0) continue; // 手前の壁は描画しない

		if (walls_[i].obj) walls_[i].obj->Draw();
#ifdef USEIMGUI
		// コライダーのデバッグ表示（3つ全て）
		for (int j = 0; j < 3; ++j) {
			if (walls_[i].colliders[j]) {
				walls_[i].colliders[j]->DebugLineAdd();
			}
		}
#endif
	}
}

void Wall::ImGui()
{
#ifdef USEIMGUI
	JsonSettings* json = JsonSettings::GetInstance();
	auto groupName = GetGlobalVariableGroupName();

	if (ImGui::TreeNode("Walls")) {
		bool changed = false;

		// エリアサイズの調整
		if (ImGui::DragFloat2("Area", &areaSize_.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(groupName, "areaSize", areaSize_);
			changed = true;
		}

		// モデルサイズの調整
		if (ImGui::DragFloat3("ModelSize", &modelSize_.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(groupName, "modelSize", modelSize_);
			changed = true;
		}

		if (changed) {
			// 入力を検証して適用
			if (areaSize_.x <= 0.0f) areaSize_.x = modelSize_.x;
			if (areaSize_.y <= 0.0f) areaSize_.y = modelSize_.x;

			ApplyGlobalVariables();
		}

		ImGui::Separator();

		// 各壁のコライダーサイズ調整
		if (ImGui::TreeNode("Collider Sizes")) {
			const char* wallNames[4] = { "Front Wall (-Z)", "Right Wall (+X)", "Back Wall (+Z)", "Left Wall (-X)" };
			bool colliderChanged = false;

			for (int i = 0; i < 4; ++i) {
				ImGui::PushID(i);

				if (ImGui::TreeNode(wallNames[i])) {
					// 各壁の3つのコライダーを調整
					for (int j = 0; j < 3; ++j) {
						ImGui::PushID(j);

						char colliderName[64];
						snprintf(colliderName, sizeof(colliderName), "Collider [%d]", j);

						if (ImGui::TreeNode(colliderName)) {
							std::string colliderSizeKey = "colliderSize_" + std::to_string(i) + "_" + std::to_string(j);
							std::string colliderOffsetKey = "colliderOffset_" + std::to_string(i) + "_" + std::to_string(j);

							// コライダーの中心座標オフセット調整
							if (ImGui::DragFloat3("Center Offset (X,Y,Z)", &walls_[i].colliderOffsets[j].x, 0.1f, -100.0f, 100.0f, "%.2f")) {
								json->SetValue(groupName, colliderOffsetKey, walls_[i].colliderOffsets[j]);
								colliderChanged = true;
							}

							// コライダーサイズの調整
							if (ImGui::DragFloat3("Size (X,Y,Z)", &walls_[i].colliderSizes[j].x, 0.1f, 0.1f, 100.0f, "%.2f")) {
								json->SetValue(groupName, colliderSizeKey, walls_[i].colliderSizes[j]);
								colliderChanged = true;
							}

							// 現在のコライダー情報表示
							if (walls_[i].colliders[j]) {
								Vector3 currentSize = walls_[i].colliders[j]->GetAABBSize();
								ImGui::Text("Current Size: (%.2f, %.2f, %.2f)", currentSize.x, currentSize.y, currentSize.z);

								Vector3 pos = walls_[i].colliders[j]->GetWorldPosition();
								ImGui::Text("World Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
							}

							// コライダー表示設定
							if (walls_[i].colliders[j]) {
								bool visible = walls_[i].colliders[j]->IsColliderVisible();
								if (ImGui::Checkbox("Show Collider", &visible)) {
									walls_[i].colliders[j]->SetColliderVisible(visible);
								}
							}

							ImGui::TreePop();
						}

						ImGui::PopID();
					}

					ImGui::TreePop();
				}

				ImGui::PopID();
			}

			// コライダーサイズが変更された場合は更新
			if (colliderChanged) {
				UpdateColliders();
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		// モデル情報
		for (int i = 0; i < 4; ++i) {
			if (walls_[i].obj) walls_[i].obj->ImGui();
		}

		ImGui::TreePop();
	}
#endif
}