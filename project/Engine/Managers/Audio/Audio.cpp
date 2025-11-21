#include "Audio.h"

///*===========================================================================*///
///								AudioData 実装
///*===========================================================================*///

AudioData::AudioData() {
	soundData = {};
}

AudioData::~AudioData() {
	Unload();
}

void AudioData::LoadFromFile(const std::string& filename) {
	HRESULT hr = S_OK;

	///*-----------------------------------------------------------------------*///
	///								ソースリーダーの作成							///
	///*-----------------------------------------------------------------------*///
	// ファイル名をワイド文字列に変換
	int wideLength = MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, nullptr, 0);
	std::wstring wfilename(wideLength - 1, L'\0');
	MultiByteToWideChar(CP_UTF8, 0, filename.c_str(), -1, &wfilename[0], wideLength);

	// ソースリーダーの実体作成
	Microsoft::WRL::ComPtr<IMFSourceReader> pMFSourceReader;
	hr = MFCreateSourceReaderFromURL(wfilename.c_str(), nullptr, &pMFSourceReader);
	assert(SUCCEEDED(hr));

	///*-----------------------------------------------------------------------*///
	///								メディアタイプの取得							///
	///*-----------------------------------------------------------------------*///
	// 出力メディアタイプの設定（PCM形式）
	Microsoft::WRL::ComPtr<IMFMediaType> pMFMediaType;
	hr = MFCreateMediaType(&pMFMediaType);
	assert(SUCCEEDED(hr));

	// PCMフォーマットを設定
	pMFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	pMFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	hr = pMFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, pMFMediaType.Get());
	assert(SUCCEEDED(hr));

	// 実際のメディアタイプを取得
	Microsoft::WRL::ComPtr<IMFMediaType> pActualMediaType;
	hr = pMFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &pActualMediaType);
	assert(SUCCEEDED(hr));

	///*-----------------------------------------------------------------------*///
	///							オーディオデータ形式の作成							///
	///*-----------------------------------------------------------------------*///
	// WAVEFORMATEXを取得
	WAVEFORMATEX* pWaveFormat{ nullptr };
	hr = MFCreateWaveFormatExFromMFMediaType(pActualMediaType.Get(), &pWaveFormat, nullptr);
	assert(SUCCEEDED(hr));

	// WAVEFORMATEXをコピー
	soundData.wfex = *pWaveFormat;
	CoTaskMemFree(pWaveFormat);

	///*-----------------------------------------------------------------------*///
	///								データの読み込み								///
	///*-----------------------------------------------------------------------*///
	// 音声データを読み込む
	std::vector<BYTE> mediaData;
	DWORD streamIndex = 0;
	DWORD dwStreamFlags = 0;
	LONGLONG timestamp = 0;

	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> pSample;

		hr = pMFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, &streamIndex, &dwStreamFlags, &timestamp, &pSample);

		if (FAILED(hr)) {
			break;
		}

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		Microsoft::WRL::ComPtr<IMFMediaBuffer> pMFMediaBuffer;
		hr = pSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

		if (SUCCEEDED(hr)) {
			BYTE* pBuffer = nullptr;
			DWORD cbCurrentLength = 0;
			hr = pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

			if (SUCCEEDED(hr)) {
				// データを追加
				size_t currentSize = mediaData.size();
				mediaData.resize(currentSize + cbCurrentLength);
				memcpy(mediaData.data() + currentSize, pBuffer, cbCurrentLength);
				pMFMediaBuffer->Unlock();
			}
		}
	}

	///*-----------------------------------------------------------------------*///
	///							データをメンバ変数に渡す							///
	///*-----------------------------------------------------------------------*///
	soundData.bufferSize = static_cast<unsigned int>(mediaData.size());
	soundData.pBuffer = new BYTE[soundData.bufferSize];
	memcpy(soundData.pBuffer, mediaData.data(), soundData.bufferSize);
}

void AudioData::Unload() {
	// バッファのメモリを解放
	if (soundData.pBuffer) {
		delete[] soundData.pBuffer;
		soundData.pBuffer = nullptr;
	}

	soundData.bufferSize = 0;
	soundData.wfex = {};
}


///*===========================================================================*///
///								AudioInstance 実装
///*===========================================================================*///

