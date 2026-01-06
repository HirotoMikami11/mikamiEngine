#pragma once
#include "ParticleEditor.h"
#include "Engine.h"
#include <array>
#include <string>

/// <summary>
/// ボス弾のヒットエフェクト用オブジェクトプール
/// 事前に複数のエミッターインスタンスを生成し、必要に応じて手動で発動・再利用する
/// ワンショットエフェクト専用（放出完了後、自動的に次の利用可能状態になる）
/// </summary>
class BossBulletHitEffectPool {
public:
	BossBulletHitEffectPool();
	~BossBulletHitEffectPool();

	/// <summary>
	/// 初期化
	/// プールサイズ分のエミッターインスタンスを事前生成
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="presetName">使用するパーティクルプリセット名</param>
	/// <param name="emitterName">エミッター名（JSON内のEmitter名）</param>
	void Initialize(DirectXCommon* dxCommon, const std::string& presetName = "BossBulletHit", const std::string& emitterName = "BossBulletHit_Emitter");

	/// <summary>
	/// ヒットエフェクトを発動
	/// 空いているエミッターを探して、指定位置・方向でエフェクトを開始
	/// </summary>
	/// <param name="position">エフェクトの発生位置</param>
	/// <param name="direction">エフェクトの方向（弾丸の進行方向の逆向き）</param>
	/// <returns>エフェクトを発動できた場合true、プールが満杯の場合false</returns>
	bool TriggerEffect(const Vector3& position, const Vector3& direction);

	/// <summary>
	/// 更新処理
	/// 各エフェクトの状態を監視し、放出完了したものを自動的に再利用可能状態にする
	/// </summary>
	void Update();

	/// <summary>
	/// 全てのエミッターインスタンスを破棄
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGui表示（デバッグ用）
	/// </summary>
	void ImGui();

	/// <summary>
	/// 現在アクティブなエフェクト数を取得
	/// </summary>
	size_t GetActiveEffectCount() const;

	/// <summary>
	/// プールサイズを取得
	/// </summary>
	size_t GetPoolSize() const { return kPoolSize_; }

private:
	/// <summary>
	/// エフェクトデータ構造体
	/// </summary>
	struct EffectData {
		ParticlePresetInstance* instance = nullptr;	// パーティクルインスタンス
		bool isActive = false;						// アクティブ状態（放出中）
		float activeTimer = 0.0f;					// アクティブ時間（デバッグ用）
	};

	/// <summary>
	/// パーティクルインスタンスを作成
	/// </summary>
	void CreateParticleInstances();

	/// <summary>
	/// 次に使用可能なエフェクトのインデックスを取得
	/// ラウンドロビン方式で空きスロットを検索
	/// </summary>
	/// <returns>使用可能なインデックス、満杯の場合-1</returns>
	int GetNextAvailableIndex();

	/// <summary>
	/// 指定インデックスのエフェクトを発動
	/// </summary>
	/// <param name="index">エフェクトのインデックス</param>
	/// <param name="position">発生位置</param>
	/// <param name="direction">方向ベクトル</param>
	void ActivateEffect(size_t index, const Vector3& position, const Vector3& direction);

	/// <summary>
	/// 指定インデックスのエフェクトが放出完了したかチェック
	/// </summary>
	/// <param name="index">エフェクトのインデックス</param>
	/// <returns>放出完了していればtrue</returns>
	bool IsEffectFinished(size_t index) const;

	// プールサイズ（同時に表示できるヒットエフェクトの最大数）
	static const size_t kPoolSize_ = 5;

	// エフェクトデータのプール
	std::array<EffectData, kPoolSize_> effects_;

	// 次に使用するインデックス（ラウンドロビン用）
	size_t nextIndex_ = 0;

	// プリセット名とエミッター名
	std::string presetName_;
	std::string emitterName_;

	// ParticleEditorへの参照
	ParticleEditor* particleEditor_ = nullptr;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;

	// デバッグ用フラグ
	bool showDebugInfo_ = false;
};