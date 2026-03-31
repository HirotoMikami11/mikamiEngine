#include "MojiTestScene.h"

MojiTestScene::MojiTestScene()
	: BaseScene("MojiTestScene")
	, cameraController_(nullptr)
	, dxCommon_(nullptr)
	, offscreenRenderer_(nullptr)
	, viewProjectionMatrix{ MakeIdentity4x4() }
{
}

MojiTestScene::~MojiTestScene() = default;

void MojiTestScene::ConfigureOffscreenEffects() {
	offscreenRenderer_->DisableAllEffects();
}

void MojiTestScene::Initialize() {
	dxCommon_ = Engine::GetInstance()->GetDirectXCommon();
	offscreenRenderer_ = Engine::GetInstance()->GetOffscreenRenderer();

	cameraController_ = CameraController::GetInstance();
	cameraController_->Initialize(dxCommon_,
		Vector3{ 0.f, 6.8f, -18.f }, Vector3{ 0.4f, 0.f, 0.f });
	cameraController_->SetActiveCamera("normal");

	InitializeGameObjects();
	ConfigureOffscreenEffects();
}

void MojiTestScene::InitializeGameObjects() {
	sphere_ = std::make_unique<Sphere>();
	sphere_->Initialize(dxCommon_, "sphere", "monsterBall");
	sphere_->SetTransform(Vector3Transform{
		{1,1,1}, {0,-std::numbers::pi_v<float> / 2.f,0}, {0,0,0} });
	sphere_->SetLightingMode(LightingMode::PhongSpecular);

	terrain_ = std::make_unique<Model3D>();
	terrain_->Initialize(dxCommon_, "model_terrain");
	terrain_->SetTransform(Vector3Transform{ {1,1,1},{0,0,0},{0,0,0} });

	LightManager::GetInstance()->GetDirectionalLight()
		.SetColor({ 1.f,1.f,1.f,1.f });
}

void MojiTestScene::Update() {
	cameraController_->Update();
	UpdateGameObjects();
}

void MojiTestScene::UpdateGameObjects() {
	viewProjectionMatrix = cameraController_->GetViewProjectionMatrix();
	sphere_->Update(viewProjectionMatrix);
	terrain_->Update(viewProjectionMatrix);
}

void MojiTestScene::DrawOffscreen() { sphere_->Draw(); terrain_->Draw(); }
void MojiTestScene::DrawBackBuffer() {}

