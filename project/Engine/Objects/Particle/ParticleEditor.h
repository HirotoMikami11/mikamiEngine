#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <filesystem>
#include "DirectXCommon.h"
#include "ParticleSystem.h"
#include "ParticlePreset.h"
#include "ParticlePresetInstance.h"

/// <summary>
/// パーティクルエディタ（シングルトン）
/// <para>プリセットの保存/読み込み、インスタンス管理、動的な作成/編集を行う</para>
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
	/// 選択されたオブジェクトのみをプリセットとして保存（フェーズ2）
	/// </summary>
	/// <param name="presetName">プリセット名</param>
	/// <returns>成功したらtrue</returns>
	bool SaveSelectedAsPreset(const std::string& presetName);

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
	/// 全てのインスタンスを削除　　　　　　
	/// </summary>
	/// <param name="instanceName"></param>
	void DestroyAllInstance();
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

	// ========================================
	// プリセット編集モード（新規）
	// ========================================

	/// <summary>
	/// プリセットを編集モードで開く
	/// </summary>
	/// <param name="presetName">編集するプリセット名</param>
	/// <returns>成功したらtrue</returns>
	bool OpenPresetForEdit(const std::string& presetName);

	/// <summary>
	/// 現在編集中のプリセットを保存（上書き）
	/// </summary>
	/// <returns>成功したらtrue</returns>
	bool SaveEditingPreset();

	/// <summary>
	/// 現在編集中のプリセットを別名で保存
	/// </summary>
	/// <param name="newPresetName">新しいプリセット名</param>
	/// <returns>成功したらtrue</returns>
	bool SaveEditingPresetAs(const std::string& newPresetName);

	/// <summary>
	/// プリセット編集モードを終了
	/// </summary>
	void ClosePresetEditor();

	/// <summary>
	/// 現在プリセット編集モードかどうか
	/// </summary>
	bool IsInPresetEditMode() const;

	/// <summary>
	/// 編集中のプリセット名を取得
	/// </summary>
	const std::string& GetEditingPresetName() const { return editingPresetName_; }

	// ========================================
	// インスタンス編集モード（既存を改良）
	// ========================================

	/// <summary>
	/// インスタンスからプリセットを保存（名前正規化を実施）
	/// </summary>
	/// <param name="instanceName">保存元のインスタンス名</param>
	/// <param name="presetName">保存先のプリセット名</param>
	/// <returns>成功したらtrue</returns>
	bool SaveInstanceAsPreset(const std::string& instanceName, const std::string& presetName);

	// ========================================
	// 選択操作（フェーズ2）
	// ========================================

	/// <summary>
	/// すべてのオブジェクトを選択
	/// </summary>
	void SelectAll();

	/// <summary>
	/// すべてのオブジェクトの選択を解除
	/// </summary>
	void DeselectAll();

	/// <summary>
	/// 選択中のオブジェクト数を取得
	/// </summary>
	/// <returns>選択中のオブジェクトの合計数</returns>
	size_t GetSelectedCount() const;

	/// <summary>
	/// 特定のグループが選択されているか確認
	/// </summary>
	bool IsGroupSelected(const std::string& groupName) const;

	/// <summary>
	/// 特定のエミッターが選択されているか確認
	/// </summary>
	bool IsEmitterSelected(const std::string& emitterName) const;

	/// <summary>
	/// 特定のフィールドが選択されているか確認
	/// </summary>
	bool IsFieldSelected(const std::string& fieldName) const;

