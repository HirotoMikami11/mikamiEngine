#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "MyFunction.h"

// 前方宣言
class ParticleSystem;
class ParticleEmitter;
class ParticleGroup;
class BaseField;

/// <summary>
/// パーティクルプリセットから生成されたインスタンス
/// <para>プリセットを複製して個別に制御できるようにする</para>
/// </summary>
class ParticlePresetInstance
{
public:
	ParticlePresetInstance() = default;
	~ParticlePresetInstance() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="instanceName">インスタンス名</param>
	/// <param name="particleSystem">ParticleSystemへのポインタ</param>
	void Initialize(const std::string& instanceName, ParticleSystem* particleSystem);

	/// <summary>
	/// エミッターを取得（プリセット内のローカル名で指定）
	/// </summary>
	/// <param name="localName">プリセット内でのエミッター名</param>
	/// <returns>エミッターへのポインタ（存在しない場合nullptr）</returns>
	ParticleEmitter* GetEmitter(const std::string& localName);

	/// <summary>
	/// グループを取得（プリセット内のローカル名で指定）
	/// </summary>
	/// <param name="localName">プリセット内でのグループ名</param>
	/// <returns>グループへのポインタ（存在しない場合nullptr）</returns>
	ParticleGroup* GetGroup(const std::string& localName);

	/// <summary>
	/// フィールドを取得（プリセット内のローカル名で指定）
	/// </summary>
	/// <param name="localName">プリセット内でのフィールド名</param>
	/// <returns>フィールドへのポインタ（存在しない場合nullptr）</returns>
	BaseField* GetField(const std::string& localName);

	/// <summary>
	/// 型指定でフィールドを取得
	/// </summary>
	template<typename FieldType>
	FieldType* GetField(const std::string& localName)
	{
		return dynamic_cast<FieldType*>(GetField(localName));
	}

	/// <summary>
	/// 全エミッターの座標を一括オフセット
	/// </summary>
	/// <param name="offset">オフセット座標</param>
	void SetPositionOffset(const Vector3& offset);

	/// <summary>
	/// 全エミッターとフィールドの有効/無効を切り替え
	/// </summary>
	/// <param name="enabled">有効化するか</param>
	void SetEnabled(bool enabled);

	/// <summary>
	/// インスタンスを破棄（全グループ・エミッター・フィールドを削除）
	/// </summary>
	void Destroy();

	/// <summary>
	/// インスタンス名を取得
	/// </summary>
	const std::string& GetInstanceName() const { return instanceName_; }

	/// <summary>
	/// グループ名を登録
	/// </summary>
	void RegisterGroup(const std::string& localName, const std::string& uniqueName);

	/// <summary>
	/// エミッター名を登録
	/// </summary>
	void RegisterEmitter(const std::string& localName, const std::string& uniqueName);

	/// <summary>
	/// フィールド名を登録
	/// </summary>
	void RegisterField(const std::string& localName, const std::string& uniqueName);

	/// <summary>
	/// 破棄済みかチェック
	/// </summary>
	bool IsDestroyed() const { return isDestroyed_; }

private:
	std::string instanceName_;

	// ローカル名 → 実際の名前のマッピング
	std::unordered_map<std::string, std::string> groupNameMap_;
	std::unordered_map<std::string, std::string> emitterNameMap_;
	std::unordered_map<std::string, std::string> fieldNameMap_;

	ParticleSystem* particleSystem_ = nullptr;
	bool isDestroyed_ = false;
};