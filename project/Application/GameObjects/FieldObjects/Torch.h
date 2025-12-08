#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"
#include "ParticleEditor.h"
#include <array>

/// <summary>
/// トーチクラス
/// 12個のトーチオブジェクトを管理し、個別に位置・角度を設定可能
/// </summary>
class Torch
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Torch();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Torch();

	/// <summary>
	/// 初期化処理
	/// 12個のトーチモデルを生成し、JsonSettingsから設定を読み込む
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// 各トーチの行列を更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// 12個のトーチを描画
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// トーチのパラメータ調整とJsonSettings保存機能
	/// </summary>
	void ImGui();

	/// <summary>
	/// 現在の設定をJsonファイルに保存
	/// </summary>
	void SaveToJson();

	/// <summary>
	/// JsonSettingsから設定を読み込んで適用
	/// </summary>
	void ApplyParameters();

private:
	/// <summary>
	/// トーチオブジェクトの構造体
	/// 各トーチのモデルとトランスフォーム情報を格納
	/// </summary>
	struct TorchObject {
		std::unique_ptr<Model3D> obj;
		Vector3Transform transform;
	};

	/// <summary>
	/// 各トーチのトランスフォームを更新
	/// </summary>
	void UpdateTransforms();


	// システム参照
	DirectXCommon* dxCommon_;

	// 12個のトーチ
	static constexpr int kTorchCount_ = 12;
	std::array<TorchObject, kTorchCount_> torches_;

	// 全体のスケール（全トーチ共通）
	Vector3 globalScale_;
	// 全体の色（全トーチ共通）
	Vector4 globalColor_;

	// 各トーチの座標（個別設定）
	std::array<Vector3, kTorchCount_> positions_;

	// 各トーチの回転（個別設定）
	std::array<Vector3, kTorchCount_> rotations_;

	//torchのちらつきイージング用
	PointLight* torchLights_[kTorchCount_];
	std::array<float, kTorchCount_> flickerTime_;
	std::array<float, kTorchCount_> flickerSpeed_;
	std::array<float, kTorchCount_> flickerOffset_;
	float baseIntensity_ = 1.5f;
	float flickerAmount_ = 0.3f;

	// JsonSettingsのグループパス
	const std::vector<std::string> kGroupPath_ = { "Torch" };
	ParticlePresetInstance* particleInstance_[12];
};