#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "DirectXCommon.h"
#include "ParticleGroup.h"
#include "ParticleEmitter.h"
#include "BaseField.h"
#include "CameraController.h"
#include "ParticleCommon.h"


/// <summary>
/// パーティクルシステム（シングルトン）
/// <para>すべてのパーティクルグループとエミッターを一元管理</para>
/// </summary>
class ParticleSystem
{
public:
	// シングルトンインスタンス取得
	static ParticleSystem* GetInstance();

	// コピー・ムーブ禁止
	ParticleSystem(const ParticleSystem&) = delete;
	ParticleSystem& operator=(const ParticleSystem&) = delete;
	ParticleSystem(ParticleSystem&&) = delete;
	ParticleSystem& operator=(ParticleSystem&&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 全グループとエミッターの更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 全グループの描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui用のデバッグ表示
	/// </summary>
	void ImGui();

	/// <summary>
	/// パーティクルグループを作成
	/// </summary>
	/// <param name="groupName">グループ名（一意である必要がある）</param>
	/// <param name="modelTag">モデルのタグ名</param>
	/// <param name="maxParticles">最大パーティクル数</param>
	/// <param name="textureName">テクスチャ名</param>
	/// <param name="useBillboard">ビルボードを使用するか（デフォルト: true）</param>
	/// <returns>作成に成功したらtrue</returns>
	bool CreateGroup(const std::string& groupName, const std::string& modelTag,
		uint32_t maxParticles, const std::string& textureName = "", bool useBillboard = true);

	/// <summary>
	/// パーティクルグループを取得
	/// </summary>
	/// <param name="groupName">グループ名</param>
	/// <returns>グループへのポインタ（存在しない場合はnullptr）</returns>
	ParticleGroup* GetGroup(const std::string& groupName);

	/// <summary>
	/// パーティクルグループを削除
	/// </summary>
	/// <param name="groupName">グループ名</param>
	void RemoveGroup(const std::string& groupName);

	/// <summary>
	/// エミッターを作成
	/// </summary>
	/// <param name="emitterName">エミッター名</param>
	/// <param name="targetGroupName">ターゲットグループ名</param>
	/// <returns>作成されたエミッターへのポインタ</returns>
	ParticleEmitter* CreateEmitter(const std::string& emitterName, const std::string& targetGroupName);

	/// <summary>
	/// エミッターを削除
	/// </summary>
	/// <param name="emitterName">エミッター名</param>
	void RemoveEmitter(const std::string& emitterName);

	/// <summary>
	/// フィールドを作成（テンプレート）
	/// </summary>
	/// <typeparam name="FieldType">作成するフィールドの型</typeparam>
	/// <param name="fieldName">フィールド名</param>
	/// <returns>作成されたフィールドへのポインタ</returns>
	template<typename FieldType>
	FieldType* CreateField(const std::string& fieldName)
	{
		static_assert(std::is_base_of<BaseField, FieldType>::value, "FieldType must derive from BaseField");

		// 同じ名前のフィールドが既に存在するかチェック
		if (fields_.find(fieldName) != fields_.end()) {
			Logger::Log(Logger::GetStream(),
				std::format("ParticleSystem: Field '{}' already exists!\n", fieldName));
			return nullptr;
		}

		// 新しいフィールドを作成
		auto field = std::make_unique<FieldType>();
		field->Initialize(dxCommon_);
		field->SetName(fieldName);

		// フィールドを登録
		FieldType* fieldPtr = field.get();
		fields_[fieldName] = std::move(field);

		Logger::Log(Logger::GetStream(),
			std::format("ParticleSystem: Created field '{}' (type: {})\n", fieldName, fieldPtr->GetTypeName()));

		return fieldPtr;
	}

	/// <summary>
	/// フィールドを削除
	/// </summary>
	/// <param name="fieldName">フィールド名</param>
	void RemoveField(const std::string& fieldName);

	/// <summary>
	/// フィールド数を取得
	/// </summary>
	size_t GetFieldCount() const { return fields_.size(); }


	/// <summary>
	/// すべてのグループとエミッターをクリア
	/// </summary>
	void Clear();

	/// <summary>
	/// グループ数を取得
	/// </summary>
	size_t GetGroupCount() const { return groups_.size(); }

	/// <summary>
	/// エミッター数を取得
	/// </summary>
	size_t GetEmitterCount() const { return emitters_.size(); }

	/// <summary>
	/// エミッターを取得（ポインタ）
	/// </summary>
	ParticleEmitter* GetEmitter(const std::string& emitterName);

	/// <summary>
	/// フィールドを取得（ポインタ）
	/// </summary>
	BaseField* GetField(const std::string& fieldName);

	/// <summary>
	/// すべてのグループ名を取得
	/// </summary>
	std::vector<std::string> GetAllGroupNames() const;

	/// <summary>
	/// すべてのエミッター名を取得
	/// </summary>
	std::vector<std::string> GetAllEmitterNames() const;

	/// <summary>
	/// すべてのフィールド名を取得
	/// </summary>
	std::vector<std::string> GetAllFieldNames() const;

private:
	ParticleSystem() = default;
	~ParticleSystem() = default;

	/// <summary>
	/// ビルボード行列を計算
	/// </summary>
	void CalculateBillboardMatrix();

	// パーティクルグループ（グループ名 : グループ）
	std::unordered_map<std::string, std::unique_ptr<ParticleGroup>> groups_;

	// エミッター（エミッター名 : エミッター）
	std::unordered_map<std::string, std::unique_ptr<ParticleEmitter>> emitters_;

	// フィールド（フィールド名 : フィールド）
	std::unordered_map<std::string, std::unique_ptr<BaseField>> fields_;

	// ビルボード行列（全グループ共通）
	Matrix4x4 billboardMatrix_;

	// システム参照
	DirectXCommon* dxCommon_ = nullptr;
	CameraController* cameraController_ = CameraController::GetInstance();
	ParticleCommon* particleCommon_ = ParticleCommon::GetInstance();
};