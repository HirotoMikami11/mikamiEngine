#define NOMINMAX
#include "AudioManager.h"
#include "ImGui/ImGuiManager.h"

// シングルトンインスタンス
AudioManager* AudioManager::GetInstance() {
	static AudioManager instance;
	return &instance;
}

AudioManager::AudioManager() : masterVoice(nullptr), masterVolume(1.0f), nextInstanceId(1) {
}

AudioManager::~AudioManager() {
	Finalize();
}

void AudioManager::Initialize() {
	HRESULT result;

	//MediaFoundationのCOMの初期化
	result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	assert(SUCCEEDED(result));
	Logger::Log(Logger::GetStream(), "Complete MFCOM initialized !!\n");

	//MediaFoundationのスタートアップ
	result = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(result));
	Logger::Log(Logger::GetStream(), "Complete MF initialized !!\n");

	//DirectX初期化の末尾にXAudio2エンジンのインスタンス生成
	result = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(result));

	// マスターボイスの生成
	result = xAudio2->CreateMasteringVoice(&masterVoice);
	assert(SUCCEEDED(result));
	Logger::Log(Logger::GetStream(), "Complete AudioManager initialized !!\n");
}

void AudioManager::Finalize() {
	// 全てのインスタンスを解放
	instanceMap.clear();
	// 全ての音声データを解放
	audioDataMap.clear();

	// マスターボイスの解放
	if (masterVoice) {
		masterVoice->DestroyVoice();
		masterVoice = nullptr;
	}

	// XAudio2の解放
	xAudio2.Reset();

	// MediaFoundationの終了処理
	MFShutdown();
	Logger::Log(Logger::GetStream(), "MediaFoundation shutdown\n");

	// COMの終了処理
	CoUninitialize();
	Logger::Log(Logger::GetStream(), "COM uninitialized\n");
}

void AudioManager::Update() {
	// 全てのインスタンスの状態を更新
	for (auto& pair : instanceMap) {
		if (pair.second) {
			pair.second->Update();
		}
	}

	// 再生完了したインスタンスを削除
	CleanupFinishedInstances();
}

void AudioManager::LoadAudio(const std::string& filename, const std::string& tagName) {
	// 既に同じタグ名で登録されていた場合は古いものを解放
	if (audioDataMap.find(tagName) != audioDataMap.end()) {
		audioDataMap.erase(tagName);
	}

	// 新しい音声データを作成
	auto audioData = std::make_unique<AudioData>();
	audioData->LoadFromFile(filename);

	// mapに移動（所有権の移動）
	audioDataMap[tagName] = std::move(audioData);
}

int AudioManager::Play(const std::string& tagName, bool isLoop, float volume) {
	// 指定したタグ名の音声データが見つからなければ失敗
	auto it = audioDataMap.find(tagName);
	if (it == audioDataMap.end()) {
		return 0;  // 失敗
	}

	// 新しいインスタンスIDを生成
	int instanceId = GenerateInstanceId();

	// マスターボリュームを考慮した音量
	float actualVolume = volume * masterVolume;

	// unique_ptrから生ポインタを取得して渡す（AudioInstanceは参照のみ保持）
	auto instance = std::make_unique<AudioInstance>(
		xAudio2.Get(),
		it->second.get(),
		instanceId,
		isLoop,
		actualVolume
	);

	// 再生開始
	instance->Start();

	// インスタンスマップに移動（所有権の移動）
	instanceMap[instanceId] = std::move(instance);

	return instanceId;
}

int AudioManager::PlayOverride(const std::string& tagName, bool isLoop, float volume) {
	// 既存の同タグインスタンスを全て停止
	StopByTag(tagName);

	// 新しいインスタンスを再生
	return Play(tagName, isLoop, volume);
}

void AudioManager::Pause(int instanceId) {
	// インスタンスが存在しなければ何もしない
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return;
	}

	it->second->Pause();
}

void AudioManager::Resume(int instanceId) {
	// インスタンスが存在しなければ何もしない
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return;
	}

	it->second->Resume();
}

void AudioManager::Stop(int instanceId) {
	// インスタンスが存在しなければ何もしない
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return;
	}

	// 停止してインスタンスを削除
	it->second->Stop();
	instanceMap.erase(it);
}

void AudioManager::SetLoop(int instanceId, bool loop) {
	// インスタンスが存在しなければ何もしない
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return;
	}

	it->second->SetLoop(loop);
}

void AudioManager::SetVolume(int instanceId, float volume) {
	// インスタンスが存在しなければ何もしない
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return;
	}

	// マスターボリュームを考慮した音量を設定
	it->second->SetVolumeWithMaster(volume, masterVolume);
}

void AudioManager::SetMasterVolume(float volume) {
	// 0.0f～1.0fの範囲にクランプ
	masterVolume = std::max(0.0f, std::min(1.0f, volume));

	// 全てのインスタンスの音量を更新
	UpdateAllVolumes();
}

