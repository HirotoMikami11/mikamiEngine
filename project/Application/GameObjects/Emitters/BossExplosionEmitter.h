#pragma once
#include "ParticleEditor.h"
#include "Parts/BaseParts.h"
#include <vector>

class Boss;

/// <summary>
/// ボスの死亡爆発演出を管理するクラス
/// 尻尾→頭の順で爆発させる
/// </summary>
class BossExplosionEmitter {
public:
	/// <summary>
	/// パーツごとの爆発データ
	/// </summary>
	struct PartExplosionData {
		BaseParts* targetPart = nullptr;
		ParticlePresetInstance* instance = nullptr;

		float explosionTimer = 0.0f;    // 次の爆発までの待機時間
		float hideTimer = 0.1f;         // パーツ非表示までの時間

		bool hasExploded = false;       // 爆発済みか
		bool hasHidden = false;         // 非表示済みか
	};

	BossExplosionEmitter();
	~BossExplosionEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="boss">Bossへのポインタ</param>
	void Initialize(Boss* boss);

	/// <summary>
	/// 爆発シーケンスを開始（尻尾→頭の順）
	/// </summary>
	void StartExplosionSequence();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 全パーツの爆発演出が完了したか
	/// </summary>
	bool IsExplosionComplete() const;

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
	void CreateParticleInstances();

	/// <summary>
	/// 次の爆発を実行
	/// </summary>
	void ExecuteNextExplosion();

	Boss* boss_;
	ParticleEditor* particleEditor_;

	// パーツごとの爆発データ（尻尾→頭の順）
	std::vector<PartExplosionData> explosionQueue_;

	// 現在処理中のインデックス
	int currentExplosionIndex_;

	// シーケンス実行中か
	bool isSequenceActive_;

	// パラメータ（ImGuiで調整可能）
	float explosionInterval_ = 0.5f;    // 爆発間隔（秒）
	float hideDelay_ = 0.1f;            // 爆発後、パーツを消すまでの時間（秒）

	// エミッター名
	const std::string kExplosionPresetName_ = "Explosion";
	const std::string kEmitterName_ = "Explosion_Emitter";
};