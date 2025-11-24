#pragma once
#include <variant>
#include <map>
#include <string>
#include <json.hpp>

#include "MyMath.h"

using json = nlohmann::json;
using namespace MyMath;

/// <summary>
/// ゲームの設定値やパラメータをグループごとに管理するクラス
/// </summary>
class JsonSettings
{
public:
	// 項目
	struct Item
	{
		// 項目の値
		std::variant<int32_t, float, bool, Vector2, Vector3, Vector4> value;
	};

	// グループ
	struct Group
	{
		std::unordered_map<std::string, std::any> items;
		std::unordered_map<std::string, Group> subGroups;
	};


	// シングルトンのインスタンスを取得
	static JsonSettings* GetInstance();

	/// <summary>
	/// グループの作成
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void CreateGroup(const std::vector<std::string>& groupPath);

	// 値の取得
	int32_t GetIntValue(const std::vector<std::string>& groupPath, const std::string& key) const;
	float GetFloatValue(const std::vector<std::string>& groupPath, const std::string& key) const;
	bool GetBoolValue(const std::vector<std::string>& groupPath, const std::string& key) const;
	Vector2 GetVector2Value(const std::vector<std::string>& groupPath, const std::string& key) const;
	Vector3 GetVector3Value(const std::vector<std::string>& groupPath, const std::string& key) const;
	Vector4 GetVector4Value(const std::vector<std::string>& groupPath, const std::string& key) const;

	// グループ階層をたどる関数
	const Group* FindGroup(const std::vector<std::string>& groupPath) const;

	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, int32_t value);
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, float value);
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, bool value);
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, const Vector2& value);
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, const Vector3& value);
	void SetValue(const std::vector<std::string>& groupPath, const std::string& key, const Vector4& value);

	// ネスとしたグループに対して値をセットさせる関数
	Group& FindOrCreateGroup(const std::vector<std::string>& groupPath);

	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, int32_t value);
	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, float value);
	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, bool value);
	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, const Vector2& value);
	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, const Vector3& value);
	void AddItem(const std::vector<std::string>& groupPath, const std::string& key, const Vector4& value);

	/// <summary>
	/// 項目の削除
	/// </summary>
	void RemoveItem(const std::vector<std::string>& groupPath, const std::string& key);

	/// <summary>
	/// 毎フレーム処理
	/// </summary>
	void Update();

	// 再帰描画関数
	void DrawGroupRecursive(const std::vector<std::string>& groupPath, Group& group);

	/// <summary>
	/// ファイルに書き出し
	/// </summary>
	/// <param name="groupName">グループ</param>
	// 階層パスを受け取り、JSONファイルの一部だけを更新する
	void SaveFile(const std::vector<std::string>& groupPath);

	// GroupをJSON に変換する再帰関数を作る
	json GroupToJson(const Group& group);

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
	const std::string kDirectoryPath_ = "resources/JsonSettings/";

	// ドラッグの感度
	int dragSensitivityInt_ = 1;
	float dragSensitivity_ = 0.1f;
	float dragSensitivityVector3_ = 0.1f;
};