void AudioManager::StopAll() {
	// 全てのインスタンスを停止
	for (auto& pair : instanceMap) {
		if (pair.second) {
			pair.second->Stop();
		}
	}
	// clear()でunique_ptrが自動的にdeleteを呼ぶ
	instanceMap.clear();
}

void AudioManager::PauseByTag(const std::string& tagName) {
	// 指定したタグ名の音声データが見つからなければ何もしない
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return;
	}

	const AudioData* targetData = dataIt->second.get();

	// 該当する全てのインスタンスを一時停止
	for (auto& pair : instanceMap) {
		if (pair.second && pair.second->GetAudioData() == targetData) {
			pair.second->Pause();
		}
	}
}

void AudioManager::ResumeByTag(const std::string& tagName) {
	// 指定したタグ名の音声データが見つからなければ何もしない
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return;
	}

	const AudioData* targetData = dataIt->second.get();

	// 該当する全てのインスタンスを再開
	for (auto& pair : instanceMap) {
		if (pair.second && pair.second->GetAudioData() == targetData) {
			pair.second->Resume();
		}
	}
}

void AudioManager::StopByTag(const std::string& tagName) {
	// 指定したタグ名の音声データが見つからなければ何もしない
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return;
	}


	const AudioData* targetData = dataIt->second.get();

	// 該当する全てのインスタンスを停止して削除
	auto it = instanceMap.begin();
	while (it != instanceMap.end()) {
		if (it->second && it->second->GetAudioData() == targetData) {
			it->second->Stop();
			it = instanceMap.erase(it);
		} else {
			++it;
		}
	}
}

void AudioManager::SetLoopByTag(const std::string& tagName, bool loop) {
	// 指定したタグ名の音声データが見つからなければ何もしない
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return;
	}

	const AudioData* targetData = dataIt->second.get();

	// 該当する全てのインスタンスのループ設定を変更
	for (auto& pair : instanceMap) {
		if (pair.second && pair.second->GetAudioData() == targetData) {
			pair.second->SetLoop(loop);
		}
	}
}

void AudioManager::SetVolumeByTag(const std::string& tagName, float volume) {
	// 指定したタグ名の音声データが見つからなければ何もしない
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return;
	}

	const AudioData* targetData = dataIt->second.get();

	// 該当する全てのインスタンスの音量を変更
	for (auto& pair : instanceMap) {
		if (pair.second && pair.second->GetAudioData() == targetData) {
			pair.second->SetVolumeWithMaster(volume, masterVolume);
		}
	}
}

bool AudioManager::HasInstance(int instanceId) const {
	return instanceMap.find(instanceId) != instanceMap.end();
}

bool AudioManager::HasAudioData(const std::string& tagName) const {
	return audioDataMap.find(tagName) != audioDataMap.end();
}

bool AudioManager::IsPlaying(int instanceId) const {
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return false;
	}
	return it->second->IsPlaying();
}

bool AudioManager::IsPaused(int instanceId) const {
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return false;
	}
	return it->second->IsPaused();
}

bool AudioManager::IsLooping(int instanceId) const {
	auto it = instanceMap.find(instanceId);
	if (it == instanceMap.end()) {
		return false;
	}
	return it->second->IsLooping();
}

bool AudioManager::IsPlayingByTag(const std::string& tagName) const {
	// 指定したタグ名の音声データが見つからなければfalse
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return false;
	}

	const AudioData* targetData = dataIt->second.get();

	// 該当するインスタンスが1つでも再生中ならtrue
	for (const auto& pair : instanceMap) {
		if (pair.second && pair.second->GetAudioData() == targetData && pair.second->IsPlaying()) {
			return true;
		}
	}

	return false;
}

int AudioManager::GetInstanceCountByTag(const std::string& tagName) const {
	// 指定したタグ名の音声データが見つからなければ0
	auto dataIt = audioDataMap.find(tagName);
	if (dataIt == audioDataMap.end()) {
		return 0;
	}

	const AudioData* targetData = dataIt->second.get();
	int count = 0;

	// 該当するインスタンスをカウント
	for (const auto& pair : instanceMap) {
		if (pair.second && pair.second->GetAudioData() == targetData) {
			count++;
		}
	}

	return count;
}

void AudioManager::UpdateAllVolumes() {
	// 全てのインスタンスの音量をマスターボリュームに合わせて更新
	for (auto& pair : instanceMap) {
		if (pair.second) {
			float instanceVolume = pair.second->GetVolume();
			pair.second->SetVolumeWithMaster(instanceVolume, masterVolume);
		}
	}
}

void AudioManager::CleanupFinishedInstances() {
	// 再生完了したインスタンスを削除
	auto it = instanceMap.begin();
	while (it != instanceMap.end()) {
		if (it->second && it->second->IsFinished()) {
			it = instanceMap.erase(it);
		} else {
			++it;
		}
	}
}

int AudioManager::GenerateInstanceId() {
	return nextInstanceId++;
}

