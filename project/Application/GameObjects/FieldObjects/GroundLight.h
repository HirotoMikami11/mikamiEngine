#pragma once
#include "LightManager.h"
#include "MyMath.h"

using namespace MyMath;

/// <summary>
/// 地面照明用のスポットライトクラス
/// 距離のイージングアニメーション機能付き
/// </summary>
class GroundLight {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	GroundLight();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~GroundLight();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="lightManager">LightManagerのポインタ</param>
	void Initialize(LightManager* lightManager);

	/// <summary>
	/// 更新処理（イージングアニメーション）
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// イージング時間を設定
	/// </summary>
	/// <param name="duration">移動時間（秒）</param>
	void SetEasingDuration(float duration) { easingDuration_ = duration; }

	/// <summary>
	/// 距離の範囲を設定
	/// </summary>
	/// <param name="minDistance">最小距離</param>
	/// <param name="maxDistance">最大距離</param>
	void SetDistanceRange(float minDistance, float maxDistance);

	/// <summary>
	/// スポットライトのポインタを取得
	/// </summary>
	SpotLight* GetSpotLight() const { return spotLight_; }

private:
	SpotLight* spotLight_ = nullptr;		// スポットライトのポインタ
	LightManager* lightManager_ = nullptr;	// LightManagerの参照

	// イージングパラメータ
	float easingTimer_ = 0.0f;			// 経過時間
	float easingDuration_ = 10.0f;		// 移動時間
	float minDistance_ = 109.63f;		// 最小距離
	float maxDistance_ = 110.0f;		// 最大距離
	float maxColorG_ = 0.4f;			// 最大緑成分
	float minColorG_ = 0.3f;			// 最小緑成分
	bool isReversing_ = false;			// 逆方向フラグ
};

