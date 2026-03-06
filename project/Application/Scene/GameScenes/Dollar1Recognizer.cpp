#include "Dollar1Recognizer.h"
#include <cmath>
#include <numeric>
#include <algorithm>
#include <limits>

static constexpr float PI_F = 3.14159265358979323846f;
static constexpr float PHI_F = 0.61803398874989484820f;

// ================================================================
// GetInstance / コンストラクタ
// ================================================================
Dollar1Recognizer* Dollar1Recognizer::GetInstance() {
	static Dollar1Recognizer instance;
	return &instance;
}

Dollar1Recognizer::Dollar1Recognizer() {
	AddTemplate("circle", MakeCircleTemplate());
	AddTemplate("triangle", MakeTriangleTemplate());
	AddTemplate("square", MakeSquareTemplate());
}

// ================================================================
// テンプレート生成
// ================================================================

// まる: 12時（上）からスタート → 時計回りに一周
std::vector<ImVec2> Dollar1Recognizer::MakeCircleTemplate() {
	std::vector<ImVec2> pts;
	int   n = 128;
	float r = 120.f;
	for (int i = 0; i <= n; i++) {
		// -π/2 から「角度を減らす方向」= 反時計回り
		float t = -PI_F / 2.f + (2.f * PI_F * float(i) / float(n));
		pts.push_back({ r * std::cos(t), r * std::sin(t) });
	}
	return pts;
}

// △: 上の頂点 → 左下 → 右下 → 上に戻る（反時計回り）
std::vector<ImVec2> Dollar1Recognizer::MakeTriangleTemplate() {
	float h = 120.f;
	float top_y = -(h * 2.f / 3.f);
	float bot_y = (h * 1.f / 3.f);
	float half_w = h / std::sqrt(3.f);

	ImVec2 A = { 0.f, top_y };  // ① 上
	ImVec2 B = { -half_w,  bot_y };  // ② 左下
	ImVec2 C = { half_w,  bot_y };  // ③ 右下

	std::vector<ImVec2> pts;
	int seg = 40;
	auto addSeg = [&](ImVec2 from, ImVec2 to) {
		for (int i = 0; i <= seg; i++) {
			float t = float(i) / float(seg);
			pts.push_back({ from.x + (to.x - from.x) * t,
							from.y + (to.y - from.y) * t });
		}
		};
	addSeg(A, B); // 上 → 左下
	addSeg(B, C); // 左下 → 右下
	addSeg(C, A); // 右下 → 上
	return pts;
}

