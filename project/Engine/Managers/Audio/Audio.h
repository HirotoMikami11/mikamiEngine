#pragma once

#include <string>
#include <xaudio2.h>
#include <wrl.h>
#include <cstdint>

#include <cassert>
#include <fstream>

#include <vector>		// std::vector用
#include <algorithm>	// std::transform用

///MP3読込(Media_Foundation)
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")

/// <summary>
/// 音声データ構造体
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

/// <summary>
/// 音声クラス
/// </summary>
class Audio {
public:
	// コンストラクタ
	Audio();
	// デストラクタ
	~Audio();

	/// <summary>
	/// 音声データの読み込み（WAV/MP3対応）
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	void LoadAudio(const std::string& filename);

	/// <summary>
	/// 音声の再生（ループなし）
	/// </summary>
	/// <param name="xAudio2"></param>
	void Play(IXAudio2* xAudio2);

	/// <summary>
	/// 音声の再生（ループあり）
	/// </summary>
	/// <param name="xAudio2"></param>
	void PlayLoop(IXAudio2* xAudio2);

	/// <summary>
	/// 音量設定
	/// </summary>
	/// <param name="volume"></param>
	void SetVolume(float volume);

	/// <summary>
	/// 音声の一時停止
	/// </summary>
	void Pause();

	/// <summary>
	/// 音声の再開（一時停止位置から）
	/// </summary>
	void Resume();

	/// <summary>
	/// 音声の完全停止
	/// </summary>
	void Stop();

	/// <summary>
	/// ループ設定の変更（再生中でも変更可能）
	/// </summary>
	/// <param name="loop">true: ループ再生, false: 通常再生</param>
	void SetLoop(bool loop);

	/// <summary>
	/// 音声データの解放
	/// </summary>
	void Unload();

	/// <summary>
	/// 再生中かどうか
	/// </summary>
	/// <returns></returns>
	bool IsPlaying();

	/// <summary>
	/// 一時停止中かどうか
	/// </summary>
	/// <returns></returns>
	bool IsPaused() const;

	/// <summary>
	/// ループ再生中かどうか
	/// </summary>
	/// <returns></returns>
	bool IsLooping() const;

private:
	/// <summary>
	/// 再生状態の更新（内部使用）
	/// </summary>
	void UpdatePlayState();

	// 音声データ
	SoundData soundData;
	// ソースボイス
	IXAudio2SourceVoice* pSourceVoice;
	// 再生中フラグ
	bool isPlaying;
	// 一時停止中フラグ
	bool isPaused;
	// ループ再生フラグ
	bool isLooping;
	// 一時停止時のサンプル位置
	UINT64 pausedSamplesPlayed;
};