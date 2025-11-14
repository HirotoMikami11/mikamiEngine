#pragma once
#include <vector>
#include <string>
#include "MyFunction.h"

/// <summary>
/// CSV入出力ユーティリティクラス
/// Vector3リストの保存・読み込み機能を提供
/// </summary>
class CSVUtility {
public:
	/// <summary>
	/// Vector3リストをCSVファイルに保存
	/// フォーマット: x,y,z (1行につき1つのVector3)
	/// </summary>
	/// <param name="filepath">保存先ファイルパス</param>
	/// <param name="points">保存するVector3のリスト</param>
	/// <returns>成功時true</returns>
	static bool SaveVector3List(const std::string& filepath, const std::vector<Vector3>& points);

	/// <summary>
	/// CSVファイルからVector3リストを読み込み
	/// フォーマット: x,y,z (1行につき1つのVector3)
	/// </summary>
	/// <param name="filepath">読み込むファイルパス</param>
	/// <param name="outPoints">読み込んだVector3を格納するリスト（出力）</param>
	/// <returns>成功時true</returns>
	static bool LoadVector3List(const std::string& filepath, std::vector<Vector3>& outPoints);

private:
	/// <summary>
	/// 文字列をトリム（前後の空白を削除）
	/// </summary>
	static std::string Trim(const std::string& str);

	/// <summary>
	/// 文字列を区切り文字で分割
	/// </summary>
	static std::vector<std::string> Split(const std::string& str, char delimiter);
};