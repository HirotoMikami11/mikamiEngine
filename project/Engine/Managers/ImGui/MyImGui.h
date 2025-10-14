#pragma once
#include <string>
#include <vector>
#include <functional>
#include "MyMath.h"
using namespace MyMath;
namespace myImGui {

	/// <summary>
	/// 中央揃えでテキストを表示する関数
	/// </summary>
	/// <param name="text">表示するテキスト</param>
	void CenterText(const char* text);

	/// <summary>
	/// セパレータ付きセクションヘッダー
	/// </summary>
	/// <param name="text">ヘッダーテキスト</param>
	void SectionHeader(const char* text);

	/// <summary>
	/// インデント付きテキスト表示
	/// </summary>
	/// <param name="text">表示するテキスト</param>
	/// <param name="indent">インデント量</param>
	void IndentedText(const char* text, float indent = 16.0f);

	/// <summary>
	/// 色付きテキスト表示
	/// </summary>
	/// <param name="text">表示するテキスト</param>
	/// <param name="color">テキストの色</param>
	void ColoredText(const char* text, const Vector4& color);

	/// <summary>
	/// ファイル操作ボタン群の構造体
	/// </summary>
	struct FileOperationButtons {
		std::function<void()> onSave;
		std::function<void()> onLoad;
		std::function<void()> onClear;
		const char* saveLabel = "Save";
		const char* loadLabel = "Load";
		const char* clearLabel = "Clear";
	};

	/// <summary>
	/// ファイル操作ボタン群を描画
	/// </summary>
	/// <param name="buttons">ボタン設定</param>
	void DrawFileOperationButtons(const FileOperationButtons& buttons);

	/// <summary>
	/// 設定チェックボックス群の構造体
	/// </summary>
	struct SettingsCheckboxes {
		std::vector<std::pair<std::string, bool*>> checkboxes;
	};

	/// <summary>
	/// 設定チェックボックス群を描画
	/// </summary>
	/// <param name="settings">設定</param>
	void DrawSettingsCheckboxes(const SettingsCheckboxes& settings);

	/// <summary>
	/// ドラッグ可能な数値入力（範囲制限付き）
	/// </summary>
	/// <param name="label">ラベル</param>
	/// <param name="value">編集する値</param>
	/// <param name="min">最小値</param>
	/// <param name="max">最大値</param>
	/// <param name="speed">変更速度</param>
	/// <returns>値が変更されたかどうか</returns>
	bool DragIntRange(const char* label, int& value, int min, int max, int speed = 1);

	/// <summary>
	/// ヘルプテキスト付きのマーカー
	/// </summary>
	/// <param name="desc">ヘルプテキスト</param>
	void HelpMarker(const char* desc);

	/// <summary>
	/// 警告テキスト表示（黄色）
	/// </summary>
	/// <param name="text">表示するテキスト</param>
	void WarningText(const char* text);

	/// <summary>
	/// 成功テキスト表示（緑色）
	/// </summary>
	/// <param name="text">表示するテキスト</param>
	void SuccessText(const char* text);

	/// <summary>
	/// エラーテキスト表示（赤色）
	/// </summary>
	/// <param name="text">表示するテキスト</param>
	void ErrorText(const char* text);

	/// <summary>
	/// 区切り線付きスペーシング
	/// </summary>
	void SeparatorWithSpacing();

	/// <summary>
	/// タブ付きセクション
	/// </summary>
	/// <param name="tabs">タブ名のリスト</param>
	/// <param name="selectedTab">選択されたタブのインデックス</param>
	/// <param name="tabContents">各タブの内容を描画する関数のリスト</param>
	void TabbedSection(const std::vector<std::string>& tabs, int& selectedTab, const std::vector<std::function<void()>>& tabContents);

	/// <summary>
	/// フレーム付きセクション
	/// </summary>
	/// <param name="label">セクションのラベル</param>
	/// <param name="content">内容を描画する関数</param>
	/// <param name="color">フレームの色（オプション）</param>
	void FramedSection(const char* label, const std::function<void()>& content, const Vector4* color = nullptr);
}