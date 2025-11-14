#pragma once
#include <vector>
#include "MyFunction.h"

/// <summary>
/// Bossのスプライン軌道データ管理クラス
/// Catmull-Romスプライン曲線を使用した軌道計算
/// </summary>
class BossSplineTrack {
public:
	/// <summary>
	/// 長さテーブルのエントリ
	/// 等間隔移動を実現するために、曲線上の距離とtパラメータの対応を記録
	/// </summary>
	struct LengthTableEntry {
		float t;				// パラメータt (0.0 ~ 1.0)
		float length;			// 累積長さ
		float segmentLength;	// このセグメントの長さ
	};

	BossSplineTrack() = default;
	~BossSplineTrack() = default;

	/// <summary>
	/// 制御点を設定
	/// </summary>
	/// <param name="controlPoints">制御点のリスト（最低4点必要）</param>
	void SetControlPoints(const std::vector<Vector3>& controlPoints);

	/// <summary>
	/// 制御点を取得
	/// </summary>
	const std::vector<Vector3>& GetControlPoints() const { return controlPoints_; }

	/// <summary>
	/// スプライン曲線上の位置を計算
	/// </summary>
	/// <param name="t">パラメータ (0.0 ~ 1.0)</param>
	/// <returns>曲線上の座標</returns>
	Vector3 CalculatePosition(float t) const;

	/// <summary>
	/// 長さテーブルを構築（等間隔移動用）
	/// 曲線を細かく分割して距離を計算し、テーブルに記録
	/// </summary>
	/// <param name="resolution">分割数（デフォルト: 1000）</param>
	void BuildLengthTable(int resolution = 1000);

	/// <summary>
	/// 長さからtパラメータを取得
	/// 等間隔移動を実現するため、指定した距離に対応するtを返す
	/// </summary>
	/// <param name="targetLength">目標距離</param>
	/// <returns>対応するtパラメータ</returns>
	float GetTFromLength(float targetLength) const;

	/// <summary>
	/// スプライン曲線の総長さを取得
	/// </summary>
	float GetTotalLength() const { return totalLength_; }

	/// <summary>
	/// 制御点が有効かチェック
	/// Catmull-Romスプラインには最低4点必要
	/// </summary>
	bool IsValid() const { return controlPoints_.size() >= 4; }

	/// <summary>
	/// 制御点の数を取得
	/// </summary>
	size_t GetControlPointCount() const { return controlPoints_.size(); }

	/// <summary>
	/// 長さテーブルが構築されているかチェック
	/// </summary>
	bool HasLengthTable() const { return !lengthTable_.empty(); }

private:
	// 制御点リスト
	std::vector<Vector3> controlPoints_;

	// 長さテーブル（等間隔移動用）
	std::vector<LengthTableEntry> lengthTable_;
	float totalLength_ = 0.0f;
};