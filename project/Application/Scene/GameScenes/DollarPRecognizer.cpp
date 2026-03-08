#include "DollarPRecognizer.h"
#include <cmath>
#include <algorithm>
#include <limits>

static constexpr float PI_F = 3.14159265358979323846f;

// ================================================================
// GetInstance / コンストラクタ
// ================================================================
DollarPRecognizer* DollarPRecognizer::GetInstance() {
	static DollarPRecognizer instance;
	return &instance;
}

// デフォルトテンプレートを登録。
// $P は書き順/向き不問なので 1 種類のテンプレートで原則十分だが、
// 縦長・横長など描き方のバリアントをカバーするため各図形 2 バリアント登録する。
DollarPRecognizer::DollarPRecognizer() {
	// 丸（2 バリアント: 基本 + やや細い楕円寄り）
	AddTemplate("circle", MakeCircleTemplate());

	// 三角形（2 バリアント: 等辺三角形 + やや細長い三角形）
	AddTemplate("triangle", MakeTriangleTemplate());

	// 四角形（2 バリアント: 正方形 + やや横長）
	AddTemplate("square", MakeSquareTemplate());
}

// ================================================================
// テンプレート生成
// ================================================================

// まる: 中心から半径 r の円。書き順不問なので反時計回りのみ定義。
std::vector<ImVec2> DollarPRecognizer::MakeCircleTemplate() {
	std::vector<ImVec2> pts;
	int   n = 96;
	float r = 110.f;
	for (int i = 0; i < n; i++) {
		float t = 2.f * PI_F * float(i) / float(n);
		pts.push_back({ r * std::cos(t), r * std::sin(t) });
	}
	return pts;
}

// △: 正三角形（重心が原点）
std::vector<ImVec2> DollarPRecognizer::MakeTriangleTemplate() {
	float h = 120.f;
	float top_y = -(h * 2.f / 3.f);
	float bot_y = (h * 1.f / 3.f);
	float half_w = h / std::sqrt(3.f);

	ImVec2 A = { 0.f,    top_y };	// 上
	ImVec2 B = { -half_w, bot_y };	// 左下
	ImVec2 C = { half_w, bot_y };	// 右下

	std::vector<ImVec2> pts;
	int seg = 32;
	auto addSeg = [&](ImVec2 from, ImVec2 to) {
		for (int i = 0; i <= seg; i++) {
			float t = float(i) / float(seg);
			pts.push_back({ from.x + (to.x - from.x) * t,
							from.y + (to.y - from.y) * t });
		}
		};
	addSeg(A, B);
	addSeg(B, C);
	addSeg(C, A);
	return pts;
}

// □: 正方形（中心が原点）
std::vector<ImVec2> DollarPRecognizer::MakeSquareTemplate() {
	float s = 100.f;
	ImVec2 TL = { -s, -s };
	ImVec2 TR = { s, -s };
	ImVec2 BR = { s,  s };
	ImVec2 BL = { -s,  s };

	std::vector<ImVec2> pts;
	int seg = 24;
	auto addSeg = [&](ImVec2 from, ImVec2 to) {
		for (int i = 0; i <= seg; i++) {
			float t = float(i) / float(seg);
			pts.push_back({ from.x + (to.x - from.x) * t,
							from.y + (to.y - from.y) * t });
		}
		};
	addSeg(TL, TR);
	addSeg(TR, BR);
	addSeg(BR, BL);
	addSeg(BL, TL);
	return pts;
}

// ================================================================
// AddTemplate
// ================================================================
void DollarPRecognizer::AddTemplate(const std::string& name,
	const std::vector<ImVec2>& points)
{
	DollarTemplate tmpl;
	tmpl.name = name;
	tmpl.points = Normalize(points);
	templates_.push_back(tmpl);
}

// ================================================================
// Normalize（$P 版: 回転ステップなし）
//   Resample → ScaleToSquare → TranslateToCentroid
//   IndicativeAngle 回転を行わないことで書き順・向きを無視する
// ================================================================
std::vector<ImVec2> DollarPRecognizer::Normalize(const std::vector<ImVec2>& pts) {
	auto r = Resample(pts, NUM_POINTS);
	r = ScaleToSquare(r, SQUARE_SIZE);
	r = TranslateToCentroid(r);
	return r;
}