private:
	ParticleEditor() = default;
	~ParticleEditor() = default;

	// ========================================
	// UI表示関数
	// ========================================

	/// <summary>
	/// Createタブの表示
	/// </summary>
	void ShowCreateTab();

	/// <summary>
	/// Editタブの表示
	/// </summary>
	void ShowEditTab();

	/// <summary>
	/// Preset Managerタブの表示
	/// </summary>
	void ShowPresetManagerTab();

	/// <summary>
	/// Instancesタブの表示
	/// </summary>
	void ShowInstancesTab();

	/// <summary>
	/// Presetsタブの表示（旧UI、互換性のため残す）
	/// </summary>
	void ShowPresetsTab();

	// ========================================
	// Create機能
	// ========================================

	/// <summary>
	/// グループ作成ダイアログ
	/// </summary>
	void ShowCreateGroupDialog();

	/// <summary>
	/// エミッター作成ダイアログ
	/// </summary>
	void ShowCreateEmitterDialog();

	/// <summary>
	/// フィールド作成ダイアログ
	/// </summary>
	void ShowCreateFieldDialog();

	// ========================================
	// Edit機能
	// ========================================

	/// <summary>
	/// オブジェクトリストパネルの表示
	/// </summary>
	void ShowObjectListPanel();

	/// <summary>
	/// 編集パネルの表示
	/// </summary>
	void ShowEditPanel();

	/// <summary>
	/// グループリストの表示
	/// </summary>
	void ShowGroupList();

	/// <summary>
	/// エミッターリストの表示
	/// </summary>
	void ShowEmitterList();

	/// <summary>
	/// フィールドリストの表示
	/// </summary>
	void ShowFieldList();

	// ========================================
	// ユーティリティ
	// ========================================

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

	/// <summary>
	/// グループデータを作成
	/// </summary>
	ParticleGroupData CreateGroupData(ParticleGroup* group) const;

	/// <summary>
	/// エミッターデータを作成
	/// </summary>
	ParticleEmitterData CreateEmitterData(ParticleEmitter* emitter) const;

	/// <summary>
	/// フィールドデータを作成
	/// </summary>
	ParticleFieldData CreateFieldData(BaseField* field) const;

	/// <summary>
	/// オブジェクト名からインスタンスプレフィックスを除去
	/// </summary>
	/// <param name="objectName">元の名前（例：「a_FireEmitter」）</param>
	/// <param name="instanceName">インスタンス名（例：「a」）</param>
	/// <returns>正規化された名前（例：「FireEmitter」）</returns>
	std::string RemoveInstancePrefix(const std::string& objectName, const std::string& instanceName) const;

	/// <summary>
	/// プリセットデータの名前を正規化（インスタンスプレフィックスを除去）
	/// </summary>
	/// <param name="data">プリセットデータ</param>
	/// <param name="instanceName">除去するインスタンス名</param>
	void NormalizePresetNames(ParticlePresetData& data, const std::string& instanceName) const;

	// ========================================
	// メンバ変数
	// ========================================

	// プリセットディレクトリ
	static constexpr const char* kPresetDirectory_ = "resources/ParticlePresets/";

	// プリセット編集用の特別なインスタンス名
	static constexpr const char* kPresetEditInstanceName_ = "__PRESET_EDITOR__";

	// エディタモード
	enum class EditorMode {
		Instance,    // インスタンス編集モード（既存）
		Preset       // プリセット編集モード（新規）
	};
	EditorMode currentMode_ = EditorMode::Instance;
	std::string editingPresetName_;  // プリセット編集モード時のプリセット名

	// インスタンス管理
	std::unordered_map<std::string, std::unique_ptr<ParticlePresetInstance>> instances_;

	// 編集状態
	enum class EditingType { None, Group, Emitter, Field };
	EditingType currentEditingType_ = EditingType::None;
	std::string currentEditingObject_;

	// 選択状態（フェーズ2）
	std::set<std::string> selectedGroups_;
	std::set<std::string> selectedEmitters_;
	std::set<std::string> selectedFields_;

	// Create Group用の一時データ
	struct GroupCreationData {
		char name[128] = "NewGroup";
		char modelTag[128] = "plane";
		char textureName[128] = "circle";
		int maxParticles = 100;
		bool useBillboard = true;
	};
	GroupCreationData groupCreationData_;
	bool showCreateGroupDialog_ = false;

	// Create Emitter用の一時データ
	struct EmitterCreationData {
		char name[128] = "NewEmitter";
		char targetGroup[128] = "";
		int selectedGroupIndex = 0;
	};
	EmitterCreationData emitterCreationData_;
	bool showCreateEmitterDialog_ = false;

	// Create Field用の一時データ
	struct FieldCreationData {
		char name[128] = "NewField";
		int fieldTypeIndex = 0;
		const char* fieldTypes[2] = { "AccelerationField", "GravityField" };
	};
	FieldCreationData fieldCreationData_;
	bool showCreateFieldDialog_ = false;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	ParticleSystem* particleSystem_ = nullptr;
};