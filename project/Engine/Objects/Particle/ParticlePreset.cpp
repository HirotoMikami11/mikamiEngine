#include "ParticlePreset.h"
#include "Logger.h"
#include <fstream>
#include <format>

// ========================================
// ParticleGroupData
// ========================================

json ParticleGroupData::ToJson() const
{
	return json{
		{"groupName", groupName},
		{"modelTag", modelTag},
		{"maxParticles", maxParticles},
		{"textureName", textureName},
		{"useBillboard", useBillboard}
	};
}

ParticleGroupData ParticleGroupData::FromJson(const json& j)
{
	ParticleGroupData data;
	data.groupName = j.value("groupName", "");
	data.modelTag = j.value("modelTag", "");
	data.maxParticles = j.value("maxParticles", 100u);
	data.textureName = j.value("textureName", "");
	data.useBillboard = j.value("useBillboard", true);
	return data;
}

// ========================================
// ParticleEmitterData
// ========================================

json ParticleEmitterData::ToJson() const
{
	return json{
		{"emitterName", emitterName},
		{"targetGroupName", targetGroupName},
		{"position", {position.x, position.y, position.z}},
		{"rotation", {rotation.x, rotation.y, rotation.z}},
		{"scale", {scale.x, scale.y, scale.z}},
		{"emitCount", emitCount},
		{"emitFrequency", emitFrequency},
		{"isEmitting", isEmitting},
		{"particleLifeTimeMin", particleLifeTimeMin},
		{"particleLifeTimeMax", particleLifeTimeMax},
		{"emitDirection", {emitDirection.x, emitDirection.y, emitDirection.z}},
		{"initialSpeed", initialSpeed},
		{"spreadAngle", spreadAngle},
		{"useDirectionalEmit", useDirectionalEmit},
		{"velocityRange", velocityRange},
		{"particleScaleMin", {particleScaleMin.x, particleScaleMin.y, particleScaleMin.z}},
		{"particleScaleMax", {particleScaleMax.x, particleScaleMax.y, particleScaleMax.z}},
		{"particleRotateMin", {particleRotateMin.x, particleRotateMin.y, particleRotateMin.z}},
		{"particleRotateMax", {particleRotateMax.x, particleRotateMax.y, particleRotateMax.z}},
		{"emitterLifeTime", emitterLifeTime},
		{"emitterLifeTimeLoop", emitterLifeTimeLoop},
		{"useEmitterLifeTime", useEmitterLifeTime},
		{"spawnArea", {
			{"min", {spawnArea.min.x, spawnArea.min.y, spawnArea.min.z}},
			{"max", {spawnArea.max.x, spawnArea.max.y, spawnArea.max.z}}
		}},
		{"showDebugAABB", showDebugAABB},
		{"debugAABBColor", {debugAABBColor.x, debugAABBColor.y, debugAABBColor.z, debugAABBColor.w}},

		// Color Over Lifetime
		{"enableColorOverLifetime", enableColorOverLifetime},
		{"particleStartColor", {particleStartColor.x, particleStartColor.y, particleStartColor.z, particleStartColor.w}},
		{"particleEndColor", {particleEndColor.x, particleEndColor.y, particleEndColor.z, particleEndColor.w}},

		// Size Over Lifetime
		{"enableSizeOverLifetime", enableSizeOverLifetime},
		{"particleStartScale", {particleStartScale.x, particleStartScale.y, particleStartScale.z}},
		{"particleEndScale", {particleEndScale.x, particleEndScale.y, particleEndScale.z}},

		// Rotation
		{"enableRotation", enableRotation},
		{"rotationSpeed", {rotationSpeed.x, rotationSpeed.y, rotationSpeed.z}}
	};
}

