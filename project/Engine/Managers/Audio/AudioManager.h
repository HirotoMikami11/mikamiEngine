#pragma once
#include <xaudio2.h>
#include <wrl.h>
#include <map>
#include <string>
#include <cassert>
#include <vector>

#include "Audio/Audio.h"
#include "Logger.h"


/// <summary>
/// 音声を管理する管理クラス（インスタンスベース設計）
/// </summary>
class AudioManager {
public:
	// ゲームアプリケーション全体で一つのインスタンスを使う（シングルトン）
	static AudioManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// 更新処理（再生完了したインスタンスのクリーンアップ）
	/// </summary>
	void Update();

	/// <summary>
	/// 音声データの読み込み（WAV/MP3対応）
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	/// <param name="tagName">識別用タグ名</param>
	void LoadAudio(const std::string& filename, const std::string& tagName);

	/// <summary>
	/// 音声の再生（インスタンスIDを返す）
	/// </summary>
	/// <param name="tagName">識別用タグ名</param>
	/// <param name="isLoop">ループ再生するか</param>
	/// <param name="volume">音量（0.0f～1.0f、デフォルトは1.0f）</param>
	/// <returns>インスタンスID（0は失敗）</returns>
	int Play(const std::string& tagName, bool isLoop = false, float volume = 1.0f);

	/// <summary>
	/// 排他的再生（同じタグの既存インスタンスを停止してから再生）
	/// </summary>
	/// <param name="tagName">識別用タグ名</param>
	/// <param name="isLoop">ループ再生するか</param>
	/// <param name="volume">音量（0.0f～1.0f、デフォルトは1.0f）</param>
	/// <returns>インスタンスID（0は失敗）</returns>
	int PlayOverride(const std::string& tagName, bool isLoop = false, float volume = 1.0f);

	// インスタンスベースの制御（IDで指定）
	void Pause(int instanceId);   // 一時停止
	void Resume(int instanceId);  // 再開
	void Stop(int instanceId);    // 停止
	void SetLoop(int instanceId, bool loop);      // ループ設定変更
	void SetVolume(int instanceId, float volume); // 音量設定

	// タグ名ベースの制御（タグの全インスタンスに適用）
	void PauseByTag(const std::string& tagName);   // そのタグの全インスタンスを一時停止
	void ResumeByTag(const std::string& tagName);  // そのタグの全インスタンスを再開
	void StopByTag(const std::string& tagName);    // そのタグの全インスタンスを停止
	void SetLoopByTag(const std::string& tagName, bool loop);      // そのタグの全インスタンスのループ設定
	void SetVolumeByTag(const std::string& tagName, float volume); // そのタグの全インスタンスの音量設定

	// マスターボリューム制御
	void SetMasterVolume(float volume);  // マスターボリュームの設定（0.0f～1.0f）
	float GetMasterVolume() const { return masterVolume; }

	// 一括停止
	void StopAll();  // 全てのインスタンスを停止

	// 状態確認（インスタンスベース）
	bool HasInstance(int instanceId) const;   // インスタンスが存在するか
	bool IsPlaying(int instanceId) const;     // インスタンスが再生中か
	bool IsPaused(int instanceId) const;      // インスタンスが一時停止中か
	bool IsLooping(int instanceId) const;     // インスタンスがループ再生中か

	// 状態確認（タグベース）
	bool HasAudioData(const std::string& tagName) const;  // 音声データが読み込まれているか
	bool IsPlayingByTag(const std::string& tagName) const; // そのタグのいずれかが再生中か

	// 統計情報
	size_t GetAudioDataCount() const { return audioDataMap.size(); }     // 読み込まれている音声データの数
	size_t GetActiveInstanceCount() const { return instanceMap.size(); } // アクティブなインスタンスの数
	int GetInstanceCountByTag(const std::string& tagName) const;         // 特定タグのインスタンス数

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

private:
	// コンストラクタ
	AudioManager();
	// デストラクタ
	~AudioManager();

	// コピーを禁止
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;

	/// <summary>
	/// 全てのインスタンスの音量を更新（マスターボリューム変更時に使用）
	/// </summary>
	void UpdateAllVolumes();

	/// <summary>
	/// 再生完了したインスタンスを削除
	/// </summary>
	void CleanupFinishedInstances();

	/// <summary>
	/// インスタンスIDを生成
	/// </summary>
	/// <returns>新しいインスタンスID</returns>
	int GenerateInstanceId();

	/// <summary>
	/// タグ名からAudioDataを取得
	/// </summary>
	/// <param name="instanceId">インスタンスID</param>
	/// <returns>タグ名（見つからない場合は空文字列）</returns>
	std::string GetTagNameFromInstance(int instanceId) const;

	// XAudio2のインスタンス
	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	// マスターボイス
	IXAudio2MasteringVoice* masterVoice;
	// 音声データマップ（タグ名 → AudioData）
	std::map<std::string, AudioData*> audioDataMap;
	// インスタンスマップ（インスタンスID → AudioInstance）
	std::map<int, AudioInstance*> instanceMap;
	// マスターボリューム（全体の音量）
	float masterVolume;
	// 次のインスタンスID
	int nextInstanceId;
};