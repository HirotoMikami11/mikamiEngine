#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <filesystem>
#include "DirectXCommon.h"
#include "ParticleSystem.h"
#include "ParticlePreset.h"
#include "ParticlePresetInstance.h"

/// <summary>
/// パーティクルエディタ（シングルトン）
/// <para>プリセットの保存/読み込み、インスタンス管理を行う</para>
/// </summary>
class ParticleEditor
{
public:
	// シングルトンインスタンス取得
	static ParticleEditor* GetInstance();

	// コピー・ムーブ禁止
	ParticleEditor(const ParticleEditor&) = delete;
	ParticleEditor& operator=(const ParticleEditor&) = delete;
	ParticleEditor(ParticleEditor&&) = delete;
	ParticleEditor& operator=(ParticleEditor&&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// ImGui用のエディタUI
	/// </summary>
	void ImGui();

	/// <summary>
	/// 現在のParticleSystemの状態をプリセットとして保存
	/// </summary>
	/// <param name="presetName">プリセット名</param>
	/// <returns>成功したらtrue</returns>
	bool SaveCurrentStateAsPreset(const std::string& presetName);

	/// <summary>
	/// プリセットからインスタンスを作成
	/// </summary>
	/// <param name="presetName">プリセット名</param>
	/// <param name="instanceName">インスタンス名（一意である必要がある）</param>
	/// <returns>作成されたインスタンスへのポインタ（失敗時nullptr）</returns>
	ParticlePresetInstance* CreateInstance(const std::string& presetName, const std::string& instanceName);

	/// <summary>
	/// インスタンスを取得
	/// </summary>
	/// <param name="instanceName">インスタンス名</param>
	/// <returns>インスタンスへのポインタ（存在しない場合nullptr）</returns>
	ParticlePresetInstance* GetInstance(const std::string& instanceName);

	/// <summary>
	/// インスタンスを削除
	/// </summary>
	/// <param name="instanceName">インスタンス名</param>
	void DestroyInstance(const std::string& instanceName);

	/// <summary>
	/// プリセットを読み込み
	/// </summary>
	/// <param name="presetName">プリセット名</param>
	/// <returns>読み込まれたプリセットデータ</returns>
	ParticlePresetData LoadPreset(const std::string& presetName);

	/// <summary>
	/// プリセットを削除
	/// </summary>
	/// <param name="presetName">プリセット名</param>
	bool DeletePreset(const std::string& presetName);

	/// <summary>
	/// 利用可能なプリセット一覧を取得
	/// </summary>
	std::vector<std::string> GetAvailablePresets() const;

private:
	ParticleEditor() = default;
	~ParticleEditor() = default;

	/// <summary>
	/// プリセットファイルのパスを取得
	/// </summary>
	std::string GetPresetFilePath(const std::string& presetName) const;

	/// <summary>
	/// 名前を一意化（インスタンス名 + ローカル名）
	/// </summary>
	std::string MakeUniqueName(const std::string& instanceName, const std::string& localName) const;

	/// <summary>
	/// フィールドを作成（型名から動的に作成）
	/// </summary>
	BaseField* CreateFieldByTypeName(const std::string& typeName, const std::string& fieldName);

	// プリセットディレクトリ
	static constexpr const char* kPresetDirectory_ = "resources/ParticlePresets/";

	// インスタンス管理
	std::unordered_map<std::string, std::unique_ptr<ParticlePresetInstance>> instances_;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	ParticleSystem* particleSystem_ = nullptr;
};