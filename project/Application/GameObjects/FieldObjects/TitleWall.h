#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"
#include <array>

/// <summary>
/// タイトルシーン用の壁クラス
/// 指定した領域を4面の壁で囲むオブジェクト
/// </summary>
class TitleWall
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	TitleWall();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~TitleWall();

	/// <summary>
	/// 初期化処理
	/// 4面の壁モデルを生成し、JsonSettingsから設定を読み込む
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// 各壁の行列を更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// 4面の壁を描画（手前と奥の壁は除く）
	/// </summary>
	void Draw();

	/// <summary>
	/// ImGui表示
	/// 壁のパラメータ調整とJsonSettings保存機能
	/// </summary>
	void ImGui();

	/// <summary>
	/// 囲みたい領域のサイズを設定
	/// 設定後、壁の位置を自動的に再計算
	/// </summary>
	/// <param name="areaSize">領域サイズ（X:幅, Y:奥行き）</param>
	void SetAreaSize(const Vector2& areaSize);

	/// <summary>
	/// Z座標オフセットを設定（セグメント移動時に使用）
	/// </summary>
	/// <param name="zOffset">Z座標のオフセット値</param>
	void SetZOffset(float zOffset);

	/// <summary>
	/// グローバル変数の変更内容を反映する（Fieldパターン）
	/// </summary>
	void ApplyGlobalVariables();

	/// <summary>
	/// グローバル変数のグループ名を取得
	/// </summary>
	std::vector<std::string> GetGlobalVariableGroupName() const { return { "TitleWall" }; }

	/// <summary>
	/// エリアサイズを取得（TitleTorchの配置計算用）
	/// </summary>
	Vector2 GetAreaSize() const { return areaSize_; }

	/// <summary>
	/// モデルサイズを取得（TitleTorchの配置計算用）
	/// </summary>
	Vector3 GetModelSize() const { return modelSize_; }

private:
	/// <summary>
	/// 壁オブジェクトの構造体
	/// 各壁のモデルとトランスフォーム情報を格納
	/// </summary>
	struct WallObject {
		std::unique_ptr<Model3D> obj;
		Vector3Transform transform;
	};

	/// <summary>
	/// 各壁のトランスフォームを計算して更新
	/// areaSize_とmodelSize_に基づいて4面の壁を適切に配置
	/// </summary>
	void UpdateTransforms();

	// システム参照
	DirectXCommon* dxCommon_;

	// 壁モデルサイズ（scale が 1 のときの実寸法）
	Vector3 modelSize_;

	// 4方向の壁（前, 右, 後, 左）
	std::array<WallObject, 4> walls_;

	// 囲みたい領域（フルサイズ)
	Vector2 areaSize_;

	// Z座標オフセット（セグメント移動用）
	float zOffset_;
};