ParticleEmitterData ParticleEmitterData::FromJson(const json& j)
{
	ParticleEmitterData data;
	data.emitterName = j.value("emitterName", "");
	data.targetGroupName = j.value("targetGroupName", "");

	auto pos = j.value("position", std::vector<float>{0, 0, 0});
	data.position = { pos[0], pos[1], pos[2] };

	auto rot = j.value("rotation", std::vector<float>{0, 0, 0});
	data.rotation = { rot[0], rot[1], rot[2] };

	auto scl = j.value("scale", std::vector<float>{1, 1, 1});
	data.scale = { scl[0], scl[1], scl[2] };

	data.emitCount = j.value("emitCount", 5u);
	data.emitFrequency = j.value("emitFrequency", 1.0f);
	data.isEmitting = j.value("isEmitting", true);

	data.particleLifeTimeMin = j.value("particleLifeTimeMin", 1.0f);
	data.particleLifeTimeMax = j.value("particleLifeTimeMax", 3.0f);

	auto emitDir = j.value("emitDirection", std::vector<float>{0, 1, 0});
	data.emitDirection = { emitDir[0], emitDir[1], emitDir[2] };

	data.initialSpeed = j.value("initialSpeed", 1.0f);
	data.spreadAngle = j.value("spreadAngle", 30.0f);
	data.useDirectionalEmit = j.value("useDirectionalEmit", false);
	data.velocityRange = j.value("velocityRange", 1.0f);

	auto scaleMin = j.value("particleScaleMin", std::vector<float>{1, 1, 1});
	data.particleScaleMin = { scaleMin[0], scaleMin[1], scaleMin[2] };

	auto scaleMax = j.value("particleScaleMax", std::vector<float>{1, 1, 1});
	data.particleScaleMax = { scaleMax[0], scaleMax[1], scaleMax[2] };

	auto rotMin = j.value("particleRotateMin", std::vector<float>{0, 0, 0});
	data.particleRotateMin = { rotMin[0], rotMin[1], rotMin[2] };

	auto rotMax = j.value("particleRotateMax", std::vector<float>{0, 0, 0});
	data.particleRotateMax = { rotMax[0], rotMax[1], rotMax[2] };

	data.emitterLifeTime = j.value("emitterLifeTime", 5.0f);
	data.emitterLifeTimeLoop = j.value("emitterLifeTimeLoop", false);
	data.useEmitterLifeTime = j.value("useEmitterLifeTime", false);

	if (j.contains("spawnArea")) {
		auto aabb = j["spawnArea"];
		auto minVec = aabb.value("min", std::vector<float>{-0.5f, -0.5f, -0.5f});
		auto maxVec = aabb.value("max", std::vector<float>{0.5f, 0.5f, 0.5f});
		data.spawnArea.min = { minVec[0], minVec[1], minVec[2] };
		data.spawnArea.max = { maxVec[0], maxVec[1], maxVec[2] };
	}

	data.showDebugAABB = j.value("showDebugAABB", false);

	auto color = j.value("debugAABBColor", std::vector<float>{1, 0, 0, 1});
	auto debugColor = j.value("debugAABBColor", std::vector<float>{1, 0, 0, 1});
	data.debugAABBColor = { debugColor[0], debugColor[1], debugColor[2], debugColor[3] };

	// Color Over Lifetime
	data.enableColorOverLifetime = j.value("enableColorOverLifetime", false);

	auto startColor = j.value("particleStartColor", std::vector<float>{1, 1, 1, 1});
	data.particleStartColor = { startColor[0], startColor[1], startColor[2], startColor[3] };

	auto endColor = j.value("particleEndColor", std::vector<float>{1, 1, 1, 0});
	data.particleEndColor = { endColor[0], endColor[1], endColor[2], endColor[3] };

	// Size Over Lifetime
	data.enableSizeOverLifetime = j.value("enableSizeOverLifetime", false);

	auto startScale = j.value("particleStartScale", std::vector<float>{1, 1, 1});
	data.particleStartScale = { startScale[0], startScale[1], startScale[2] };

	auto endScale = j.value("particleEndScale", std::vector<float>{1, 1, 1});
	data.particleEndScale = { endScale[0], endScale[1], endScale[2] };

	// Rotation
	data.enableRotation = j.value("enableRotation", false);

	auto rotSpeed = j.value("rotationSpeed", std::vector<float>{0, 0, 0});
	data.rotationSpeed = { rotSpeed[0], rotSpeed[1], rotSpeed[2] };

	return data;
}

// ========================================
// ParticleFieldData
// ========================================