AudioInstance::AudioInstance(IXAudio2* xAudio2, const AudioData* audioData, int instanceId, bool isLoop, float volume)
	: instanceId(instanceId)
	, pSourceVoice(nullptr)
	, pAudioData(audioData)
	, isPlaying(false)
	, isPaused(false)
	, isLooping(isLoop)
	, isFinished(false)
	, currentVolume(volume)
	, pausedSamplesPlayed(0) {

	HRESULT result;

	// 波型フォーマットをもとにSourceVoiceを生成
	const SoundData& soundData = audioData->GetSoundData();
	result = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(result));

	// 音量を設定
	pSourceVoice->SetVolume(volume);

	// 再生する波型データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// ループ設定
	if (isLoop) {
		buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// バッファを送信（まだ再生は開始しない）
	result = pSourceVoice->SubmitSourceBuffer(&buf);
	assert(SUCCEEDED(result));
}

AudioInstance::~AudioInstance() {
	// ソースボイスの解放
	if (pSourceVoice) {
		pSourceVoice->DestroyVoice();
		pSourceVoice = nullptr;
	}
}

void AudioInstance::Start() {
	if (pSourceVoice && !isPlaying) {
		HRESULT result = pSourceVoice->Start();
		assert(SUCCEEDED(result));
		isPlaying = true;
		isPaused = false;
	}
}

void AudioInstance::Pause() {
	if (pSourceVoice && isPlaying && !isPaused) {
		// 現在の再生位置を保存
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		pausedSamplesPlayed = state.SamplesPlayed;

		// 一時停止
		pSourceVoice->Stop();
		isPaused = true;
	}
}

void AudioInstance::Resume() {
	if (pSourceVoice && isPlaying && isPaused) {
		// 一時停止位置から再開
		pSourceVoice->Start();
		isPaused = false;
	}
}

void AudioInstance::Stop() {
	if (pSourceVoice) {
		pSourceVoice->Stop();
		pSourceVoice->FlushSourceBuffers();
		isPlaying = false;
		isPaused = false;
		isFinished = true;  // 停止されたら削除対象
		pausedSamplesPlayed = 0;
	}
}

void AudioInstance::SetVolume(float volume) {
	currentVolume = volume;
	if (pSourceVoice) {
		pSourceVoice->SetVolume(currentVolume);
	}
}

void AudioInstance::SetVolumeWithMaster(float volume, float masterVolume) {
	currentVolume = volume;
	float actualVolume = volume * masterVolume;
	if (pSourceVoice) {
		pSourceVoice->SetVolume(actualVolume);
	}
}

void AudioInstance::SetLoop(bool loop) {
	if (pSourceVoice && isPlaying) {
		// 再生中の場合、一旦停止して再設定
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);
		UINT64 currentSample = state.SamplesPlayed;

		// 現在の音量を保存
		float currentVol = currentVolume;

		// 一旦停止
		pSourceVoice->Stop();
		pSourceVoice->FlushSourceBuffers();

		// 新しい設定でバッファを再設定
		const SoundData& soundData = pAudioData->GetSoundData();
		XAUDIO2_BUFFER buf{};
		buf.pAudioData = soundData.pBuffer;
		buf.AudioBytes = soundData.bufferSize;
		buf.Flags = XAUDIO2_END_OF_STREAM;

		if (loop) {
			buf.LoopCount = XAUDIO2_LOOP_INFINITE;
		}

		// 再生位置を調整（可能な範囲で）
		UINT32 bytesPerSample = soundData.wfex.wBitsPerSample / 8 * soundData.wfex.nChannels;
		UINT32 totalSamples = soundData.bufferSize / bytesPerSample;
		if (totalSamples > 0) {
			UINT32 playBegin = static_cast<UINT32>(currentSample % totalSamples);
			buf.PlayBegin = playBegin;
		}

		// バッファを再送信
		pSourceVoice->SubmitSourceBuffer(&buf);

		// 音量を復元
		pSourceVoice->SetVolume(currentVol);

		// 再生再開
		pSourceVoice->Start();

		isLooping = loop;
	} else {
		// 再生していない場合は単にフラグを設定
		isLooping = loop;
	}
}

void AudioInstance::Update() {
	if (pSourceVoice && isPlaying && !isPaused) {
		XAUDIO2_VOICE_STATE state;
		pSourceVoice->GetState(&state);

		// バッファが空で、キューに何もない場合は再生終了
		if (state.BuffersQueued == 0 && !isLooping) {
			isPlaying = false;
			isFinished = true;
		}
	}
}