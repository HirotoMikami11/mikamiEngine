#pragma once
#include <vector>
#include <string>
#include "imgui.h"

// ================================================================
// $1 Recognizer の判定結果
// ================================================================
struct DollarResult {
    std::string name;       // マッチしたテンプレート名（空 = 不明）
    float       score;      // スコア（0.0〜1.0、高いほど一致）
    bool        matched;    // score が閾値を超えたか

    const char* GetShapeName() const {
        if (!matched)            return "不明";
        if (name == "circle")    return "まる";
        if (name == "triangle")  return "さんかく";
        if (name == "square")    return "しかく";
        return name.c_str();
    }
};

// ================================================================
// テンプレート1件
// ================================================================
struct DollarTemplate {
    std::string         name;
    std::vector<ImVec2> points; // 正規化済み 64 点
};

// ================================================================
// $1 Unistroke Recognizer
// ================================================================
class Dollar1Recognizer {
public:
    // デフォルトテンプレートを登録してインスタンスを生成
    static Dollar1Recognizer* GetInstance();

    // ストロークを認識して結果を返す
    DollarResult Recognize(const std::vector<ImVec2>& rawPoints) const;

    // テンプレートを追加登録（点列はそのまま渡す。内部で正規化する）
    void AddTemplate(const std::string& name, const std::vector<ImVec2>& points);

    // 内部処理（可視化用に公開）
    static std::vector<ImVec2> Normalize(const std::vector<ImVec2>& pts);

    // テンプレート一覧
    const std::vector<DollarTemplate>& GetTemplates() const { return templates_; }

private:
    Dollar1Recognizer();

    static constexpr int   NUM_POINTS = 64;
    static constexpr float SQUARE_SIZE = 250.0f;
    static constexpr float MATCH_THRESH = 0.75f; // これ以上なら matched=true

    static std::vector<ImVec2> Resample(const std::vector<ImVec2>& pts, int n);
    static float               PathLength(const std::vector<ImVec2>& pts);
    static std::vector<ImVec2> RotateByIndicativeAngle(const std::vector<ImVec2>& pts);
    static std::vector<ImVec2> ScaleToSquare(const std::vector<ImVec2>& pts, float size);
    static std::vector<ImVec2> TranslateToCentroid(const std::vector<ImVec2>& pts);
    static float               PathDistance(const std::vector<ImVec2>& a,
        const std::vector<ImVec2>& b);
    static float               DistanceAtBestAngle(const std::vector<ImVec2>& pts,
        const std::vector<ImVec2>& tmpl);
    static ImVec2              Centroid(const std::vector<ImVec2>& pts);
    static float               Dist(ImVec2 a, ImVec2 b);

    // デフォルトテンプレートを生成する関数群
    static std::vector<ImVec2> MakeCircleTemplate();
    static std::vector<ImVec2> MakeTriangleTemplate();
    static std::vector<ImVec2> MakeSquareTemplate();

    std::vector<DollarTemplate> templates_;
};

// ================================================================
// 書き順ガイドの描画ユーティリティ（ImGuiキャンバス用）
// ================================================================
namespace StrokeGuide {
    // キャンバス上に書き順ガイドを描画する
    // origin: キャンバス左上スクリーン座標, size: キャンバスサイズ
    void DrawCircleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size);
    void DrawTriangleGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size);
    void DrawSquareGuide(ImDrawList* dl, ImVec2 origin, ImVec2 size);

    // 数字バッジ
    void DrawNumberBadge(ImDrawList* dl, ImVec2 center, int num, ImU32 color);

    // stub（互換用）
    void DrawArrow(ImDrawList* dl, ImVec2 from, ImVec2 to,
        ImU32 color, float thickness = 1.5f);

    // 3種まとめてガイドパネルを描画
    // highlightMode: 0=○, 1=△, 2=□, -1=なし
    void DrawAllGuides(ImDrawList* dl, ImVec2 origin,
        float totalWidth, float panelHeight,
        int highlightMode = -1);
}