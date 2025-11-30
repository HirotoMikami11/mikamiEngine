#pragma once
#include "MyMath.h"
using namespace MyMath;

/// <summary>
/// カメラシェイク（揺れ）管理クラス
/// CameraControllerに組み込まれ、全カメラに揺れ効果を適用
/// </summary>
class CameraShake {
public:
	CameraShake() = default;
	~CameraShake() = default;

	/// <summary>
	/// 揺れを開始
	/// </summary>
	/// <param name="duration">揺れ継続時間（秒）</param>
	/// <param name="amplitude">揺れの強さ</param>
	void StartShake(float duration, float amplitude);

	/// <summary>
	/// 複数回の揺れを開始（爆発演出用）
	/// </summary>
	/// <param name="duration">総継続時間（秒）</param>
	/// <param name="amplitude">最大揺れ強さ</param>
	/// <param name="frequency">揺れ頻度（Hz）</param>
	void StartMultiShake(float duration, float amplitude, float frequency = 8.0f);

	/// <summary>
	/// 揺れを即座に停止
	/// </summary>
	void StopShake();

	/// <summary>
	/// 毎フレーム更新
	/// </summary>
	/// <param name="deltaTime">経過時間（秒）</param>
	void Update(float deltaTime);

	/// <summary>
	/// 現在の揺れオフセットを取得
	/// </summary>
	/// <returns>揺れによる位置オフセット</returns>
	Vector3 GetOffset() const;

	/// <summary>
	/// 揺れ中かどうか判定
	/// </summary>
	/// <returns>揺れ中ならtrue</returns>
	bool IsShaking() const;

	/// <summary>
	/// ImGuiデバッグ表示
	/// </summary>
	void ImGui();

private:
	// 揺れパラメータ
	float shakeDuration_ = 0.0f;        // 揺れ継続時間
	float shakeTimer_ = 0.0f;           // 経過時間
	float shakeAmplitude_ = 0.0f;       // 揺れ強度
	float shakeFrequency_ = 10.0f;      // 揺れ頻度（Hz）

	// 現在のオフセット
	Vector3 shakeOffset_ = { 0.0f, 0.0f, 0.0f };

	// 揺れパターン
	enum class ShakePattern {
		Simple,     // シンプルな減衰揺れ
		Multi       // 複数回揺れ（爆発用）
	};
	ShakePattern currentPattern_ = ShakePattern::Simple;

	// 内部計算
	void UpdateSimpleShake(float deltaTime);
	void UpdateMultiShake(float deltaTime);
};
