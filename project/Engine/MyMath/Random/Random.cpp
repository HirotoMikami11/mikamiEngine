#include "Random.h"
Random::Random() : randomEngine_(seedGenerator_()) {
	//シードの設定完了してる
}

Random& Random::GetInstance() {
	static Random instance;
	return instance;
}

float Random::GenerateFloat(float min, float max) {
	std::lock_guard<std::mutex> lock(mutex_);
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine_);
}

int Random::GenerateInt(int min, int max) {
	std::lock_guard<std::mutex> lock(mutex_);
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine_);
}

float Random::GenerateNormalized() {
	return GenerateFloat(0.0f, 1.0f);
}

float Random::GenerateFloatWithOffset(float baseValue, float offset) {
	return GenerateFloat(baseValue - offset, baseValue + offset);
}

Vector3 Random::GenerateVector3WithOffset(const Vector3& baseVector, float offset) {
	return Vector3{
		GenerateFloatWithOffset(baseVector.x, offset),
		GenerateFloatWithOffset(baseVector.y, offset),
		GenerateFloatWithOffset(baseVector.z, offset)
	};
}

Vector3 Random::GenerateVector3WithOffset(const Vector3& baseVector, const Vector3& offsetVector) {
	return Vector3{
		GenerateFloatWithOffset(baseVector.x, offsetVector.x),
		GenerateFloatWithOffset(baseVector.y, offsetVector.y),
		GenerateFloatWithOffset(baseVector.z, offsetVector.z)
	};
}

Vector3 Random::GenerateVector3OriginOffset(float offset)
{
	return Vector3{
		GenerateFloatWithOffset(0.0f, offset),
		GenerateFloatWithOffset(0.0f, offset),
		GenerateFloatWithOffset(0.0f, offset)
	};
}

Vector4 Random::GenerateVector4WithOffset(const Vector4& baseColor, float offset) {
	return Vector4{
		GenerateFloatWithOffset(baseColor.x, offset),
		GenerateFloatWithOffset(baseColor.y, offset),
		GenerateFloatWithOffset(baseColor.z, offset),
		baseColor.w  // A値は維持
	};
}

Vector4 Random::GenerateRandomVector4(float alpha) {
	return Vector4{
		GenerateNormalized(),
		GenerateNormalized(),
		GenerateNormalized(),
		alpha  // 指定されたA値を使用
	};
}