// ================================================================
// $P コア: GreedyCloudMatch
//
// 論文の Greedy-5 に準拠。
// step = floor(n^(1-eps)) = floor(sqrt(n)) ≈ 5 (n=32 の場合)
// その step ごとに開始点を変えて CloudDistance を計算し最小値を返す。
// CloudDistance を pts→tmpl と tmpl→pts の両方向で評価することで
// 一方向の偏りを防ぐ。
// ================================================================
float DollarPRecognizer::GreedyCloudMatch(
	const std::vector<ImVec2>& pts,
	const std::vector<ImVec2>& tmpl)
{
	int   n = (int)pts.size();
	float eps = 0.5f;
	// step = floor(n^(1 - eps)) = floor(sqrt(n))
	int   step = std::max(1, (int)std::floor(std::pow((float)n, 1.f - eps)));
	float minD = std::numeric_limits<float>::max();

	for (int i = 0; i < n; i += step) {
		float d1 = CloudDistance(pts, tmpl, i);
		float d2 = CloudDistance(tmpl, pts, i);
		minD = std::min(minD, std::min(d1, d2));
	}
	return minD;
}

// ================================================================
// $P コア: CloudDistance
//
// start を起点として pts を順番に処理。
// 各 pts[i] に対し tmpl の未割当点で最も近いものを貪欲に選び、
// (1 - i/n) の重みを掛けた距離を累積する。
// 重みにより「開始点付近の一致度」を重視するが、
// 開始点を複数試すことで書き順依存を排除する。
// ================================================================
float DollarPRecognizer::CloudDistance(
	const std::vector<ImVec2>& pts,
	const std::vector<ImVec2>& tmpl,
	int start)
{
	int n = (int)pts.size();
	std::vector<bool> matched(n, false);
	float sum = 0.f;
	int   i = start;

	do {
		float minDist = std::numeric_limits<float>::max();
		int   index = -1;
		for (int j = 0; j < n; j++) {
			if (!matched[j]) {
				float d = Dist(pts[i], tmpl[j]);
				if (d < minDist) {
					minDist = d;
					index = j;
				}
			}
		}
		if (index >= 0) {
			matched[index] = true;
			// weight: 開始点ほど大きい（1.0 → 1/n に線形減少）
			float weight = 1.f - float((i - start + n) % n) / float(n);
			sum += weight * minDist;
		}
		i = (i + 1) % n;
	} while (i != start);

	return sum;
}

// ================================================================
// Recognize
// ================================================================
DollarResult DollarPRecognizer::Recognize(const std::vector<ImVec2>& rawPoints) const {
	DollarResult result;
	result.matched = false;
	result.score = 0.f;
	result.circularity = 0.f;
	result.byCircularity = false;

	if ((int)rawPoints.size() < 10) return result;

	// ---- 前段: 真円度チェック ----
	// 正規化前の生点列で計算することで $P のスケール正規化の影響を受けない
	float circ = ComputeCircularity(rawPoints);
	result.circularity = circ;

	if (circ >= CIRCLE_CIRCULARITY_THRESH) {
		result.name = "circle";
		result.score = circ;
		result.matched = true;
		result.byCircularity = true;
		return result;
	}

	// ---- 後段: $P GreedyCloudMatch ----
	auto  pts = Normalize(rawPoints);
	// スコアの正規化基準: 250×250 の正方形の半対角線
	float halfDiag = 0.5f * std::sqrt(2.f) * SQUARE_SIZE;

	float       bestScore = -1.f;
	std::string bestName;

	for (const auto& tmpl : templates_) {
		float dist = GreedyCloudMatch(pts, tmpl.points);
		float score = std::max(0.f, 1.f - dist / halfDiag);
		if (score > bestScore) {
			bestScore = score;
			bestName = tmpl.name;
		}
	}

	result.name = bestName;
	result.score = bestScore;
	result.matched = (bestScore >= MATCH_THRESH);
	return result;
}

// ================================================================
// 真円度: 4π × 面積 / 周長²
//   完全な円 = 1.0、正方形 ≈ 0.785、正三角形 ≈ 0.605
// ================================================================
float DollarPRecognizer::ComputeCircularity(const std::vector<ImVec2>& pts) {
	if (pts.size() < 3) return 0.f;

	float perimeter = PathLength(pts);
	perimeter += Dist(pts.back(), pts.front()); // ストローク未閉合分を補完
	if (perimeter < 1e-6f) return 0.f;

	float area = 0.f;
	int   n = (int)pts.size();
	for (int i = 0; i < n; i++) {
		const ImVec2& a = pts[i];
		const ImVec2& b = pts[(i + 1) % n];
		area += a.x * b.y - b.x * a.y;
	}
	area = std::abs(area) * 0.5f;

	return (4.f * PI_F * area) / (perimeter * perimeter);
}

