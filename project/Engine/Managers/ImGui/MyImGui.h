#pragma once
#include <string>
#include <vector>
#include <functional>
#include "MyMath.h"
using namespace MyMath;
namespace MyImGui {

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

	/// <summary>
	/// 汎用的なコンボボックス
	/// </summary>
	/// <param name="label">UI ラベル</param>
	/// <param name="currentIndex">現在選択されている index (参照渡し)</param>
	/// <param name="items">表示する項目</param>
	/// <returns>選択が変更されたら true</returns>
	bool Combo(const char* label, int& currentIndex, const std::vector<std::string>& items);

	/// <summary>
	/// bool変更ボタンを作成する関数
	/// </summary>
	/// <param name="isOn">オンオフしたいフラグ</param>
	/// <param name="text">表示するテキスト</param>
	/// <param name="size">buttonのサイズ</param>
	void OnOffButton(bool& isOn, const char* text, Vector2 size = Vector2(120, 0));

}