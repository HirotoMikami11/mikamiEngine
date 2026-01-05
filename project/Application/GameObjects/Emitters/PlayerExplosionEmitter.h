#pragma once
#include "ParticleEditor.h"
#include <string>

class Player;

/// <summary>
/// プレイヤーの死亡爆発演出を管理するクラス
/// 爆発エフェクト → 待機時間 → 演出完了の流れを制御
/// </summary>
class PlayerExplosionEmitter {
public:
	PlayerExplosionEmitter();
	~PlayerExplosionEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="player">Playerへのポインタ</param>
	void Initialize(Player* player);

	/// <summary>
	/// 爆発シーケンスを開始
	/// </summary>
	void StartExplosionSequence();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 演出が完了したか
	/// </summary>
	bool IsExplosionComplete() const { return isComplete_; }

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

private:
	/// <summary>
	/// パーティクルインスタンスを作成
	/// </summary>
	void CreateParticleInstance();

	/// <summary>
	/// 爆発を実行
	/// </summary>
	void ExecuteExplosion();

	/// <summary>
	/// エフェクトが終了したかチェック
	/// </summary>
	bool IsEffectFinished() const;

	Player* player_;
	ParticleEditor* particleEditor_;

	// パーティクルインスタンス
	ParticlePresetInstance* explosionInstance_;

	// シーケンス制御
	bool isSequenceActive_;		// シーケンス実行中か
	bool hasExploded_;			// 爆発済みか
	bool isComplete_;			// 演出完了か

	// タイマー
	float waitTimer_;			// 待機時間カウンター

	// パラメータ
	float waitTime_ = 2.0f;		// 爆発後の待機時間（秒）

	// エミッター名
	const std::string kExplosionPresetName_ = "PlayerExplosion";
	const std::string kEmitterName_ = "PlayerExplosion_Emitter";
	const std::string kInstanceName_ = "PlayerExplosion";
};