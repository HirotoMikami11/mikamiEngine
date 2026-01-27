#pragma once
#include "Object3D.h"
#include "DirectXCommon.h"
#include "Collider.h"
#include <array>

/// <summary>
/// 壁の個別コライダークラス
/// 各壁がColliderを継承し、独立した衝突判定を持つ
/// </summary>
class WallCollider : public Collider {
public:
	WallCollider() = default;
	~WallCollider() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="position">壁の位置</param>
	/// <param name="aabbSize">AABBのサイズ</param>
	void Initialize(const Vector3& position, const Vector3& aabbSize);

	/// <summary>
	/// 位置を更新
	/// </summary>
	void SetPosition(const Vector3& position) { position_ = position; }

	/// <summary>
	/// AABBサイズを更新
	/// </summary>
	void SetAABBSize(const Vector3& size);

	/// <summary>
	/// 現在のAABBサイズを取得
	/// </summary>
	Vector3 GetAABBSize() const { return aabbSize_; }

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	void OnCollision(Collider* other) override;

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	Vector3 GetWorldPosition() override { return position_; }

private:
	Vector3 position_ = { 0.0f, 0.0f, 0.0f };
	Vector3 aabbSize_ = { 1.0f, 1.0f, 1.0f };  // 現在のAABBサイズを保存
};

/// <summary>
/// 壁クラス
/// 指定した領域を4面の壁で囲むオブジェクト
/// </summary>
class Wall
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Wall();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Wall();

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
	/// 4面の壁を描画（手前の壁は除く）
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
	/// 現在の設定をJsonファイルに保存
	/// </summary>
	void SaveToJson();

	/// <summary>
	/// JsonSettingsから設定を読み込んで適用
	/// </summary>
	void ApplyParameters();

	/// <summary>
	/// 全ての壁のコライダーを取得（CollisionManagerに登録するため）
	/// </summary>
	std::vector<Collider*> GetColliders();

private:
	/// <summary>
	/// 壁オブジェクトの構造体
	/// 各壁のモデルとトランスフォーム情報を格納
	/// </summary>
	struct WallObject {
		std::unique_ptr<Model3D> obj;
		Vector3Transform transform;
		std::array<std::unique_ptr<WallCollider>, 3> colliders;  // 各壁に3つのコライダー
		std::array<Vector3, 3> colliderOffsets;  // 各コライダーの中心座標オフセット
		std::array<Vector3, 3> colliderSizes;    // 各コライダーのサイズ
	};

	/// <summary>
	/// 各壁のトランスフォームを計算して更新
	/// areaSize_とmodelSizeに基づいて4面の壁を適切に配置
	/// </summary>
	void UpdateTransforms();

	/// <summary>
	/// コライダーを更新
	/// </summary>
	void UpdateColliders();

	// システム参照
	DirectXCommon* dxCommon_;

	// 壁モデルサイズ（scale が 1 のときの実寸法）
	Vector3 modelSize;

	// 4方向の壁（前, 右, 後, 左）
	std::array<WallObject, 4> walls_;

	// 囲みたい領域（フルサイズ)
	Vector2 areaSize_;

	// JsonSettingsのグループパス
	const std::vector<std::string> kGroupPath_ = { "Wall" };
};
