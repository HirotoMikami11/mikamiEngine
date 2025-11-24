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
/// </summary>
class JsonSettings
{
public:

	using ItemValue = std::variant<int32_t, float, bool, Vector2, Vector3, Vector4>;

	// グループ
	struct Group
	{
		std::unordered_map<std::string, ItemValue> items;
		std::unordered_map<std::string, Group> subGroups;
	};

	// シングルトンのインスタンスを取得
	static JsonSettings* GetInstance();

	/// <summary>
	/// グループの作成
	/// </summary>
	/// <param name="groupPath">グループパス</param>
	void CreateGroup(const std::vector<std::string>& groupPath);


	template<typename T>
	std::optional<T> GetValue(const std::vector<std::string>& groupPath, const std::string& key) const
	{
		const Group* group = FindGroup(groupPath);
		if (!group) return std::nullopt;

		auto itItem = group->items.find(key);
		if (itItem == group->items.end()) return std::nullopt;

		// std::variantから値を取得
		try
		{
			return std::get<T>(itItem->second);
		} catch (const std::bad_variant_access&)
		{
			return std::nullopt;
		}
	}

	// グループ階層をたどる関数
	const Group* FindGroup(const std::vector<std::string>& groupPath) const;


	template<typename T>
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, const T& value)
	{
		Group& group = FindOrCreateGroup(groupPath);
		group.items[key] = value;
	}

	// ネストしたグループに対して値をセットさせる関数
	Group& FindOrCreateGroup(const std::vector<std::string>& groupPath);

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
	void RemoveItem(const std::vector<std::string>& groupPath, const std::string& key);

	/// <summary>
	/// 調整項目の設定
	/// </summary>
	void ImGui();

	// 再帰描画関数
	void DrawGroupRecursive(const std::vector<std::string>& groupPath, Group& group);

	/// <summary>
	/// ファイルに書き出し
	/// </summary>
	/// <param name="groupPath">グループパス</param>
	void SaveFile(const std::vector<std::string>& groupPath);

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

	// 全データ
	std::map<std::string, Group> datas_;

	// グローバル変数の保存先ファイルパス
	static constexpr const char* kDirectoryPath_ = "resources/JsonSettings/";

	// ドラッグの感度
	int dragSensitivityInt_ = 1;
	float dragSensitivity_ = 0.1f;
	float dragSensitivityVector3_ = 0.1f;
};