std::string AudioManager::GetTagNameFromInstance(int instanceId) const {
	// インスタンスが存在しなければ空文字列を返す
	auto instIt = instanceMap.find(instanceId);
	if (instIt == instanceMap.end()) {
		return "";
	}

	const AudioData* targetData = instIt->second->GetAudioData();

	// AudioDataからタグ名を逆引き
	for (const auto& pair : audioDataMap) {
		if (pair.second.get() == targetData) {
			return pair.first;
		}
	}

	return "";
}

void AudioManager::ImGui() {
#ifdef USEIMGUI

	// マスターボリューム調整
	ImGui::Text("Master Volume:");
	ImGui::SameLine();
	ImGui::PushItemWidth(150);
	if (ImGui::SliderFloat("##MasterVolume", &masterVolume, 0.0f, 1.0f, "%.2f")) {
		UpdateAllVolumes();
	}
	ImGui::PopItemWidth();

	// マスターボリュームのプリセットボタン
	ImGui::SameLine();
	if (ImGui::SmallButton("25%##Master")) {
		SetMasterVolume(0.25f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("50%##Master")) {
		SetMasterVolume(0.5f);
	}
	ImGui::SameLine();
	if (ImGui::SmallButton("100%##Master")) {
		SetMasterVolume(1.0f);
	}

	ImGui::Separator();

	// 統計情報
	ImGui::Text("Audio Data: %zu", audioDataMap.size());
	ImGui::SameLine(150);
	ImGui::Text("Active Instances: %zu", instanceMap.size());

	// 全停止ボタン
	ImGui::SameLine();
	if (ImGui::SmallButton("Stop All")) {
		StopAll();
	}

	// 音声データが1つも読み込まれていない場合
	if (audioDataMap.empty()) {
		ImGui::TextDisabled("No audio data loaded");
		return;
	}

	ImGui::Separator();

	// 音声データ一覧
	ImGui::Text("Audio Data List:");

	for (const auto& dataPair : audioDataMap) {
		const std::string& tagName = dataPair.first;

		// このデータのインスタンス数をカウント
		int instanceCount = 0;
		for (const auto& instPair : instanceMap) {
			if (instPair.second && instPair.second->GetAudioData() == dataPair.second.get()) {
				instanceCount++;
			}
		}

		// データ名とインスタンス数を表示
		ImGui::PushID(tagName.c_str());
		ImGui::BulletText("%s (Instances: %d)", tagName.c_str(), instanceCount);

		// 再生ボタン
		ImGui::SameLine(250);
		if (ImGui::SmallButton("Play")) {
			Play(tagName, false);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("PlayOverride")) {
			PlayOverride(tagName, false);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Loop")) {
			Play(tagName, true);
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("LoopOverride")) {
			PlayOverride(tagName, true);
		}
		// このデータの全インスタンスを停止
		if (instanceCount > 0) {
			ImGui::SameLine();
			if (ImGui::SmallButton("Stop All")) {
				StopByTag(tagName);
			}
		}

		ImGui::PopID();
	}

	// アクティブなインスタンスが存在する場合
	if (!instanceMap.empty()) {
		ImGui::Separator();
		ImGui::Text("Active Instances:");

		// インスタンスごとの詳細表示
		std::vector<int> instancesToRemove;  // 削除対象

		for (const auto& pair : instanceMap) {
			int instanceId = pair.first;
			AudioInstance* instance = pair.second.get();

			if (!instance) continue;

			// タグ名を取得
			std::string tagName = GetTagNameFromInstance(instanceId);

			ImGui::PushID(instanceId);

			// インスタンスID表示
			ImGui::Text("ID:%d", instanceId);
			ImGui::SameLine(75);

			// タグ名表示
			ImGui::Text("%s", tagName.c_str());
			ImGui::SameLine(200);

			// 状態表示
			if (instance->IsPlaying()) {
				if (instance->IsPaused()) {
					ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[Paused]");
				} else if (instance->IsLooping()) {
					ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "[Looping]");
				} else {
					ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.0f, 1.0f), "[Playing]");
				}
			} else {
				ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "[Stopped]");
			}

			ImGui::SameLine(300);

			// コントロールボタン
			if (instance->IsPaused()) {
				if (ImGui::SmallButton("Resume")) {
					Resume(instanceId);
				}
			} else {
				if (ImGui::SmallButton("Pause")) {
					Pause(instanceId);
				}
			}

			ImGui::SameLine();
			if (ImGui::SmallButton("Stop")) {
				instancesToRemove.push_back(instanceId);
			}

			// ループチェックボックス
			ImGui::SameLine();
			bool isLooping = instance->IsLooping();
			if (ImGui::Checkbox("Loop", &isLooping)) {
				SetLoop(instanceId, isLooping);
			}

			// 音量スライダー
			ImGui::SameLine();
			float volume = instance->GetVolume();
			ImGui::PushItemWidth(100);
			if (ImGui::SliderFloat("##Vol", &volume, 0.0f, 1.0f, "%.2f")) {
				SetVolume(instanceId, volume);
			}
			ImGui::PopItemWidth();

			ImGui::PopID();
		}

		// 停止対象のインスタンスを削除
		for (int id : instancesToRemove) {
			Stop(id);
		}
	}

#endif
}