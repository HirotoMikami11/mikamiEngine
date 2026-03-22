#include "ShapeRecognizer.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <cassert>

// ================================================================
//  チューニング用定数 — ここを調整して精度を上げる 
// ================================================================
namespace Cfg {
	// Step2: 真円度の閾値
	constexpr float CIRCLE_CIRCULARITY_HIGH = 0.75f; // これ以上なら円の強い候補
	constexpr float CIRCLE_CIRCULARITY_LOW = 0.55f; // フォールバック時の円閾値

	// Step3: 曲率分散の閾値 (小さい=均一=円らしい)
	constexpr float CIRCLE_VARIANCE_MAX = 0.018f;

	// Step4: スパイク検出の角度閾値(度)
	constexpr float SPIKE_THRESH_DEG = 18.0f;
	// スパイクのクールダウン幅（近接点を同一コーナーとみなす点数）
	constexpr int   SPIKE_COOLDOWN = 4;

	// Step5: RDP の epsilon = 対角線長 × この係数
	constexpr float RDP_EPSILON_RATIO = 0.06f;
	// △□の角度検証
	constexpr float TRI_ANGLE_MIN = 15.0f;  // 三角の角の最小(度)
	constexpr float TRI_ANGLE_MAX = 150.0f; // 三角の角の最大(度)
	constexpr float TRI_ANGLE_SUM_MIN = 140.0f; // 3角の合計最小
	constexpr float TRI_ANGLE_SUM_MAX = 220.0f; // 3角の合計最大(≈180°)
	constexpr float SQ_ANGLE_MIN = 50.0f;  // 四角の角の最小(度)
	constexpr float SQ_ANGLE_MAX = 140.0f; // 四角の角の最大(度)

	// 閉じているか判定: 始終点距離 / ストローク全長 がこれ以下
	constexpr float CLOSE_RATIO = 0.22f;

	// リサンプリング後の点数
	constexpr int   RESAMPLE_N = 64;
}

#ifdef USEIMGUI
// ================================================================
// Utility
// ================================================================
static constexpr float PI_F = 3.14159265358979323846f;

