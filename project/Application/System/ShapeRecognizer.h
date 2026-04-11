#pragma once
#include <vector>
#include "imgui.h"

// 判定結果
enum class ShapeType {
	Unknown,
	Circle,			// ○
	Triangle,		// △
	Square			// □
};

#ifdef USEIMGUI
struct RecognitionResult {
	ShapeType shape = ShapeType::Unknown;

	// --- デバッグ表示用の中間指標 ---
	float circularity = 0.f;  // 真円度 (0〜1, 円=1.0)
	float curvVariance = 0.f;  // 曲率分散 (小=均一=円らしい)
	int   spikeCount = 0;    // 方向転換スパイク数
	int   rdpCornerCount = 0;    // RDP簡略化後の点数
	bool  isClosed = false;// 閉じているか

	const char* GetShapeName() const {
		switch (shape) {
		case ShapeType::Circle:   return "○  Circle";
		case ShapeType::Triangle: return "△  Triangle";
		case ShapeType::Square:   return "□  Square";
		default:                  return "?  Unknown";
		}
	}
};

// ShapeRecognizer
// ストロークの ImVec2 点列を受け取り図形を判定する
class ShapeRecognizer {
public:
	static RecognitionResult Recognize(const std::vector<ImVec2>& rawPoints);

private:
	//前処理
	static std::vector<ImVec2> Resample(const std::vector<ImVec2>& pts, int n);
	static float PathLength(const std::vector<ImVec2>& pts);
	static float Dist(ImVec2 a, ImVec2 b);

	//特徴計算
	static bool  CheckClosed(const std::vector<ImVec2>& pts, float ratio);
	static float ComputeCircularity(const std::vector<ImVec2>& pts);
	static std::vector<float> ComputeCurvatures(const std::vector<ImVec2>& pts);
	static float ComputeVariance(const std::vector<float>& v);
	static int   CountSpikes(const std::vector<float>& curv, float threshDeg);

	//ポリゴン簡略化・角度検証
	static std::vector<ImVec2> RDP(const std::vector<ImVec2>& pts, float epsilon);
	static float CornerAngle(ImVec2 a, ImVec2 b, ImVec2 c); // b点での内角(度)
	static bool  VerifyAsTriangle(const std::vector<ImVec2>& corners);
	static bool  VerifyAsSquare(const std::vector<ImVec2>& corners);
};

#endif