#pragma once

#include <string>
#include <xaudio2.h>
#include <wrl.h>
#include <cstdint>

#include <cassert>
#include <fstream>

#include <vector>
#include <algorithm>

///MP3読込(Media_Foundation)
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

/// <summary>
/// 音声データ構造体（バッファとフォーマット情報）
/// </summary>
struct SoundData {
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファサイズ
	unsigned int bufferSize;
};

/// <summary>
/// チャンクヘッダ
/// </summary>
struct ChunkHeader {
	char id[4];			// チャンクごとのid
	int32_t size;		// チャンクサイズ
};

/// <summary>
/// RIFFヘッダチャンク
/// </summary>
struct RiffHeader {
	ChunkHeader chunk;	// RIFF
	char type[4];		// WAVE
};

/// <summary>
/// FMTチャンク
/// </summary>
struct FormatChunk {
	ChunkHeader chunk;	// fmt
	WAVEFORMATEX fmt;	// 波型フォーマット(18byteまで対応)
};

// 前方宣言
class AudioData;

/// <summary>
/// 音声再生インスタンス（1つの再生を表す）
/// </summary>
class AudioInstance {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	/// <param name="xAudio2">XAudio2インスタンス</param>
	/// <param name="audioData">音声データ</param>
	/// <param name="instanceId">インスタンスID</param>
	/// <param name="isLoop">ループ再生するか</param>
	/// <param name="volume">初期音量</param>
	AudioInstance(IXAudio2* xAudio2, const AudioData* audioData, int instanceId, bool isLoop, float volume);

	/// <summary>
	/// デストラクタ
	/// </summary>
	~AudioInstance();

	/// <summary>
	/// 再生開始
	/// </summary>
	void Start();

	/// <summary>
	/// 一時停止
	/// </summary>
	void Pause();

	/// <summary>
	/// 再開
	/// </summary>
	void Resume();

	/// <summary>
	/// 停止
	/// </summary>
	void Stop();

	/// <summary>
	/// 音量設定
	/// </summary>
	/// <param name="volume">音量（0.0f～1.0f）</param>
	void SetVolume(float volume);

	/// <summary>
	/// マスターボリュームを考慮した音量設定
	/// </summary>
	/// <param name="volume">個別音量</param>
	/// <param name="masterVolume">マスターボリューム</param>
	void SetVolumeWithMaster(float volume, float masterVolume);

	/// <summary>
	/// ループ設定の変更
	/// </summary>
	/// <param name="loop">ループするか</param>
	void SetLoop(bool loop);

	/// <summary>
	/// 再生状態の更新（内部使用）
	/// </summary>
	void Update();

	/// <summary>
	/// インスタンスIDを取得
	/// </summary>
	/// <returns>インスタンスID</returns>
	int GetInstanceId() const { return instanceId; }

	/// <summary>
	/// 現在の音量を取得
	/// </summary>
	/// <returns>音量（0.0f～1.0f）</returns>
	float GetVolume() const { return currentVolume; }

	/// <summary>
	/// 再生中かどうか
	/// </summary>
	/// <returns>再生中ならtrue</returns>
	bool IsPlaying() const { return isPlaying; }

	/// <summary>
	/// 一時停止中かどうか
	/// </summary>
	/// <returns>一時停止中ならtrue</returns>
	bool IsPaused() const { return isPaused; }

	/// <summary>
	/// ループ再生中かどうか
	/// </summary>
	/// <returns>ループ再生中ならtrue</returns>
	bool IsLooping() const { return isLooping; }

	/// <summary>
	/// 再生が完了したかどうか（削除対象かどうか）
	/// </summary>
	/// <returns>再生完了ならtrue</returns>
	bool IsFinished() const { return isFinished; }

	/// <summary>
	/// 参照している音声データのタグ名を取得用（AudioManager経由で取得）
	/// </summary>
	const AudioData* GetAudioData() const { return pAudioData; }

private:
	// インスタンスID
	int instanceId;
	// ソースボイス
	IXAudio2SourceVoice* pSourceVoice;
	// 元の音声データへの参照
	const AudioData* pAudioData;
	// 再生中フラグ
	bool isPlaying;
	// 一時停止中フラグ
	bool isPaused;
	// ループ再生フラグ
	bool isLooping;
	// 再生完了フラグ
	bool isFinished;
	// 現在の音量
	float currentVolume;
	// 一時停止時のサンプル位置
	UINT64 pausedSamplesPlayed;
};

/// <summary>
/// 音声データクラス（1つの音声ファイルを表す）
/// </summary>
class AudioData {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	AudioData();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~AudioData();

	/// <summary>
	/// 音声ファイルの読み込み
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	void LoadFromFile(const std::string& filename);

	/// <summary>
	/// 音声データの解放
	/// </summary>
	void Unload();

	/// <summary>
	/// SoundDataを取得
	/// </summary>
	/// <returns>SoundData参照</returns>
	const SoundData& GetSoundData() const { return soundData; }

private:
	// 音声データ
	SoundData soundData;
};