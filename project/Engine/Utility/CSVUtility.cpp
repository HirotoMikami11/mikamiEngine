#include "CSVUtility.h"
#include "Logger.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <format>

bool CSVUtility::SaveVector3List(const std::string& filepath, const std::vector<Vector3>& points) {
	// ディレクトリが存在しない場合は作成
	std::filesystem::path path(filepath);
	std::filesystem::path directory = path.parent_path();

	if (!directory.empty() && !std::filesystem::exists(directory)) {
		try {
			std::filesystem::create_directories(directory);
			Logger::Log(std::format("CSVUtility: Created directory: {}\n", directory.string()));
		} catch (const std::exception& e) {
			Logger::Log(std::format("CSVUtility: Failed to create directory: {} ({})\n",
				directory.string(), e.what()));
			return false;
		}
	}

	// ファイルを開く
	std::ofstream file(filepath);
	if (!file.is_open()) {
		Logger::Log(std::format("CSVUtility: Failed to open file for writing: {}\n", filepath));
		return false;
	}

	// 各Vector3をCSV形式で書き込み
	for (const auto& point : points) {
		file << point.x << "," << point.y << "," << point.z << "\n";
	}

	file.close();

	Logger::Log(std::format("CSVUtility: Successfully saved {} points to {}\n",
		points.size(), filepath));

	return true;
}

bool CSVUtility::LoadVector3List(const std::string& filepath, std::vector<Vector3>& outPoints) {
	// 出力リストをクリア
	outPoints.clear();

	// ファイルの存在チェック
	if (!std::filesystem::exists(filepath)) {
		Logger::Log(std::format("CSVUtility: File not found: {}\n", filepath));
		return false;
	}

	// ファイルを開く
	std::ifstream file(filepath);
	if (!file.is_open()) {
		Logger::Log(std::format("CSVUtility: Failed to open file for reading: {}\n", filepath));
		return false;
	}

	std::string line;
	int lineNumber = 0;

	// 1行ずつ読み込み
	while (std::getline(file, line)) {
		lineNumber++;

		// 空行やコメント行をスキップ
		line = Trim(line);
		if (line.empty() || line[0] == '#' || line[0] == ';') {
			continue;
		}

		// カンマで分割
		std::vector<std::string> tokens = Split(line, ',');

		// 要素数チェック（x, y, z の3つ必要）
		if (tokens.size() != 3) {
			Logger::Log(std::format("CSVUtility: Invalid format at line {} (expected 3 values, got {})\n",
				lineNumber, tokens.size()));
			continue;
		}

		// 文字列をfloatに変換
		try {
			float x = std::stof(Trim(tokens[0]));
			float y = std::stof(Trim(tokens[1]));
			float z = std::stof(Trim(tokens[2]));

			outPoints.push_back({ x, y, z });
		} catch (const std::exception& e) {
			Logger::Log(std::format("CSVUtility: Failed to parse line {} ({})\n",
				lineNumber, e.what()));
			continue;
		}
	}

	file.close();

	// 読み込み結果をログ出力
	if (outPoints.empty()) {
		Logger::Log(std::format("CSVUtility: No valid points loaded from {}\n", filepath));
		return false;
	}

	Logger::Log(std::format("CSVUtility: Successfully loaded {} points from {}\n",
		outPoints.size(), filepath));

	return true;
}

std::string CSVUtility::Trim(const std::string& str) {
	// 前後の空白を削除
	size_t start = str.find_first_not_of(" \t\r\n");
	size_t end = str.find_last_not_of(" \t\r\n");

	if (start == std::string::npos) {
		return "";
	}

	return str.substr(start, end - start + 1);
}

std::vector<std::string> CSVUtility::Split(const std::string& str, char delimiter) {
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;

	while (std::getline(ss, token, delimiter)) {
		tokens.push_back(token);
	}

	return tokens;
}