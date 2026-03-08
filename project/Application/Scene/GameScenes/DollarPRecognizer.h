#pragma once
#include <vector>
#include <string>
#include "imgui.h"

// ================================================================
// $P Recognizer の判定結果（$1 と互換）
// ================================================================
struct DollarResult {
	std::string	name;						// マッチしたテンプレート名（空 = 不明）
	float		score;						// スコア（0.0〜1.0、高いほど一致）
	bool		matched;					// score が閾値を超えたか
	float		circularity = 0.f;			// 真円度（デバッグ用）
	bool		byCircularity = false;		// true = 真円度で確定（$P 未使用）

	const char* GetShapeName() const {
		if (!matched)			return "不明";
		if (name == "circle")	return "丸";
		if (name == "triangle")	return "三角形";
		if (name == "square")	return "四角形";
		return name.c_str();
	}
};

// ================================================================
// テンプレート1件
// ================================================================
struct DollarTemplate {
	std::string         name;
	std::vector<ImVec2> points; // 正規化済み NUM_POINTS 点
};

// ================================================================
// $P Point-Cloud Recognizer
//
// $1 との主な違い：
//   ・ジェスチャーを「順序なし点群」として扱う
//   ・書き順・向き（時計回り/反時計回り）を完全に無視
//   ・IndicativeAngle 回転ステップが不要
//   ・マッチングに GreedyCloudMatch（ハンガリアン近似）を使用
// ================================================================
class DollarPRecognizer {
public:
	// シングルトン（デフォルトテンプレート登録済み）
	static DollarPRecognizer* GetInstance();

	// ストロークを認識して結果を返す
	DollarResult Recognize(const std::vector<ImVec2>& rawPoints) const;

	// テンプレートを追加登録（点列はそのまま渡す、内部で正規化）
	void AddTemplate(const std::string& name, const std::vector<ImVec2>& points);

	// 正規化（可視化・デバッグ用に公開）
	// $P では: Resample → ScaleToSquare → TranslateToCentroid（回転なし）
	static std::vector<ImVec2> Normalize(const std::vector<ImVec2>& pts);

	// テンプレート一覧
	const std::vector<DollarTemplate>& GetTemplates() const { return templates_; }

private:
	DollarPRecognizer();

	// ---- パラメータ ----
	static constexpr int   NUM_POINTS = 32;		// $P 標準は 32 点
	static constexpr float SQUARE_SIZE = 250.0f;
	static constexpr float MATCH_THRESH = 0.80f;	// これ以上なら matched=true
	static constexpr float CIRCLE_CIRCULARITY_THRESH = 0.82f;

	// ---- 前処理（$1 と共通） ----
	static std::vector<ImVec2> Resample(const std::vector<ImVec2>& pts, int n);
	static float               PathLength(const std::vector<ImVec2>& pts);
	static std::vector<ImVec2> ScaleToSquare(const std::vector<ImVec2>& pts, float size);
	static std::vector<ImVec2> TranslateToCentroid(const std::vector<ImVec2>& pts);
	static ImVec2              Centroid(const std::vector<ImVec2>& pts);
	static float               Dist(ImVec2 a, ImVec2 b);

	// ---- $P コア：GreedyCloudMatch ----
	// GreedyCloudMatch: n^0.5 ステップで開始点を変えて CloudDistance を計算し最小値を返す
	static float GreedyCloudMatch(const std::vector<ImVec2>& pts,
		const std::vector<ImVec2>& tmpl);

	// CloudDistance: start を起点に最近傍貪欲割り当て（重み付きユークリッド和）
	static float CloudDistance(const std::vector<ImVec2>& pts,
		const std::vector<ImVec2>& tmpl,
		int start);

	// ---- 真円度チェック（$P 前段フィルタ） ----
	static float ComputeCircularity(const std::vector<ImVec2>& pts);

	// ---- デフォルトテンプレート生成 ----
	// 書き順不問なので 1 方向のみで十分だが、形状の違いをカバーするため
	// 各図形に複数バリアントを追加
	static std::vector<ImVec2> MakeCircleTemplate();
	static std::vector<ImVec2> MakeTriangleTemplate();
	static std::vector<ImVec2> MakeSquareTemplate();

	std::vector<DollarTemplate> templates_;
};

// ================================================================
// 書き順ガイドの描画ユーティリティ（$P では「書き順」は参考表示）
// ================================================================
namespace StrokeGuide {
	void DrawCircleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size);
	void DrawTriangleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size);
	void DrawSquareGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size);
	void DrawNumberBadge(ImDrawList* dl, ImVec2 center, int num, ImU32 color);
	void DrawArrow(ImDrawList* dl, ImVec2 from, ImVec2 to,
		ImU32 color, float thickness = 1.5f);
	void DrawAllGuides(ImDrawList* dl, ImVec2 origin,
		float totalWidth, float panelHeight,
		int highlightMode = -1);
}