#pragma once
#include <variant>
#include <map>
#include <string>
#include <json.hpp>
#include <optional>
#include <any>

#include "MyMath.h"

using json = nlohmann::json;
using namespace MyMath;

/// <summary>
/// ゲームの設定値やパラメータをグループごとに管理するクラス
/// GlobalVariablesパターンを採用し、使いやすさを向上
/// </summary>
class JsonSettings
{
public:
	// グループ
	struct Group
	{
		std::unordered_map<std::string, std::any> items;
		std::unordered_map<std::string, Group> subGroups;
	};

	// シングルトン
	static JsonSettings* GetInstance();

	/// <summary>
	/// グループの作成
	/// </summary>
	/// <param name="groupPath">グループパス</param>
	void CreateGroup(const std::vector<std::string>& groupPath);

	/// <summary>
	/// 階層を調べる関数
	/// </summary>
	/// <param name="groupPath"></param>
	/// <returns></returns>
	const Group* FindGroup(const std::vector<std::string>& groupPath) const;

	/// <summary>
	/// ネストしたグループに対して値をセットさせる関数
	/// </summary>
	Group& FindOrCreateGroup(const std::vector<std::string>& groupPath);

	// 型安全な値の取得（GlobalVariablesパターン）
	int32_t GetIntValue(const std::vector<std::string>& groupPath, const std::string& key) const;
	float GetFloatValue(const std::vector<std::string>& groupPath, const std::string& key) const;
	bool GetBoolValue(const std::vector<std::string>& groupPath, const std::string& key) const;
	Vector2 GetVector2Value(const std::vector<std::string>& groupPath, const std::string& key) const;
	Vector3 GetVector3Value(const std::vector<std::string>& groupPath, const std::string& key) const;
	Vector4 GetVector4Value(const std::vector<std::string>& groupPath, const std::string& key) const;

	// 値のセット（テンプレート版）
	template<typename T>
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, const T& value)
	{
		Group& group = FindOrCreateGroup(groupPath);
		group.items[key] = value;
	}

	// 項目の追加（既に存在する場合は追加されない）
	template<typename T>
	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, const T& value)
	{
		Group& group = FindOrCreateGroup(groupPath);

		// keyが未登録なら追加
		if (group.items.find(key) == group.items.end())
		{
			group.items[key] = value;
		}
	}

	/// <summary>
	/// 項目の削除
	/// </summary>
	/// <param name="groupPath"></param>
	/// <param name="key"></param>
	void RemoveItem(const std::vector<std::string>& groupPath, const std::string& key);

	/// <summary>
	/// 調整項目の設定
	/// </summary>
	void ImGui();

	// 再帰描画関数
	void DrawGroupRecursive(const std::vector<std::string>& groupPath, Group& group);

	/// <summary>
	/// ファイルに書き出し（階層パスを受け取り、JSONファイルの一部だけを更新）
	/// </summary>
	/// <param name="groupPath">グループパス</param>
	void SaveFile(const std::vector<std::string>& groupPath);

	/// <summary>
	/// 全ファイルを保存
	/// </summary>
	void SaveAllFiles();

	// GroupをJSON に変換する再帰関数
	json GroupToJson(const Group& group) const;

	/// <summary>
	/// ディレクトリの全ファイル読み込み
	/// </summary>
	void LoadFiles();

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName">グループ</param>
	void LoadFile(const std::string& groupName);

	// 再帰的にグループを読み込む関数
	void LoadGroupRecursive(const std::vector<std::string>& groupPath, const json& jGroup);

private:

	JsonSettings() = default;
	~JsonSettings() = default;
	JsonSettings(const JsonSettings&) = delete;
	JsonSettings& operator=(const JsonSettings&) = delete;

	// データ
	std::map<std::string, Group> datas_;

	// グローバル変数の保存先ファイルパス
	static constexpr const char* kDirectoryPath_ = "resources/JsonSettings/";

	// ドラッグの感度
	float dragSensitivity_ = 0.1f;

	// 画面表示用のステータスメッセージ
	std::string statusMessage_;
};