#include "Logger.h"
#include "StringUtility.h"
#include <Windows.h>
#include <strsafe.h>

//変数の定義
std::ofstream Logger::logFileStream_;
bool Logger::isEnabled_ = true;

void Logger::Initialize()
{
	// ログが無効の場合は何もしない
	if (!isEnabled_) {
		return;
	}

	//ログにディレクトリを用意する
	std::filesystem::create_directory("logs");
	//現在時刻を取得（UTC時刻）
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	//ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	//日本時間（PCの設定時間）に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	//formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	//時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";

	// ログファイルを開く（一度だけ開いて保持）
	logFileStream_.open(logFilePath);

	// ファイルが正常に開かれたかチェック
	if (!logFileStream_.is_open()) {
		OutputDebugStringA("Error: Could not open log file\n");
		return;
	}

	//初期化完了のログ出力
	Logger::Log("Hello,DirectX!\n");
}

void Logger::Finalize()
{
	if (logFileStream_.is_open()) {
		Log("Logger finalized.\n");
		logFileStream_.close();
	}
}

// std::string版
void Logger::Log(const std::string& message) {
	if (!isEnabled_) {
		return;
	}

	// 出力ウィンドウに出力
	OutputDebugStringA(message.c_str());

	// ログファイルに出力
	if (logFileStream_.is_open()) {
		logFileStream_ << message;
		logFileStream_.flush();
	}
}

// std::wstring版（内部で変換）
void Logger::Log(const std::wstring& message) {
	if (!isEnabled_) {
		return;
	}

	// StringUtilityを使ってwstring -> stringに変換
	std::string converted = StringUtility::ConvertString(message);

	// string版のLogを呼び出す
	Log(converted);
}

void Logger::Log(std::ostream& os, const std::string& message) {
	if (!isEnabled_) {
		return;
	}

	// カスタムストリームに出力
	os << message;
	if (&os == &logFileStream_) {
		os.flush();
	}

	// 出力ウィンドウにも出力
	OutputDebugStringA(message.c_str());
}