// ================================================================
// ImGui
// ================================================================
void MojiTestScene::ImGui() {
#ifdef USEIMGUI
	sphere_->ImGui();
	terrain_->ImGui();

	// ================================================================
	// ハイライトインデックス計算（ガイドとキャンバス両ウィンドウで共有）
	// ================================================================
	int highlight = -1;
	if (hasResult_ && lastResult_.matched) {
		if (lastResult_.name == "circle")    highlight = 0;
		else if (lastResult_.name == "triangle")  highlight = 1;
		else if (lastResult_.name == "square")    highlight = 2;
		else if (lastResult_.name == "star")      highlight = 3;
		else if (lastResult_.name == "check")     highlight = 4;
		else if (lastResult_.name == "lightning") highlight = 5;
		else if (lastResult_.name == "spiral")    highlight = 6;
		else if (lastResult_.name == "omega")     highlight = 7;
	}

	// ================================================================
	// [ウィンドウ A] 書き順ガイド
	// ================================================================
	ImGui::SetNextWindowSize(ImVec2(540.f, 310.f), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(20.f, 80.f), ImGuiCond_Once);
	ImGui::Begin("書き順ガイド", nullptr,
		ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2      origin = ImGui::GetCursorScreenPos();
		ImVec2      avail = ImGui::GetContentRegionAvail();
		float       guideH = avail.y;

		StrokeGuide::DrawAllGuides(dl, origin, avail.x, guideH, highlight);
		ImGui::Dummy(ImVec2(avail.x, guideH));
	}
	ImGui::End();

	// ================================================================
	// [ウィンドウ B] 描画キャンバス
	// ================================================================
	constexpr float CANVAS_W = 420.f;
	constexpr float CANVAS_H = 360.f;

	ImGui::SetNextWindowSize(ImVec2(460.f, 620.f), ImGuiCond_Once);
	ImGui::SetNextWindowPos(ImVec2(570.f, 80.f), ImGuiCond_Once);
	ImGui::Begin("描画キャンバス", nullptr, ImGuiWindowFlags_NoScrollbar);

	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		ImVec2 canvasOrigin = ImGui::GetCursorScreenPos();

		// --- 背景 ---
		dl->AddRectFilled(canvasOrigin,
			{ canvasOrigin.x + CANVAS_W, canvasOrigin.y + CANVAS_H },
			IM_COL32(16, 16, 26, 255));

		if (strokePoints_.empty() && !isDrawing_) {
			const char* hint = "ここに図形を描いてください";
			ImVec2 hs = ImGui::CalcTextSize(hint);
			dl->AddText(
				{ canvasOrigin.x + (CANVAS_W - hs.x) * 0.5f,
				  canvasOrigin.y + (CANVAS_H - hs.y) * 0.5f },
				IM_COL32(55, 55, 75, 200), hint);
		}

		// --- 枠線 ---
		ImU32 borderCol = IM_COL32(70, 70, 100, 180);
		float borderW = 1.5f;
		if (isDrawing_) {
			borderCol = IM_COL32(180, 180, 220, 220);
			borderW = 2.5f;
		} else if (hasResult_) {
			if (lastResult_.matched) {
				if (lastResult_.name == "circle")    borderCol = IM_COL32(255, 200, 60, 200);
				else if (lastResult_.name == "triangle")  borderCol = IM_COL32(100, 220, 130, 200);
				else if (lastResult_.name == "square")    borderCol = IM_COL32(100, 160, 255, 200);
				else if (lastResult_.name == "star")      borderCol = IM_COL32(230, 130, 255, 200);
				else if (lastResult_.name == "check")     borderCol = IM_COL32(60, 220, 180, 200);
				else if (lastResult_.name == "lightning") borderCol = IM_COL32(255, 220, 40, 200);
				else if (lastResult_.name == "spiral")    borderCol = IM_COL32(60, 210, 200, 200);
				else if (lastResult_.name == "omega")     borderCol = IM_COL32(230, 160, 80, 200);
				borderW = 2.f;
			} else {
				borderCol = IM_COL32(200, 70, 70, 200);
				borderW = 2.f;
			}
		}
		dl->AddRect(canvasOrigin,
			{ canvasOrigin.x + CANVAS_W, canvasOrigin.y + CANVAS_H },
			borderCol, 0.f, 0, borderW);

		// --- マウス入力 ---
		ImGui::InvisibleButton("canvas", { CANVAS_W, CANVAS_H });
		bool   hovered = ImGui::IsItemHovered();
		ImVec2 mpos = ImGui::GetMousePos();

		if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
			isDrawing_ = true;
			strokePoints_.clear();
			hasResult_ = false;
		}
		if (isDrawing_ && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
			ImVec2 local = { mpos.x - canvasOrigin.x, mpos.y - canvasOrigin.y };
			if (strokePoints_.empty()) {
				strokePoints_.push_back(local);
			} else {
				float dx = local.x - strokePoints_.back().x;
				float dy = local.y - strokePoints_.back().y;
				if (dx * dx + dy * dy > 4.f) strokePoints_.push_back(local);
			}
		}
		if (isDrawing_ && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
			isDrawing_ = false;
			if ((int)strokePoints_.size() >= 10) {
				lastResult_ = Dollar1Recognizer::GetInstance()->Recognize(strokePoints_);
				hasResult_ = true;
			}
		}

		// --- ストローク描画 ---
		if ((int)strokePoints_.size() > 1) {
			ImU32 strokeCol = IM_COL32(220, 220, 220, 210);
			if (hasResult_) {
				if (lastResult_.matched) {
					if (lastResult_.name == "circle")    strokeCol = IM_COL32(255, 200, 60, 255);
					else if (lastResult_.name == "triangle")  strokeCol = IM_COL32(100, 220, 130, 255);
					else if (lastResult_.name == "square")    strokeCol = IM_COL32(100, 160, 255, 255);
					else if (lastResult_.name == "star")      strokeCol = IM_COL32(230, 130, 255, 255);
					else if (lastResult_.name == "check")     strokeCol = IM_COL32(60, 220, 180, 255);
					else if (lastResult_.name == "lightning") strokeCol = IM_COL32(255, 220, 40, 255);
					else if (lastResult_.name == "spiral")    strokeCol = IM_COL32(60, 210, 200, 255);
					else if (lastResult_.name == "omega")     strokeCol = IM_COL32(230, 160, 80, 255);
				} else {
					strokeCol = IM_COL32(210, 65, 65, 255);
				}
			}
			for (int i = 1; i < (int)strokePoints_.size(); ++i) {
				dl->AddLine(
					{ canvasOrigin.x + strokePoints_[i - 1].x, canvasOrigin.y + strokePoints_[i - 1].y },
					{ canvasOrigin.x + strokePoints_[i].x, canvasOrigin.y + strokePoints_[i].y },
					strokeCol, 2.5f);
			}
			// スタートマーカー
			dl->AddCircleFilled(
				{ canvasOrigin.x + strokePoints_.front().x,
				  canvasOrigin.y + strokePoints_.front().y },
				5.f, IM_COL32(60, 215, 60, 230));
		}

		// --- 判定結果 ---
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (isDrawing_) {
			ImGui::TextDisabled("描いています... (%d 点)", (int)strokePoints_.size());

		} else if (hasResult_) {
			ImGui::SetWindowFontScale(1.8f);
			if (lastResult_.matched) {
				ImVec4 resCol =
					lastResult_.name == "circle" ? ImVec4(1.00f, 0.78f, 0.24f, 1.f) :
					lastResult_.name == "triangle" ? ImVec4(0.40f, 0.86f, 0.51f, 1.f) :
					lastResult_.name == "square" ? ImVec4(0.40f, 0.63f, 1.00f, 1.f) :
					lastResult_.name == "check" ? ImVec4(0.24f, 0.86f, 0.71f, 1.f) :
					lastResult_.name == "lightning" ? ImVec4(1.00f, 0.86f, 0.16f, 1.f) :
					lastResult_.name == "spiral" ? ImVec4(0.24f, 0.82f, 0.78f, 1.f) :
					lastResult_.name == "omega" ? ImVec4(0.90f, 0.63f, 0.31f, 1.f) :
					ImVec4(0.90f, 0.51f, 1.00f, 1.f);
				ImGui::TextColored(resCol, "%s", lastResult_.GetShapeName());
			} else {
				ImGui::TextColored(ImVec4(0.88f, 0.3f, 0.3f, 1.f), "認識できませんでした");
			}
			ImGui::SetWindowFontScale(1.0f);

			ImGui::Spacing();
			ImGui::TextDisabled("真円度:%.3f  %s",
				lastResult_.circularity,
				lastResult_.byCircularity ? "[丸確定]" : "[$1判定]");
			ImGui::Text("スコア: %.0f%%", lastResult_.score * 100.f);

			ImVec4 barCol =
				lastResult_.score > 0.90f ? ImVec4(0.2f, 0.88f, 0.3f, 1.f) :
				lastResult_.score > 0.75f ? ImVec4(0.9f, 0.78f, 0.2f, 1.f) :
				ImVec4(0.88f, 0.3f, 0.2f, 1.f);
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, barCol);
			ImGui::ProgressBar(lastResult_.score, ImVec2(-1.f, 0.f), "");
			ImGui::PopStyleColor();

			if (!lastResult_.matched) {
				ImGui::Spacing();
				ImGui::TextColored(ImVec4(0.75f, 0.75f, 0.45f, 1.f),
					"ガイドの番号順に描いてみてください");
			}

		} else {
			ImGui::TextColored(ImVec4(0.45f, 0.45f, 0.55f, 1.f),
				"ガイドの番号順に図形を描いてください");
		}

		ImGui::Spacing();
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.15f, 0.15f, 0.22f, 1.f));
		if (ImGui::Button("クリア", ImVec2(100.f, 28.f))) {
			strokePoints_.clear();
			hasResult_ = false;
		}
		ImGui::PopStyleColor();
	}
	ImGui::End(); // 描画キャンバス

#endif
}

void MojiTestScene::Finalize() {}