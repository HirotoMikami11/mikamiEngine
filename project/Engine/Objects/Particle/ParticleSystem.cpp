#include "ParticleSystem.h"
#include "Managers/ImGui/ImGuiManager.h"
#include "Logger.h"

ParticleSystem* ParticleSystem::GetInstance()
{
	static ParticleSystem instance;
	return &instance;
}

void ParticleSystem::Initialize(DirectXCommon* dxCommon)
{
	directXCommon_ = dxCommon;
	billboardMatrix_ = MakeIdentity4x4();

	Logger::Log(Logger::GetStream(), "ParticleSystem: Initialized\n");
}

void ParticleSystem::Update(const Matrix4x4& viewProjectionMatrix, float deltaTime)
{
	// ビルボード行列を計算（全グループ共通）
	CalculateBillboardMatrix();

	// すべてのフィールドを更新
	for (auto& [fieldName, field] : fields_) {
		field->Update(deltaTime);
	}

	// すべてのエミッターを更新
	for (auto& [emitterName, emitter] : emitters_) {
		// ターゲットグループを取得
		ParticleGroup* targetGroup = GetGroup(emitter->GetTargetGroupName());

		// エミッターを更新（パーティクルを発生）
		emitter->Update(deltaTime, targetGroup);
	}

	// フィールドのポインタリストを作成
	std::vector<BaseField*> fieldPtrs;
	for (auto& [fieldName, field] : fields_) {
		fieldPtrs.push_back(field.get());
	}

	// すべてのグループを更新（フィールドを渡す）
	for (auto& [groupName, group] : groups_) {
		group->Update(viewProjectionMatrix, billboardMatrix_, deltaTime, fieldPtrs);
	}
}

void ParticleSystem::Draw(const Light& directionalLight)
{
	// すべてのグループを描画
	for (auto& [groupName, group] : groups_) {
		group->Draw(directionalLight);
	}

#ifdef USEIMGUI
	/// デバッグ描画(LineSystemに追加するだけでここで実際に描画してない)
	// すべてのエミッターのデバッグ描画
	for (auto& [emitterName, emitter] : emitters_) {
		emitter->AddLineDebug();
	}

	// すべてのフィールドのデバッグ描画
	for (auto& [fieldName, field] : fields_) {
		field->AddLineDebug();
	}
#endif 

}


void ParticleSystem::ImGui()
{
#ifdef USEIMGUI
	ImGui::Text("Fields: %zu", fields_.size());
	if (ImGui::TreeNode("Particle System")) {
		// 統計情報
		if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Groups: %zu", groups_.size());
			ImGui::Text("Emitters: %zu", emitters_.size());

			// 全パーティクル数の集計
			uint32_t totalActiveParticles = 0;
			uint32_t totalMaxParticles = 0;
			for (const auto& [name, group] : groups_) {
				totalActiveParticles += group->GetActiveParticleCount();
				totalMaxParticles += group->GetMaxParticleCount();
			}
			ImGui::Text("Total Particles: %u / %u", totalActiveParticles, totalMaxParticles);

			ImGui::Separator();
		}

		// パーティクルグループ
		if (ImGui::CollapsingHeader("Particle Groups")) {
			if (groups_.empty()) {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No groups created");
			} else {
				for (auto& [groupName, group] : groups_) {
					group->ImGui();
				}
			}
		}

		// エミッター
		if (ImGui::CollapsingHeader("Emitters")) {
			if (emitters_.empty()) {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No emitters created");
			} else {
				for (auto& [emitterName, emitter] : emitters_) {
					emitter->ImGui();
				}
			}
		}


		// フィールド
		if (ImGui::CollapsingHeader("Acceleration Fields")) {
			if (fields_.empty()) {
				ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No fields created");
			} else {
				for (auto& [fieldName, field] : fields_) {
					field->ImGui();
				}
			}
		}
		ImGui::TreePop();
	}
#endif
}

bool ParticleSystem::CreateGroup(const std::string& groupName, const std::string& modelTag,
	uint32_t maxParticles, const std::string& textureName, bool useBillboard)
{
	// 同じ名前のグループが既に存在するかチェック
	if (groups_.find(groupName) != groups_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Group '{}' already exists!\n", groupName));
		return false;
	}

	// 新しいグループを作成
	auto group = std::make_unique<ParticleGroup>();
	group->Initialize(directXCommon_, modelTag, maxParticles, textureName, useBillboard);
	group->SetName(groupName);

	// グループを登録
	groups_[groupName] = std::move(group);

	Logger::Log(Logger::GetStream(),
		std::format("ParticleSystem: Created group '{}' (max: {}, model: {}, texture: {})\n",
			groupName, maxParticles, modelTag, textureName.empty() ? "none" : textureName));

	return true;
}

ParticleGroup* ParticleSystem::GetGroup(const std::string& groupName)
{
	auto it = groups_.find(groupName);
	if (it != groups_.end()) {
		return it->second.get();
	}
	return nullptr;
}

void ParticleSystem::RemoveGroup(const std::string& groupName)
{
	auto it = groups_.find(groupName);
	if (it != groups_.end()) {
		groups_.erase(it);
		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Removed group '{}'\n", groupName));
	}
}

ParticleEmitter* ParticleSystem::CreateEmitter(const std::string& emitterName, const std::string& targetGroupName)
{
	// 同じ名前のエミッターが既に存在するかチェック
	if (emitters_.find(emitterName) != emitters_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Emitter '{}' already exists!\n", emitterName));
		return nullptr;
	}

	// ターゲットグループが存在するかチェック
	if (groups_.find(targetGroupName) == groups_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Target group '{}' does not exist!\n", targetGroupName));
		return nullptr;
	}

	// 新しいエミッターを作成
	auto emitter = std::make_unique<ParticleEmitter>();
	emitter->Initialize(directXCommon_, targetGroupName);
	emitter->SetName(emitterName);

	// エミッターを登録
	ParticleEmitter* emitterPtr = emitter.get();
	emitters_[emitterName] = std::move(emitter);

	Logger::Log(Logger::GetStream(),
		std::format("ParticleSystem: Created emitter '{}' targeting group '{}'\n",
			emitterName, targetGroupName));

	return emitterPtr;
}

void ParticleSystem::RemoveEmitter(const std::string& emitterName)
{
	auto it = emitters_.find(emitterName);
	if (it != emitters_.end()) {
		emitters_.erase(it);
		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Removed emitter '{}'\n", emitterName));
	}
}

void ParticleSystem::Clear()
{
	emitters_.clear();
	groups_.clear();
	fields_.clear();
	Logger::Log(Logger::GetStream(), "ParticleSystem: Cleared all groups, emitters, and fields\n");
}

void ParticleSystem::CalculateBillboardMatrix()
{
	// カメラ行列を取得
	Matrix4x4 cameraMatrix = cameraController_->GetCameraMatrix();

	// ビルボード行列を作成
	billboardMatrix_ = cameraMatrix;

	// 平行移動成分を0にする
	billboardMatrix_.m[3][0] = 0.0f;
	billboardMatrix_.m[3][1] = 0.0f;
	billboardMatrix_.m[3][2] = 0.0f;
}

void ParticleSystem::RemoveField(const std::string& fieldName)
{
	auto it = fields_.find(fieldName);
	if (it != fields_.end()) {
		fields_.erase(it);
		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Removed field '{}'\n", fieldName));
	}
}