float ShapeRecognizer::Dist(ImVec2 a, ImVec2 b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

float ShapeRecognizer::PathLength(const std::vector<ImVec2>& pts) {
	float len = 0.f;
	for (size_t i = 1; i < pts.size(); ++i)
		len += Dist(pts[i - 1], pts[i]);
	return len;
}

// ================================================================
// Resample: N点に等間隔リサンプリング
// 点の密度が速度依存するのを解消する
// ================================================================
std::vector<ImVec2> ShapeRecognizer::Resample(const std::vector<ImVec2>& pts, int n) {
	if (pts.empty()) return {};
	float I = PathLength(pts) / float(n - 1);
	if (I < 1e-5f) return pts;

	float D = 0.f;
	std::vector<ImVec2> result;
	result.reserve(n);
	result.push_back(pts[0]);

	// 挿入しながら走査するためコピー
	std::vector<ImVec2> work = pts;
	for (size_t i = 1; i < work.size() && (int)result.size() < n; ++i) {
		float d = Dist(work[i - 1], work[i]);
		if (D + d >= I) {
			float t = (I - D) / d;
			ImVec2 q = {
				work[i - 1].x + t * (work[i].x - work[i - 1].x),
				work[i - 1].y + t * (work[i].y - work[i - 1].y)
			};
			result.push_back(q);
			work.insert(work.begin() + (int)i, q);
			D = 0.f;
		} else {
			D += d;
		}
	}
	// 端数を補完
	while ((int)result.size() < n)
		result.push_back(pts.back());

	return result;
}

// ================================================================
// CheckClosed: 始点〜終点の距離がストローク全長の ratio 以下か
// ================================================================
bool ShapeRecognizer::CheckClosed(const std::vector<ImVec2>& pts, float ratio) {
	float totalLen = PathLength(pts);
	if (totalLen < 1e-5f) return false;
	float endDist = Dist(pts.front(), pts.back());
	return endDist < totalLen * ratio;
}

// ================================================================
// ComputeCircularity: 4π×Area / Perimeter²
// 完全な円なら 1.0、ガタつくほど 0 に近づく
// ================================================================
float ShapeRecognizer::ComputeCircularity(const std::vector<ImVec2>& pts) {
	// Shoelace formula で面積
	float area = 0.f;
	int n = (int)pts.size();
	for (int i = 0; i < n; ++i) {
		int j = (i + 1) % n;
		area += pts[i].x * pts[j].y;
		area -= pts[j].x * pts[i].y;
	}
	area = std::abs(area) * 0.5f;

	// 周長（始終点の距離も足して閉じる）
	float perimeter = PathLength(pts) + Dist(pts.front(), pts.back());
	if (perimeter < 1e-5f) return 0.f;

	return (4.f * PI_F * area) / (perimeter * perimeter);
}

// ================================================================
// ComputeCurvatures: 各点の方向変化角(ラジアン絶対値)
// 円: 全点で均一な小さな値
// 四角: 4箇所で大きくなりそれ以外は0に近い
// ================================================================
std::vector<float> ShapeRecognizer::ComputeCurvatures(const std::vector<ImVec2>& pts) {
	int n = (int)pts.size();
	std::vector<float> curv(n, 0.f);
	for (int i = 1; i < n - 1; ++i) {
		float ax = pts[i].x - pts[i - 1].x, ay = pts[i].y - pts[i - 1].y;
		float bx = pts[i + 1].x - pts[i].x, by = pts[i + 1].y - pts[i].y;
		float a1 = std::atan2(ay, ax);
		float a2 = std::atan2(by, bx);
		float diff = a2 - a1;
		// -π〜π に正規化
		while (diff > PI_F) diff -= 2.f * PI_F;
		while (diff < -PI_F) diff += 2.f * PI_F;
		curv[i] = std::abs(diff);
	}
	return curv;
}

float ShapeRecognizer::ComputeVariance(const std::vector<float>& v) {
	if (v.empty()) return 0.f;
	float mean = std::accumulate(v.begin(), v.end(), 0.f) / float(v.size());
	float var = 0.f;
	for (float x : v) var += (x - mean) * (x - mean);
	return var / float(v.size());
}

// ================================================================
// CountSpikes: 閾値以上の曲率ピークをクラスタリングして数える
// クールダウンで近接する激しい変化を1コーナーとしてまとめる
// ================================================================
int ShapeRecognizer::CountSpikes(const std::vector<float>& curv, float threshDeg) {
	float thresh = threshDeg * PI_F / 180.f;
	int spikes = 0;
	int cooldown = 0;
	for (float c : curv) {
		if (c >= thresh) {
			if (cooldown == 0) spikes++;
			cooldown = Cfg::SPIKE_COOLDOWN;
		} else {
			if (cooldown > 0) --cooldown;
		}
	}
	return spikes;
}

// ================================================================
// RDP (Ramer-Douglas-Peucker): 折れ線を最小点数に簡略化
// ================================================================
static void RDPHelper(const std::vector<ImVec2>& pts, int s, int e,
	float eps, std::vector<bool>& keep) {
	if (e <= s + 1) return;
	float dx = pts[e].x - pts[s].x;
	float dy = pts[e].y - pts[s].y;
	float len = std::sqrt(dx * dx + dy * dy);

	float maxDist = 0.f;
	int   maxIdx = s;
	for (int i = s + 1; i < e; ++i) {
		float dist;
		if (len < 1e-5f) {
			float ex = pts[i].x - pts[s].x, ey = pts[i].y - pts[s].y;
			dist = std::sqrt(ex * ex + ey * ey);
		} else {
			// 直線 s→e への垂直距離
			dist = std::abs(dy * pts[i].x - dx * pts[i].y
				+ pts[e].x * pts[s].y - pts[e].y * pts[s].x) / len;
		}
		if (dist > maxDist) { maxDist = dist; maxIdx = i; }
	}

	if (maxDist > eps) {
		keep[maxIdx] = true;
		RDPHelper(pts, s, maxIdx, eps, keep);
		RDPHelper(pts, maxIdx, e, eps, keep);
	}
}

std::vector<ImVec2> ShapeRecognizer::RDP(const std::vector<ImVec2>& pts, float epsilon) {
	if ((int)pts.size() < 3) return pts;
	int n = (int)pts.size();
	std::vector<bool> keep(n, false);
	keep[0] = keep[n - 1] = true;
	RDPHelper(pts, 0, n - 1, epsilon, keep);
	std::vector<ImVec2> result;
	for (int i = 0; i < n; ++i)
		if (keep[i]) result.push_back(pts[i]);
	return result;
}

// ================================================================
// CornerAngle: 頂点 b における内角(度)
// a→b→c の折れ角を返す
// ================================================================
float ShapeRecognizer::CornerAngle(ImVec2 a, ImVec2 b, ImVec2 c) {
	float ax = a.x - b.x, ay = a.y - b.y;
	float cx = c.x - b.x, cy = c.y - b.y;
	float dot = ax * cx + ay * cy;
	float cross = ax * cy - ay * cx;
	float angle = std::atan2(std::abs(cross), dot) * 180.f / PI_F;
	return angle;
}

// VerifyAsTriangle: 3コーナーの角度が三角形として妥当か
// 各角が [MIN, MAX] かつ 合計 ≈ 180°
bool ShapeRecognizer::VerifyAsTriangle(const std::vector<ImVec2>& corners) {
	if ((int)corners.size() != 3) return false;
	float total = 0.f;
	for (int i = 0; i < 3; ++i) {
		float ang = CornerAngle(corners[(i + 2) % 3], corners[i], corners[(i + 1) % 3]);
		if (ang < Cfg::TRI_ANGLE_MIN || ang > Cfg::TRI_ANGLE_MAX) return false;
		total += ang;
	}
	return total > Cfg::TRI_ANGLE_SUM_MIN && total < Cfg::TRI_ANGLE_SUM_MAX;
}

// VerifyAsSquare: 4コーナーの角度が四角形として妥当か
// 各角が [MIN, MAX] (≈90°の許容範囲)
bool ShapeRecognizer::VerifyAsSquare(const std::vector<ImVec2>& corners) {
	if ((int)corners.size() != 4) return false;
	for (int i = 0; i < 4; ++i) {
		float ang = CornerAngle(corners[(i + 3) % 4], corners[i], corners[(i + 1) % 4]);
		if (ang < Cfg::SQ_ANGLE_MIN || ang > Cfg::SQ_ANGLE_MAX) return false;
	}
	return true;
}

// ================================================================
// Recognize メイン
// ================================================================
RecognitionResult ShapeRecognizer::Recognize(const std::vector<ImVec2>& rawPoints) {
	RecognitionResult result;
	if ((int)rawPoints.size() < 10) return result; // 点が少なすぎ

	// ---- 前処理: 等間隔リサンプリング ----
	auto pts = Resample(rawPoints, Cfg::RESAMPLE_N);

	// ---- Step 1: 閉じているか ----
	result.isClosed = CheckClosed(rawPoints, Cfg::CLOSE_RATIO);

	// ---- Step 2: 真円度 ----
	result.circularity = ComputeCircularity(pts);

	// ---- Step 3: 曲率分散 ----
	auto curvatures = ComputeCurvatures(pts);
	result.curvVariance = ComputeVariance(curvatures);

	// ---- Step 4: スパイク数 ----
	result.spikeCount = CountSpikes(curvatures, Cfg::SPIKE_THRESH_DEG);

	// ---- Step 5: RDP 簡略化 ----
	// epsilon はバウンディングボックス対角線長の相対値で設定
	float minX = pts[0].x, maxX = minX, minY = pts[0].y, maxY = minY;
	for (auto& p : pts) {
		minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
		minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
	}
	float diagLen = std::sqrt((maxX - minX) * (maxX - minX) + (maxY - minY) * (maxY - minY));
	float epsilon = diagLen * Cfg::RDP_EPSILON_RATIO;
	auto simplified = RDP(pts, epsilon);
	result.rdpCornerCount = (int)simplified.size();

	// ================================================================
	// 判定ロジック
	// ================================================================

	// [A] 円判定: 真円度が高く 曲率が均一
	//   → 四角・三角はここでブロックされる
	if (result.circularity > Cfg::CIRCLE_CIRCULARITY_HIGH
		&& result.curvVariance < Cfg::CIRCLE_VARIANCE_MAX) {
		result.shape = ShapeType::Circle;
		return result;
	}

	// 閉じていない → 不明
	if (!result.isClosed) {
		result.shape = ShapeType::Unknown;
		return result;
	}

	// [B] RDP の角数 + 角度検証で三角/四角を確定する
	//     ここが「角丸四角を三角に誤判定しない」ための核心

	// simplified が3点 → 三角検証
	if ((int)simplified.size() == 3) {
		if (VerifyAsTriangle(simplified)) {
			result.shape = ShapeType::Triangle;
			return result;
		}
		// 3点だが角度が三角形でない = 角丸四角の角1つが吸収された
		// → 四角として扱う (スパイク数でさらに確認)
		if (result.spikeCount >= 3) {
			result.shape = ShapeType::Square;
			return result;
		}
	}

	// simplified が4点 → 四角検証
	if ((int)simplified.size() == 4) {
		if (VerifyAsSquare(simplified)) {
			result.shape = ShapeType::Square;
			return result;
		}
		// 4点だが四角でない → epsを上げて3点に絞れれば三角
		auto s2 = RDP(simplified, epsilon * 2.5f);
		if ((int)s2.size() == 3 && VerifyAsTriangle(s2)) {
			result.shape = ShapeType::Triangle;
			return result;
		}
	}

	// simplified が5点以上 → epsを上げて再試行
	if ((int)simplified.size() >= 5) {
		auto s3 = RDP(pts, epsilon * 2.f);
		if ((int)s3.size() == 4 && VerifyAsSquare(s3)) {
			result.shape = ShapeType::Square;
			return result;
		}
		auto s4 = RDP(pts, epsilon * 3.5f);
		if ((int)s4.size() == 3 && VerifyAsTriangle(s4)) {
			result.shape = ShapeType::Triangle;
			return result;
		}
	}

	// [C] フォールバック: RDP が決定打にならなかった場合はスパイク数で判断
	if (result.spikeCount <= 1) {
		// スパイクがほぼない = 滑らか = 円
		if (result.circularity > Cfg::CIRCLE_CIRCULARITY_LOW) {
			result.shape = ShapeType::Circle;
			return result;
		}
	}
	if (result.spikeCount == 3) {
		result.shape = ShapeType::Triangle;
		return result;
	}
	if (result.spikeCount == 4) {
		result.shape = ShapeType::Square;
		return result;
	}

	// 判定不能
	result.shape = ShapeType::Unknown;
	return result;
}

#endif