// ================================================================
// 共通ユーティリティ
// ================================================================
float DollarPRecognizer::Dist(ImVec2 a, ImVec2 b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

float DollarPRecognizer::PathLength(const std::vector<ImVec2>& pts) {
	float len = 0.f;
	for (size_t i = 1; i < pts.size(); ++i) len += Dist(pts[i - 1], pts[i]);
	return len;
}

ImVec2 DollarPRecognizer::Centroid(const std::vector<ImVec2>& pts) {
	float sx = 0.f, sy = 0.f;
	for (auto& p : pts) { sx += p.x; sy += p.y; }
	return { sx / float(pts.size()), sy / float(pts.size()) };
}

std::vector<ImVec2> DollarPRecognizer::Resample(const std::vector<ImVec2>& pts, int n) {
	if (pts.empty()) return {};
	float I = PathLength(pts) / float(n - 1);
	if (I < 1e-6f) return std::vector<ImVec2>(n, pts[0]);

	float D = 0.f;
	std::vector<ImVec2> result;
	result.reserve(n);
	result.push_back(pts[0]);

	std::vector<ImVec2> work = pts;
	for (size_t i = 1; i < work.size() && (int)result.size() < n; ++i) {
		float d = Dist(work[i - 1], work[i]);
		if (D + d >= I) {
			float t = (I - D) / d;
			ImVec2 q = { work[i - 1].x + t * (work[i].x - work[i - 1].x),
						 work[i - 1].y + t * (work[i].y - work[i - 1].y) };
			result.push_back(q);
			work.insert(work.begin() + (int)i, q);
			D = 0.f;
		} else {
			D += d;
		}
	}
	while ((int)result.size() < n) result.push_back(pts.back());
	return result;
}

std::vector<ImVec2> DollarPRecognizer::ScaleToSquare(
	const std::vector<ImVec2>& pts, float size)
{
	float minX = pts[0].x, maxX = minX;
	float minY = pts[0].y, maxY = minY;
	for (auto& p : pts) {
		minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
		minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
	}
	float w = maxX - minX, h = maxY - minY;
	float scale = (w > 1e-6f && h > 1e-6f) ? std::min(size / w, size / h) : 1.f;

	std::vector<ImVec2> result;
	result.reserve(pts.size());
	for (auto& p : pts) result.push_back({ p.x * scale, p.y * scale });
	return result;
}

std::vector<ImVec2> DollarPRecognizer::TranslateToCentroid(
	const std::vector<ImVec2>& pts)
{
	ImVec2 c = Centroid(pts);
	std::vector<ImVec2> result;
	result.reserve(pts.size());
	for (auto& p : pts) result.push_back({ p.x - c.x, p.y - c.y });
	return result;
}

// ================================================================
// StrokeGuide
// $P に移行後もガイドはそのまま「参考表示」として残す
// ================================================================
namespace StrokeGuide {

	void DrawNumberBadge(ImDrawList* dl, ImVec2 center, int num, ImU32 col) {
		dl->AddCircleFilled(center, 11.f, IM_COL32(15, 15, 25, 220));
		dl->AddCircle(center, 11.f, col, 16, 1.5f);
		char buf[4];
		snprintf(buf, sizeof(buf), "%d", num);
		ImVec2 ts = ImGui::CalcTextSize(buf);
		dl->AddText({ center.x - ts.x * 0.5f, center.y - ts.y * 0.5f }, col, buf);
	}

	// まる
	void DrawCircleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size) {
		ImVec2 ctr = { origin.x + size.x * 0.5f, origin.y + size.y * 0.5f };
		float  r = std::min(size.x, size.y) * 0.32f;
		ImU32  col = IM_COL32(255, 200, 60, 255);

		dl->AddCircle(ctr, r, IM_COL32(255, 200, 60, 45), 64, 1.5f);

		for (int i = 0; i < 4; i++) {
			float  angle = -PI_F / 2.f + (PI_F / 2.f * float(i));
			ImVec2 pos = { ctr.x + r * std::cos(angle),
							 ctr.y + r * std::sin(angle) };
			DrawNumberBadge(dl, pos, i + 1, col);
		}
	}

	// △
	void DrawTriangleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size) {
		ImVec2 ctr = { origin.x + size.x * 0.5f, origin.y + size.y * 0.5f };
		float  h = std::min(size.x, size.y) * 0.38f;
		float  hw = h / std::sqrt(3.f);
		ImU32  col = IM_COL32(100, 220, 130, 255);

		ImVec2 A = { ctr.x,      ctr.y - h * 0.65f };
		ImVec2 B = { ctr.x - hw, ctr.y + h * 0.35f };
		ImVec2 C = { ctr.x + hw, ctr.y + h * 0.35f };

		dl->AddTriangle(A, B, C, IM_COL32(100, 220, 130, 45), 1.5f);
		DrawNumberBadge(dl, A, 1, col);
		DrawNumberBadge(dl, B, 2, col);
		DrawNumberBadge(dl, C, 3, col);
	}

	// □
	void DrawSquareGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size) {
		ImVec2 ctr = { origin.x + size.x * 0.5f, origin.y + size.y * 0.5f };
		float  s = std::min(size.x, size.y) * 0.30f;
		ImU32  col = IM_COL32(100, 160, 255, 255);

		ImVec2 TL = { ctr.x - s, ctr.y - s };
		ImVec2 TR = { ctr.x + s, ctr.y - s };
		ImVec2 BR = { ctr.x + s, ctr.y + s };
		ImVec2 BL = { ctr.x - s, ctr.y + s };

		dl->AddRect(TL, BR, IM_COL32(100, 160, 255, 45), 0.f, 0, 1.5f);
		DrawNumberBadge(dl, TL, 1, col);
		DrawNumberBadge(dl, TR, 2, col);
		DrawNumberBadge(dl, BR, 3, col);
		DrawNumberBadge(dl, BL, 4, col);
	}

	// 3種まとめて横並び表示
	void DrawAllGuides(ImDrawList* dl, ImVec2 origin,
		float totalWidth, float panelHeight,
		int highlightMode)
	{
		dl->AddRectFilled(
			origin,
			{ origin.x + totalWidth, origin.y + panelHeight },
			IM_COL32(14, 14, 22, 200)
		);
		dl->AddRect(
			origin,
			{ origin.x + totalWidth, origin.y + panelHeight },
			IM_COL32(60, 60, 80, 180), 3.f, 0, 1.f
		);

		float titleH = 20.f;
		float labelH = 20.f;
		float cellW = totalWidth / 3.f;
		float cellH = panelHeight - titleH - labelH;

		const char* title = "書き順ガイド（参考）";	// $P では書き順不問
		ImVec2 ts = ImGui::CalcTextSize(title);
		dl->AddText(
			{ origin.x + (totalWidth - ts.x) * 0.5f, origin.y + 2.f },
			IM_COL32(180, 180, 200, 200), title
		);

		struct CellInfo {
			const char* label;
			ImU32       frameCol;
			ImU32       labelCol;
			void (*draw)(ImDrawList*, ImVec2, ImVec2);
		} cells[3] = {
			{ "丸",   IM_COL32(255,200, 60,120), IM_COL32(255,200, 60,230), DrawCircleGuide   },
			{ "三角", IM_COL32(100,220,130,120), IM_COL32(100,220,130,230), DrawTriangleGuide },
			{ "四角", IM_COL32(100,160,255,120), IM_COL32(100,160,255,230), DrawSquareGuide   },
		};

		for (int i = 0; i < 3; i++) {
			ImVec2 cellOrigin = { origin.x + cellW * float(i), origin.y + titleH };
			ImVec2 cellSize = { cellW, cellH };
			ImVec2 cellBR = { cellOrigin.x + cellW, cellOrigin.y + cellH };

			if (i == highlightMode) {
				dl->AddRectFilled(cellOrigin, cellBR, IM_COL32(40, 40, 60, 120), 3.f);
			}

			float borderThick = (i == highlightMode) ? 2.f : 0.8f;
			ImU32 borderCol = (i == highlightMode)
				? cells[i].frameCol : IM_COL32(50, 50, 70, 180);
			dl->AddRect(cellOrigin, cellBR, borderCol, 3.f, 0, borderThick);

			cells[i].draw(dl, cellOrigin, cellSize);

			const char* lbl = cells[i].label;
			ts = ImGui::CalcTextSize(lbl);
			dl->AddText(
				{ cellOrigin.x + (cellW - ts.x) * 0.5f, cellOrigin.y + cellH + 2.f },
				cells[i].labelCol, lbl
			);
		}
	}

	void DrawArrow(ImDrawList*, ImVec2, ImVec2, ImU32, float) {} // stub（互換用）

} // namespace StrokeGuide