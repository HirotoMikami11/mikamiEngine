#pragma once
#include <string>
#include <ostream>
#include <format>
#include <filesystem>
#include <fstream>
#include <chrono>

class Logger
{
public:
	static void Initialize();
	static void Finalize();

	// std::string版
	static void Log(const std::string& message);

	// std::wstring版（内部でConvertStringを使用）
	static void Log(const std::wstring& message);

	static void Log(std::ostream& os, const std::string& message);

	// std::format を使用したもの（string）
	template<typename... Args>
	static void Log(const std::string& format, Args&&... args) {
		if constexpr (sizeof...(args) > 0) {
			std::string formatted = std::format(format, std::forward<Args>(args)...);
			Log(formatted);
		} else {
			Log(format);
		}
	}

	// std::format を使用したもの（wstring）
	template<typename... Args>
	static void Log(const std::wstring& format, Args&&... args) {
		if constexpr (sizeof...(args) > 0) {
			std::wstring formatted = std::format(format, std::forward<Args>(args)...);
			Log(formatted);
		} else {
			Log(format);
		}
	}

	static std::ofstream& GetStream() { return logFileStream_; }
	static void SetEnabled(bool enabled) { isEnabled_ = enabled; }
	static bool IsEnabled() { return isEnabled_; }

private:
	static std::ofstream logFileStream_;
	static bool isEnabled_;
};