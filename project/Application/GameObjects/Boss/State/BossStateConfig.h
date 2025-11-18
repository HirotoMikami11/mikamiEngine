#pragma once
#include <vector>
#include <string>

/// <summary>
/// SplineMove8WayShootStateの設定
/// </summary>
struct SplineMove8WayShootConfig {
	std::vector<std::string> csvFilePaths;	// CSVファイルパスのリスト
	int shootInterval = 180;				// 弾発射間隔（フレーム）
	float bulletSpeed = 0.2f;				// 弾の速度
	int onShootBulletNumber = 2;			// 同時発射数
	/// <summary>
	/// 有効な設定かチェック
	/// </summary>
	bool IsValid() const {
		return !csvFilePaths.empty();
	}
};

/// <summary>
/// SplineMoveRotateShootStateの設定
/// </summary>
struct SplineMoveRotateShootConfig {
	std::vector<std::string> csvFilePaths;	// CSVファイルパスのリスト
	int stopControlPointIndex = 2;			// 停止する制御点のインデックス
	float startAngle = -60.0f;				// 回転開始角度（度）
	float endAngle = 60.0f;					// 回転終了角度（度）
	float rotationSpeed = 2.0f;				// 回転速度（度/フレーム）
	int shootInterval = 4;					// 発射間隔（フレーム）
	float bulletSpeed = 0.3f;				// 弾の速度
	int angleIntervalDuration = 30;			// 角度到達時の停止時間（フレーム）
	int maxRepeatCount = 1;					// 往復回数

	/// <summary>
	/// 有効な設定かチェック
	/// </summary>
	bool IsValid() const {
		return !csvFilePaths.empty();
	}
};

/// <summary>
/// SplineMoveStateの設定
/// </summary>
struct SplineMoveConfig {
	std::vector<std::string> csvFilePaths;	// CSVファイルパスのリスト

	/// <summary>
	/// 有効な設定かチェック
	/// </summary>
	bool IsValid() const {
		return !csvFilePaths.empty();
	}
};