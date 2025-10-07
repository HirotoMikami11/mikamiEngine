#include "MyImGui.h"
#include "../externals/imgui/imgui.h"

namespace myImGui {

	void CenterText(const char* text) {
		// テキストを中央に配置するためのオフセットを計算
		float windowWidth = ImGui::GetWindowSize().x;
		float textWidth = ImGui::CalcTextSize(text).x;
		float offsetX = (windowWidth - textWidth) * 0.5f;

		// 現在のY位置を取得（カーソル位置復元用）
		ImVec2 cursorPos = ImGui::GetCursorPos();

		// 中央にカーソルを設定して描画
		if (offsetX > 0.0f) {
			ImGui::SetCursorPosX(offsetX);
		}
		ImGui::TextUnformatted(text);

		// 左揃えに戻す（次の行のXを元の位置に）
		ImGui::SetCursorPosX(cursorPos.x);
	}

	void SectionHeader(const char* text) {
		ImGui::Separator();
		CenterText(text);
		ImGui::Separator();
		ImGui::Spacing();
	}

	void IndentedText(const char* text, float indent) {
		ImGui::Indent(indent);
		ImGui::TextUnformatted(text);
		ImGui::Unindent(indent);
	}

	void ColoredText(const char* text, const Vector4& color) {
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(
			static_cast<int>(color.x * 255),
			static_cast<int>(color.y * 255),
			static_cast<int>(color.z * 255),
			static_cast<int>(color.w * 255)
		));
		ImGui::TextUnformatted(text);
		ImGui::PopStyleColor();
	}

	void DrawFileOperationButtons(const FileOperationButtons& buttons) {
		if (buttons.onSave && ImGui::Button(buttons.saveLabel)) {
			buttons.onSave();
		}
		if (buttons.onLoad) {
			ImGui::SameLine();
			if (ImGui::Button(buttons.loadLabel)) {
				buttons.onLoad();
			}
		}
		if (buttons.onClear) {
			ImGui::SameLine();
			if (ImGui::Button(buttons.clearLabel)) {
				buttons.onClear();
			}
		}
	}


	void DrawSettingsCheckboxes(const SettingsCheckboxes& settings) {
		for (const auto& [label, valuePtr] : settings.checkboxes) {
			ImGui::Checkbox(label.c_str(), valuePtr);
		}
	}

	bool DragIntRange(const char* label, int& value, int min, int max, int speed) {
		return ImGui::DragInt(label, &value, static_cast<float>(speed), min, max);
	}

	void HelpMarker(const char* desc) {
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	void WarningText(const char* text) {
		ColoredText(text, { 1.0f, 1.0f, 0.0f, 1.0f }); // 黄色
	}

	void SuccessText(const char* text) {
		ColoredText(text, { 0.0f, 1.0f, 0.0f, 1.0f }); // 緑色
	}

	void ErrorText(const char* text) {
		ColoredText(text, { 1.0f, 0.0f, 0.0f, 1.0f }); // 赤色
	}

	void SeparatorWithSpacing() {
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	void TabbedSection(const std::vector<std::string>& tabs, int& selectedTab, const std::vector<std::function<void()>>& tabContents) {
		if (tabs.empty() || tabContents.empty()) return;

		// タブバーを描画
		if (ImGui::BeginTabBar("TabbedSection")) {
			for (size_t i = 0; i < tabs.size(); ++i) {
				if (ImGui::BeginTabItem(tabs[i].c_str())) {
					selectedTab = static_cast<int>(i);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}

		// 選択されたタブの内容を描画
		if (selectedTab >= 0 && selectedTab < static_cast<int>(tabContents.size())) {
			tabContents[selectedTab]();
		}
	}

	void FramedSection(const char* label, const std::function<void()>& content, const Vector4* color) {
		ImVec4 frameColor = color ?
			ImVec4(color->x, color->y, color->z, color->w) :
			ImGui::GetStyleColorVec4(ImGuiCol_Border);

		ImGui::PushStyleColor(ImGuiCol_Border, frameColor);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

		ImGui::BeginGroup();

		// ラベルを描画
		if (label && strlen(label) > 0) {
			ImGui::Text("%s", label);
			ImGui::Separator();
		}

		// 内容を描画
		content();

		ImGui::EndGroup();

		// フレームを描画
		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImGui::GetWindowDrawList()->AddRect(min, max, ImGui::GetColorU32(frameColor));

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor();
	}
}