// □: 左上 → 右上 → 右下 → 左下 → 戻る（時計回り）
std::vector<ImVec2> Dollar1Recognizer::MakeSquareTemplate() {
	float s = 100.f;
	ImVec2 TL = { -s, -s };
	ImVec2 TR = { s, -s };
	ImVec2 BR = { s,  s };
	ImVec2 BL = { -s,  s };

	std::vector<ImVec2> pts;
	int seg = 30;
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
void Dollar1Recognizer::AddTemplate(const std::string& name,
	const std::vector<ImVec2>& points) {
	DollarTemplate tmpl;
	tmpl.name = name;
	tmpl.points = Normalize(points);
	templates_.push_back(tmpl);
}

// ================================================================
// ComputeCircularity: 4π × 面積 / 周長²
//   完全な円 = 1.0、正方形 ≈ 0.785、三角形 ≈ 0.60
//   正規化前の生の点列に対して計算する
// ================================================================
float Dollar1Recognizer::ComputeCircularity(const std::vector<ImVec2>& pts) {
	if (pts.size() < 3) return 0.f;

	// 周長
	float perimeter = PathLength(pts);
	// 始点と終点が離れている場合は閉じた距離を足す
	perimeter += Dist(pts.back(), pts.front());
	if (perimeter < 1e-6f) return 0.f;

	// 面積（シューレース公式）
	float area = 0.f;
	int n = (int)pts.size();
	for (int i = 0; i < n; i++) {
		const ImVec2& a = pts[i];
		const ImVec2& b = pts[(i + 1) % n];
		area += a.x * b.y - b.x * a.y;
	}
	area = std::abs(area) * 0.5f;

	static constexpr float PI_F_LOCAL = 3.14159265f;
	return (4.f * PI_F_LOCAL * area) / (perimeter * perimeter);
}

// ================================================================
// Recognize
// ================================================================
DollarResult Dollar1Recognizer::Recognize(const std::vector<ImVec2>& rawPoints) const {
	DollarResult result;
	result.matched = false;
	result.score = 0.f;
	result.circularity = 0.f;
	result.byCircularity = false;

	if ((int)rawPoints.size() < 10) return result;

	// ---- 前段: 真円度チェック ----
	// 正規化前の生の点列で計算することで $1 の弱点（バウンディングボックス正規化）を回避
	float circ = ComputeCircularity(rawPoints);
	result.circularity = circ;

	if (circ >= CIRCLE_CIRCULARITY_THRESH) {
		// スコアは真円度をそのまま流用（デバッグで見やすい）
		result.name = "circle";
		result.score = circ;
		result.matched = true;
		result.byCircularity = true;
		return result;
	}

	// ---- 後段: $1 Recognizer（さんかく / しかく の判定）----
	auto pts = Normalize(rawPoints);

	float bestScore = -1.f;
	std::string bestName;

	for (const auto& tmpl : templates_) {
		float dist = DistanceAtBestAngle(pts, tmpl.points);
		float halfDiag = 0.5f * std::sqrt(2.f) * SQUARE_SIZE;
		float score = 1.f - dist / halfDiag;
		if (score > bestScore) {
			bestScore = score;
			bestName = tmpl.name;
		}
	}

	result.name = bestName;
	result.score = bestScore;
	result.matched = bestScore >= MATCH_THRESH;
	return result;
}

// ================================================================
// Normalize
// ================================================================
std::vector<ImVec2> Dollar1Recognizer::Normalize(const std::vector<ImVec2>& pts) {
	auto r = Resample(pts, NUM_POINTS);
	r = RotateByIndicativeAngle(r);
	r = ScaleToSquare(r, SQUARE_SIZE);
	r = TranslateToCentroid(r);
	return r;
}

// ================================================================
// Utility
// ================================================================
float Dollar1Recognizer::Dist(ImVec2 a, ImVec2 b) {
	float dx = a.x - b.x, dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

float Dollar1Recognizer::PathLength(const std::vector<ImVec2>& pts) {
	float len = 0.f;
	for (size_t i = 1; i < pts.size(); ++i) len += Dist(pts[i - 1], pts[i]);
	return len;
}

ImVec2 Dollar1Recognizer::Centroid(const std::vector<ImVec2>& pts) {
	float sx = 0.f, sy = 0.f;
	for (auto& p : pts) { sx += p.x; sy += p.y; }
	return { sx / pts.size(), sy / pts.size() };
}

std::vector<ImVec2> Dollar1Recognizer::Resample(const std::vector<ImVec2>& pts, int n) {
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

std::vector<ImVec2> Dollar1Recognizer::RotateByIndicativeAngle(
	const std::vector<ImVec2>& pts)
{
	ImVec2 c = Centroid(pts);
	float  angle = std::atan2(pts[0].y - c.y, pts[0].x - c.x);
	float  cosA = std::cos(-angle);
	float  sinA = std::sin(-angle);

	std::vector<ImVec2> result;
	result.reserve(pts.size());
	for (auto& p : pts) {
		float dx = p.x - c.x, dy = p.y - c.y;
		result.push_back({ dx * cosA - dy * sinA,
						   dx * sinA + dy * cosA });
	}
	return result;
}

std::vector<ImVec2> Dollar1Recognizer::ScaleToSquare(
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

std::vector<ImVec2> Dollar1Recognizer::TranslateToCentroid(
	const std::vector<ImVec2>& pts)
{
	ImVec2 c = Centroid(pts);
	std::vector<ImVec2> result;
	result.reserve(pts.size());
	for (auto& p : pts) result.push_back({ p.x - c.x, p.y - c.y });
	return result;
}

float Dollar1Recognizer::PathDistance(const std::vector<ImVec2>& a,
	const std::vector<ImVec2>& b)
{
	float d = 0.f;
	int n = (int)std::min(a.size(), b.size());
	for (int i = 0; i < n; ++i) d += Dist(a[i], b[i]);
	return d / float(n);
}

float Dollar1Recognizer::DistanceAtBestAngle(const std::vector<ImVec2>& pts,
	const std::vector<ImVec2>& tmpl)
{
	constexpr float ANGLE_RANGE = 45.f * PI_F / 180.f;
	constexpr float ANGLE_PREC = 2.f * PI_F / 180.f;

	float a = -ANGLE_RANGE, b = ANGLE_RANGE;

	auto distAtAngle = [&](float theta) {
		float c = std::cos(theta), s = std::sin(theta);
		std::vector<ImVec2> rot;
		rot.reserve(pts.size());
		for (auto& p : pts)
			rot.push_back({ p.x * c - p.y * s, p.x * s + p.y * c });
		return PathDistance(rot, tmpl);
		};

	float x1 = PHI_F * a + (1.f - PHI_F) * b;
	float x2 = (1.f - PHI_F) * a + PHI_F * b;
	float f1 = distAtAngle(x1);
	float f2 = distAtAngle(x2);

	while (std::abs(b - a) > ANGLE_PREC) {
		if (f1 < f2) {
			b = x2; x2 = x1; f2 = f1;
			x1 = PHI_F * a + (1.f - PHI_F) * b;
			f1 = distAtAngle(x1);
		} else {
			a = x1; x1 = x2; f1 = f2;
			x2 = (1.f - PHI_F) * a + PHI_F * b;
			f2 = distAtAngle(x2);
		}
	}
	return std::min(f1, f2);
}

// ================================================================
// StrokeGuide
//
// 変更点:
//   - 矢印を全廃止、数字バッジのみ
//   - まるは①〜④を時計回りに90°ごと配置
//     ①=12時、②=9時、③=6時、④=3時
//   - 三角は①上、②左下、③右下
//   - 四角は①左上、②右上、③右下、④左下
//   - DrawAllGuides: 3種を横並びで一覧表示するまとめ関数を追加
// ================================================================
namespace StrokeGuide {

	// ---- 共通ヘルパー ------------------------------------------------

	// 数字バッジ（円 + 番号）
	void DrawNumberBadge(ImDrawList* dl, ImVec2 center, int num, ImU32 col) {
		dl->AddCircleFilled(center, 11.f, IM_COL32(15, 15, 25, 220));
		dl->AddCircle(center, 11.f, col, 16, 1.5f);
		char buf[4];
		snprintf(buf, sizeof(buf), "%d", num);
		ImVec2 ts = ImGui::CalcTextSize(buf);
		dl->AddText({ center.x - ts.x * 0.5f, center.y - ts.y * 0.5f }, col, buf);
	}

	// ---- 各図形ガイド（origin=左上, size=描画領域サイズ） ------------

	// まる: 時計回り、①=12時, ②=3時, ③=6時, ④=9時
	void DrawCircleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size)
	{
		ImVec2 ctr = { origin.x + size.x * 0.5f, origin.y + size.y * 0.5f };
		float  r = std::min(size.x, size.y) * 0.32f;
		ImU32  col = IM_COL32(255, 200, 60, 255);

		// 薄い参考円
		dl->AddCircle(ctr, r, IM_COL32(255, 200, 60, 45), 64, 1.5f);

		// ①〜④: 時計回りに90°ずつ
		// ①=12時(-π/2), ②=9時(-π), ③=6時(-3π/2), ④=3時(0 or -2π)
		for (int i = 0; i < 4; i++) {
			float angle = -PI_F / 2.f + (PI_F / 2.f * float(i));
			ImVec2 pos = { ctr.x + r * std::cos(angle),
							ctr.y + r * std::sin(angle) };
			DrawNumberBadge(dl, pos, i + 1, col);
		}
	}

	// △: ①上, ②左下, ③右下
	void DrawTriangleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size)
	{
		ImVec2 ctr = { origin.x + size.x * 0.5f, origin.y + size.y * 0.5f };
		float  h = std::min(size.x, size.y) * 0.38f;
		float  hw = h / std::sqrt(3.f);
		ImU32  col = IM_COL32(100, 220, 130, 255);

		ImVec2 A = { ctr.x,       ctr.y - h * 0.65f }; // ① 上
		ImVec2 B = { ctr.x - hw,  ctr.y + h * 0.35f }; // ② 左下
		ImVec2 C = { ctr.x + hw,  ctr.y + h * 0.35f }; // ③ 右下

		// 薄い参考三角形
		dl->AddTriangle(A, B, C, IM_COL32(100, 220, 130, 45), 1.5f);

		DrawNumberBadge(dl, A, 1, col);
		DrawNumberBadge(dl, B, 2, col);
		DrawNumberBadge(dl, C, 3, col);
	}

	// □: ①左上, ②右上, ③右下, ④左下
	void DrawSquareGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size)
	{
		ImVec2 ctr = { origin.x + size.x * 0.5f, origin.y + size.y * 0.5f };
		float  s = std::min(size.x, size.y) * 0.30f;
		ImU32  col = IM_COL32(100, 160, 255, 255);

		ImVec2 TL = { ctr.x - s, ctr.y - s };
		ImVec2 TR = { ctr.x + s, ctr.y - s };
		ImVec2 BR = { ctr.x + s, ctr.y + s };
		ImVec2 BL = { ctr.x - s, ctr.y + s };

		// 薄い参考四角
		dl->AddRect(TL, BR, IM_COL32(100, 160, 255, 45), 0.f, 0, 1.5f);

		DrawNumberBadge(dl, TL, 1, col);
		DrawNumberBadge(dl, TR, 2, col);
		DrawNumberBadge(dl, BR, 3, col);
		DrawNumberBadge(dl, BL, 4, col);
	}

	// ---- 3種まとめて横並び表示 ---------------------------------------
	//
	// DrawAllGuides: キャンバスとは別のエリアに3種を並べて描画する。
	//
	//  ┌──────────────────────────────────────────────────────┐
	//  │  書き順ガイド                                          │
	//  │  ┌───────────┐ ┌───────────┐ ┌───────────┐          │
	//  │  │     ○     │ │     △     │ │     □     │          │
	//  │  │  ①〜④   │ │  ①②③   │ │ ①②③④  │          │
	//  │  └───────────┘ └───────────┘ └───────────┘          │
	//  │  まる      さんかく   しかく                  │
	//  └──────────────────────────────────────────────────────┘
	//
	// origin: パネル左上のスクリーン座標
	// totalWidth: パネル横幅（ウィンドウ幅に合わせる）
	// panelHeight: パネル高さ
	void DrawAllGuides(ImDrawList* dl, ImVec2 origin, float totalWidth, float panelHeight,
		int highlightMode) // 0=○, 1=△, 2=□, -1=ハイライトなし
	{
		// パネル背景
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

		// タイトル行の高さ
		float titleH = 20.f;
		float labelH = 20.f;
		float cellW = totalWidth / 3.f;
		float cellH = panelHeight - titleH - labelH;

		// --- タイトル ---
		const char* title = "書き順ガイド";
		ImVec2 ts = ImGui::CalcTextSize(title);
		dl->AddText(
			{ origin.x + (totalWidth - ts.x) * 0.5f, origin.y + 2.f },
			IM_COL32(180, 180, 200, 200), title
		);

		// --- 各セル ---
		struct CellInfo {
			const char* label;
			ImU32       frameCol;
			ImU32       labelCol;
			void (*draw)(ImDrawList*, ImVec2, ImVec2);
		} cells[3] = {
			{ "まる",     IM_COL32(255,200,60,120),  IM_COL32(255,200,60,230),  DrawCircleGuide   },
			{ "さんかく", IM_COL32(100,220,130,120), IM_COL32(100,220,130,230), DrawTriangleGuide },
			{ "しかく",   IM_COL32(100,160,255,120), IM_COL32(100,160,255,230), DrawSquareGuide   },
		};

		for (int i = 0; i < 3; i++) {
			ImVec2 cellOrigin = { origin.x + cellW * float(i), origin.y + titleH };
			ImVec2 cellSize = { cellW, cellH };
			ImVec2 cellBR = { cellOrigin.x + cellW, cellOrigin.y + cellH };

			// ハイライト（現在のモードを明るく）
			if (i == highlightMode) {
				dl->AddRectFilled(cellOrigin, cellBR,
					IM_COL32(40, 40, 60, 120), 3.f);
			}

			// セル枠線（ハイライト中は太く）
			float borderThick = (i == highlightMode) ? 2.f : 0.8f;
			ImU32 borderCol = (i == highlightMode) ? cells[i].frameCol
				: IM_COL32(50, 50, 70, 180);
			dl->AddRect(cellOrigin, cellBR, borderCol, 3.f, 0, borderThick);

			// ガイド描画（ラベル分だけ上方向に寄せる）
			cells[i].draw(dl, cellOrigin, cellSize);

			// ラベル（セル下部）
			const char* lbl = cells[i].label;
			ts = ImGui::CalcTextSize(lbl);
			dl->AddText(
				{ cellOrigin.x + (cellW - ts.x) * 0.5f,
				  cellOrigin.y + cellH + 2.f },
				cells[i].labelCol, lbl
			);
		}
	}

	// ヘッダで宣言した個別関数も残す（以前との互換）
	void DrawArrow(ImDrawList*, ImVec2, ImVec2, ImU32, float) {}  // stub（未使用）

} // namespace StrokeGuide