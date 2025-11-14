#pragma once
#include <string>

class StringUtility
{
public:
	// string->wstring
	static std::wstring ConvertString(const std::string& str);

	// wstring->string
	static std::string ConvertString(const std::wstring& wstr);

private:
	// ユーティリティクラスのためインスタンス化禁止
	StringUtility() = delete;
	~StringUtility() = delete;
	StringUtility(const StringUtility&) = delete;
	StringUtility& operator=(const StringUtility&) = delete;
};

