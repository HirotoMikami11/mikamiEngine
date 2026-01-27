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
	modelSize = { 60.0f, 30.0f, 1.0f };

	// 囲みたい領域（フルサイズ）。デフォルトは modelSize.x を幅・奥行きに使う
	areaSize_ = { modelSize.x, modelSize.x };
}

Wall::~Wall() {}

void Wall::Initialize(DirectXCommon* dxCommon)
{
	dxCommon_ = dxCommon;
	JsonSettings* json = JsonSettings::GetInstance();
	// JsonSettingsのグループを作成
	json->CreateGroup(kGroupPath_);
	json->LoadFiles();
	// デフォルト値を追加（既に存在する場合は追加されない）
	json->AddItem(kGroupPath_, "modelSize", modelSize);
	json->AddItem(kGroupPath_, "areaSize", areaSize_);

	// JSONファイルから値を読み込み
	modelSize = json->GetValue<Vector3>(kGroupPath_, "modelSize").value_or(modelSize);
	areaSize_ = json->GetValue<Vector2>(kGroupPath_, "areaSize").value_or(areaSize_);

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
				float sectionWidth = modelSize.x / 3.0f;
				defaultSize = { sectionWidth, modelSize.y, modelSize.z };
				defaultOffset.x = (j - 1) * sectionWidth; // -1, 0, 1 * sectionWidth
			} else {
				// 左と右の壁（Z軸方向に長い）
				// 壁を3分割して、手前、中央、奥に配置
				float sectionDepth = areaSize_.y / 3.0f;
				defaultSize = { modelSize.z, modelSize.y, sectionDepth };
				defaultOffset.z = (j - 1) * sectionDepth; // -1, 0, 1 * sectionDepth
			}

			// JsonSettingsからコライダーサイズとオフセットを読み込み（存在すれば）
			std::string colliderSizeKey = "colliderSize_" + std::to_string(i) + "_" + std::to_string(j);
			std::string colliderOffsetKey = "colliderOffset_" + std::to_string(i) + "_" + std::to_string(j);

			walls_[i].colliderSizes[j] = json->GetValue<Vector3>(kGroupPath_, colliderSizeKey).value_or(defaultSize);
			walls_[i].colliderOffsets[j] = json->GetValue<Vector3>(kGroupPath_, colliderOffsetKey).value_or(defaultOffset);

			json->AddItem(kGroupPath_, colliderSizeKey, walls_[i].colliderSizes[j]);
			json->AddItem(kGroupPath_, colliderOffsetKey, walls_[i].colliderOffsets[j]);
		}
	}

	// 初回 transform 計算＆適用
	UpdateTransforms();

	// コライダーを初期化
	UpdateColliders();
}

void Wall::SaveToJson()
{
	JsonSettings* json = JsonSettings::GetInstance();
	json->SetValue(kGroupPath_, "modelSize", modelSize);
	json->SetValue(kGroupPath_, "areaSize", areaSize_);

	// 各壁の3つのコライダーサイズとオフセットを保存
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j) {
			std::string colliderSizeKey = "colliderSize_" + std::to_string(i) + "_" + std::to_string(j);
			std::string colliderOffsetKey = "colliderOffset_" + std::to_string(i) + "_" + std::to_string(j);
			
			json->SetValue(kGroupPath_, colliderSizeKey, walls_[i].colliderSizes[j]);
			json->SetValue(kGroupPath_, colliderOffsetKey, walls_[i].colliderOffsets[j]);
		}
	}

	json->SaveFile(kGroupPath_);
}

void Wall::ApplyParameters()
{
	// JsonSettingsから現在の値を取得して適用
	JsonSettings* json = JsonSettings::GetInstance();

	// value_or()でデフォルト値を指定
	modelSize = json->GetValue<Vector3>(kGroupPath_, "modelSize").value_or(modelSize);
	areaSize_ = json->GetValue<Vector2>(kGroupPath_, "areaSize").value_or(areaSize_);

	// 各壁の3つのコライダーサイズとオフセットを読み込み
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j) {
			std::string colliderSizeKey = "colliderSize_" + std::to_string(i) + "_" + std::to_string(j);
			std::string colliderOffsetKey = "colliderOffset_" + std::to_string(i) + "_" + std::to_string(j);
			
			walls_[i].colliderSizes[j] = json->GetValue<Vector3>(kGroupPath_, colliderSizeKey).value_or(walls_[i].colliderSizes[j]);
			walls_[i].colliderOffsets[j] = json->GetValue<Vector3>(kGroupPath_, colliderOffsetKey).value_or(walls_[i].colliderOffsets[j]);
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
	areaSize_.x = (areaSize.x > 0.0001f) ? areaSize.x : modelSize.x;
	areaSize_.y = (areaSize.y > 0.0001f) ? areaSize.y : modelSize.x;

	// JsonSettingsに反映
	JsonSettings::GetInstance()->SetValue(kGroupPath_, "areaSize", areaSize_);

	// 変更があったら transform を再計算して反映
	UpdateTransforms();

	// コライダーを更新
	UpdateColliders();
}

void Wall::UpdateTransforms()
{
	// modelSize: scale==1 のときの実寸
	const float modelHalfX = modelSize.x * 0.5f;
	const float modelHalfY = modelSize.y * 0.5f;
	const float modelHalfZ = modelSize.z * 0.5f;

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

		if (i == 0) continue;

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

	if (ImGui::TreeNode("Walls")) {
		// areaSize の表示/編集
		Vector2 a = areaSize_;
		bool changed = false;


		if (ImGui::DragFloat2("Area", &a.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(kGroupPath_, "areaSize", a);
			changed = true;
		}


		if (ImGui::DragFloat3("ModelSize", &modelSize.x, 0.1f, 0.0f, 100.0f, "%.2f")) {
			json->SetValue(kGroupPath_, "modelSize", modelSize);
			changed = true;
		}

		if (changed) {
			// 入力を検証して適用
			if (a.x <= 0.0f) a.x = modelSize.x;
			if (a.y <= 0.0f) a.y = modelSize.x;
			areaSize_.x = a.x;
			areaSize_.y = a.y;

			// modelSize の変更に合わせて area デフォルトを変える等するならここで処理
			UpdateTransforms();
			UpdateColliders();
		}

		// 保存ボタン
		if (ImGui::Button("Save Wall Settings")) {
			SaveToJson();
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
							// コライダーの中心座標オフセット調整
							Vector3 offset = walls_[i].colliderOffsets[j];
							if (ImGui::DragFloat3("Center Offset (X,Y,Z)", &offset.x, 0.1f, -100.0f, 100.0f, "%.2f")) {
								walls_[i].colliderOffsets[j] = offset;
								colliderChanged = true;
							}

							// コライダーサイズの調整
							Vector3 size = walls_[i].colliderSizes[j];
							if (ImGui::DragFloat3("Size (X,Y,Z)", &size.x, 0.1f, 0.1f, 100.0f, "%.2f")) {
								walls_[i].colliderSizes[j] = size;
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