json ParticleFieldData::ToJson() const
{
	return json{
		{"fieldName", fieldName},
		{"fieldType", fieldType},
		{"position", {position.x, position.y, position.z}},
		{"rotation", {rotation.x, rotation.y, rotation.z}},
		{"scale", {scale.x, scale.y, scale.z}},
		{"isEnabled", isEnabled},
		{"showDebugVisualization", showDebugVisualization},
		{"debugColor", {debugColor.x, debugColor.y, debugColor.z, debugColor.w}},
		{"parameters", parameters}
	};
}

ParticleFieldData ParticleFieldData::FromJson(const json& j)
{
	ParticleFieldData data;
	data.fieldName = j.value("fieldName", "");
	data.fieldType = j.value("fieldType", "");

	auto pos = j.value("position", std::vector<float>{0, 0, 0});
	data.position = { pos[0], pos[1], pos[2] };

	auto rot = j.value("rotation", std::vector<float>{0, 0, 0});
	data.rotation = { rot[0], rot[1], rot[2] };

	auto scl = j.value("scale", std::vector<float>{1, 1, 1});
	data.scale = { scl[0], scl[1], scl[2] };

	data.isEnabled = j.value("isEnabled", true);
	data.showDebugVisualization = j.value("showDebugVisualization", false);

	auto color = j.value("debugColor", std::vector<float>{1, 1, 1, 1});
	data.debugColor = { color[0], color[1], color[2], color[3] };

	data.parameters = j.value("parameters", json::object());

	return data;
}

// ========================================
// ParticlePresetData
// ========================================

json ParticlePresetData::ToJson() const
{
	json j;
	j["presetName"] = presetName;

	j["groups"] = json::array();
	for (const auto& group : groups) {
		j["groups"].push_back(group.ToJson());
	}

	j["emitters"] = json::array();
	for (const auto& emitter : emitters) {
		j["emitters"].push_back(emitter.ToJson());
	}

	j["fields"] = json::array();
	for (const auto& field : fields) {
		j["fields"].push_back(field.ToJson());
	}

	return j;
}

ParticlePresetData ParticlePresetData::FromJson(const json& j)
{
	ParticlePresetData data;
	data.presetName = j.value("presetName", "");

	if (j.contains("groups")) {
		for (const auto& groupJson : j["groups"]) {
			data.groups.push_back(ParticleGroupData::FromJson(groupJson));
		}
	}

	if (j.contains("emitters")) {
		for (const auto& emitterJson : j["emitters"]) {
			data.emitters.push_back(ParticleEmitterData::FromJson(emitterJson));
		}
	}

	if (j.contains("fields")) {
		for (const auto& fieldJson : j["fields"]) {
			data.fields.push_back(ParticleFieldData::FromJson(fieldJson));
		}
	}

	return data;
}

bool ParticlePresetData::SaveToFile(const std::string& filePath) const
{
	try {
		json j = ToJson();
		std::ofstream ofs(filePath);
		if (ofs.fail()) {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticlePreset] Failed to open file for saving: {}\n", filePath));
			return false;
		}

		ofs << std::setw(4) << j << std::endl;
		ofs.close();

		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePreset] Successfully saved preset '{}' to: {}\n",
				presetName, filePath));
		return true;

	} catch (const std::exception& e) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePreset] Exception while saving: {}\n", e.what()));
		return false;
	}
}

ParticlePresetData ParticlePresetData::LoadFromFile(const std::string& filePath)
{
	try {
		std::ifstream ifs(filePath);
		if (ifs.fail()) {
			Logger::Log(Logger::GetStream(),
				std::format("[ParticlePreset] Failed to open file for loading: {}\n", filePath));
			return ParticlePresetData{};
		}

		json j;
		ifs >> j;
		ifs.close();

		ParticlePresetData data = FromJson(j);
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePreset] Successfully loaded preset '{}' from: {}\n",
				data.presetName, filePath));

		return data;

	} catch (const std::exception& e) {
		Logger::Log(Logger::GetStream(),
			std::format("[ParticlePreset] Exception while loading: {}\n", e.what()));
		return ParticlePresetData{};
	}
}