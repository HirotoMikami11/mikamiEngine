#include "ParticlePresetInstance.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleGroup.h"
#include "BaseField.h"
#include "Logger.h"
#include <format>

void ParticlePresetInstance::Initialize(const std::string& instanceName, ParticleSystem* particleSystem)
{
	instanceName_ = instanceName;
	particleSystem_ = particleSystem;
	isDestroyed_ = false;
}

ParticleEmitter* ParticlePresetInstance::GetEmitter(const std::string& localName)
{
	if (isDestroyed_) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePresetInstance] Cannot get emitter from destroyed instance '{}'\n", instanceName_));
		return nullptr;
	}

	auto it = emitterNameMap_.find(localName);
	if (it == emitterNameMap_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePresetInstance] Emitter '{}' not found in instance '{}'\n",
				localName, instanceName_));
		return nullptr;
	}

	return particleSystem_->GetEmitter(it->second);
}

ParticleGroup* ParticlePresetInstance::GetGroup(const std::string& localName)
{
	if (isDestroyed_) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePresetInstance] Cannot get group from destroyed instance '{}'\n", instanceName_));
		return nullptr;
	}

	auto it = groupNameMap_.find(localName);
	if (it == groupNameMap_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePresetInstance] Group '{}' not found in instance '{}'\n",
				localName, instanceName_));
		return nullptr;
	}

	return particleSystem_->GetGroup(it->second);
}

BaseField* ParticlePresetInstance::GetField(const std::string& localName)
{
	if (isDestroyed_) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePresetInstance] Cannot get field from destroyed instance '{}'\n", instanceName_));
		return nullptr;
	}

	auto it = fieldNameMap_.find(localName);
	if (it == fieldNameMap_.end()) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePresetInstance] Field '{}' not found in instance '{}'\n",
				localName, instanceName_));
		return nullptr;
	}

	return particleSystem_->GetField(it->second);
}

void ParticlePresetInstance::SetPositionOffset(const Vector3& offset)
{
	if (isDestroyed_) {
		return;
	}

	// 全エミッターの座標を設定
	for (const auto& [localName, uniqueName] : emitterNameMap_) {
		ParticleEmitter* emitter = particleSystem_->GetEmitter(uniqueName);
		if (emitter) {
			emitter->GetTransform().SetPosition(offset);
		}
	}

	// 全フィールドの座標を設定
	for (const auto& [localName, uniqueName] : fieldNameMap_) {
		BaseField* field = particleSystem_->GetField(uniqueName);
		if (field) {
			field->GetTransform().SetPosition(offset);
		}
	}
}

void ParticlePresetInstance::SetEnabled(bool enabled)
{
	if (isDestroyed_) {
		return;
	}

	// 全エミッターの有効/無効を設定
	for (const auto& [localName, uniqueName] : emitterNameMap_) {
		ParticleEmitter* emitter = particleSystem_->GetEmitter(uniqueName);
		if (emitter) {
			emitter->SetEmitEnabled(enabled);
		}
	}

	// 全フィールドの有効/無効を設定
	for (const auto& [localName, uniqueName] : fieldNameMap_) {
		BaseField* field = particleSystem_->GetField(uniqueName);
		if (field) {
			field->SetEnabled(enabled);
		}
	}
}

void ParticlePresetInstance::Destroy()
{
	if (isDestroyed_) {
		return;
	}

	// すべてのエミッターを削除
	for (const auto& [localName, uniqueName] : emitterNameMap_) {
		particleSystem_->RemoveEmitter(uniqueName);
	}

	// すべてのフィールドを削除
	for (const auto& [localName, uniqueName] : fieldNameMap_) {
		particleSystem_->RemoveField(uniqueName);
	}

	// すべてのグループを削除
	for (const auto& [localName, uniqueName] : groupNameMap_) {
		particleSystem_->RemoveGroup(uniqueName);
	}

	groupNameMap_.clear();
	emitterNameMap_.clear();
	fieldNameMap_.clear();

	isDestroyed_ = true;

	Logger::Log(Logger::GetStream(),
		std::format("[ParticlePresetInstance] Destroyed instance '{}'\n", instanceName_));
}

void ParticlePresetInstance::RegisterGroup(const std::string& localName, const std::string& uniqueName)
{
	groupNameMap_[localName] = uniqueName;
}

void ParticlePresetInstance::RegisterEmitter(const std::string& localName, const std::string& uniqueName)
{
	emitterNameMap_[localName] = uniqueName;
}

void ParticlePresetInstance::RegisterField(const std::string& localName, const std::string& uniqueName)
{
	fieldNameMap_[localName] = uniqueName;
}