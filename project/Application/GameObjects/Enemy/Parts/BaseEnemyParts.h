#pragma once
#include "Object3D.h"
#include "Collider.h"

using namespace MyMath;

// 前方宣言
class EnemyWorm;

/// <summary>
/// EnemyWormのパーツ基底クラス（衝突判定機能付き、HP管理なし）
/// </summary>
class BaseEnemyParts : public Collider {
public:
	BaseEnemyParts() = default;
	virtual ~BaseEnemyParts() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="modelName">使用するモデル</param>
	/// <param name="textureName">使用するテクスチャ名</param>
	virtual void Initialize(DirectXCommon* dxCommon, const Vector3& position,
		const std::string& modelName = "Boss_Body",
		const std::string& textureName = "white2x2");

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw();

	/// <summary>
	/// ImGui表示
	/// </summary>
	/// <param name="label">表示ラベル</param>
	virtual void ImGui(const char* label);

	// 位置の取得・設定
	Vector3 GetPosition() const;
	void SetPosition(const Vector3& position);

	// 回転の取得・設定
	Vector3 GetRotation() const;
	void SetRotation(const Vector3& rotation);

	// Y軸回転のみ設定（向き制御用）
	void SetRotationY(float rotationY);

	// スケールの取得・設定
	Vector3 GetScale() const;
	void SetScale(const Vector3& scale);

	// 色の設定（派生クラスでオーバーライド）
	virtual void SetColor(uint32_t color);

	// Colliderインターフェースの実装
	void OnCollision(Collider* other) override;
	Vector3 GetWorldPosition() override;

	// アクティブ状態管理
	bool IsActive() const { return isActive_; }
	void SetActive(bool active) { isActive_ = active; }

	// デフォルトカラーの保存
	void SetDefaultColor(const uint32_t& color) { defaultColor_ = color; }
	uint32_t GetDefaultColor() const { return defaultColor_; }

	/// <summary>
	/// ダメージを受けたときの色を設定（赤色）
	/// </summary>
	void SetDamageColor();

	/// <summary>
	/// ダメージ色タイマーの更新
	/// </summary>
	void UpdateDamageColorTimer();

	/// <summary>
	/// 描画の可視性を取得
	/// </summary>
	bool IsVisible() const { return isVisible_; }

	/// <summary>
	/// 描画の可視性を設定
	/// </summary>
	void SetVisible(bool visible) { isVisible_ = visible; }

	/// <summary>
	/// 親のEnemyWormを設定
	/// </summary>
	void SetEnemy(EnemyWorm* enemy) { enemy_ = enemy; }

protected:
	std::unique_ptr<Object3D> gameObject_;
	DirectXCommon* dxCommon_ = nullptr;

	// アクティブ状態
	bool isActive_ = true;

	// 描画可視性フラグ
	bool isVisible_ = true;

	// デフォルトカラー
	uint32_t defaultColor_ = 0xFFFFFFFF;

	// ダメージ色表示タイマー
	int damageColorTimer_ = 0;
	static const int kDamageColorDuration = 1;  // ダメージ色の表示フレーム数
	const uint32_t kDamageColor = 0xFF0000FF;   // ダメージ時の色（赤）

	// 親のEnemyWorm（ダメージを通知するため）
	EnemyWorm* enemy_ = nullptr;
};
