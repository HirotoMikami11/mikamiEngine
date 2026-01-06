#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"
#include "ParticleEditor.h"
#include "TitleWall.h"
#include <array>

/// <summary>
/// タイトルシーンのトーチクラス
/// TitleWallに配置される8個のトーチを管理
/// 右側の壁に4本（0~3）、左側の壁に4本（4~7）
/// </summary>
class TitleTorch
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TitleTorch();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleTorch();

	/// <summary>
	/// 初期化処理
	/// TitleWallの位置を参照してトーチを自動配置
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="titleWall">参照するTitleWallのポインタ（位置情報取得用）</param>
	/// <param name="segmentIndex">セグメントのインデックス（パーティクル名の一意性確保用）</param>
	void Initialize(DirectXCommon* dxCommon, TitleWall* titleWall, int segmentIndex);

	/// <summary>
	/// 更新処理
	/// ちらつき効果とライト更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
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

	/// <summary>
	/// Z座標オフセットを設定（セグメント移動時に使用）
	/// </summary>
	/// <param name="zOffset">Z座標のオフセット値</param>
	void SetZOffset(float zOffset);

private:
	/// <summary>
	/// トーチオブジェクトの構造体
	/// </summary>
	struct TorchObject {
		std::unique_ptr<Model3D> obj;
		Vector3Transform transform;
	};

	/// <summary>
	/// 各トーチのトランスフォームを更新
	/// </summary>
	void UpdateTransforms();

	/// <summary>
	/// TitleWallの位置情報から自動的にトーチ位置を計算
	/// </summary>
	void AutoPlaceTorches();

	// システム参照
	DirectXCommon* dxCommon_;
	TitleWall* titleWall_;

	// 8個のトーチ
	static constexpr int kTorchCount_ = 8;
	std::array<TorchObject, kTorchCount_> torches_;

	// 全体のスケール（全トーチ共通）
	Vector3 globalScale_;
	// 全体の色（全トーチ共通）
	Vector4 globalColor_;

	// 各トーチの座標（個別設定）
	std::array<Vector3, kTorchCount_> positions_;

	// 各トーチの回転（個別設定）
	std::array<Vector3, kTorchCount_> rotations_;

	// Z座標オフセット（セグメント移動用）
	float zOffset_ = 0.0f;

	// セグメントインデックス（パーティクル名の一意性確保用）
	int segmentIndex_ = 0;

	// トーチのちらつき効果用
	PointLight* torchLights_[kTorchCount_];
	std::array<float, kTorchCount_> flickerTime_;
	std::array<float, kTorchCount_> flickerSpeed_;
	std::array<float, kTorchCount_> flickerOffset_;
	float baseIntensity_ = 1.5f;
	float flickerAmount_ = 0.3f;

	// パーティクルインスタンス（炎のエフェクト）
	ParticlePresetInstance* particleInstance_[kTorchCount_];

	// 壁からの距離（トーチを壁の内側に配置）
	float distanceFromWall_ = 1.5f;

	// JsonSettingsのグループパス
	const std::vector<std::string> kGroupPath_ = { "TitleTorch" };
};