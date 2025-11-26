#pragma once
#include "Collider.h"			//衝突判定
#include "CollisionConfig.h"	//衝突属性のフラグを定義する
#include "PlayerBullet.h"		// 自機弾
#include "PlayerUI.h"			// UI
#include "OffscreenRenderer/EffectFunc/DamageVignette.h"	
#include <list>
#include <memory>

class Player : public Collider
{
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Player();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position);

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	/// <param name="viewProjectionMatrix">スプライト行列</param>
	void Update(const Matrix4x4& viewProjectionMatrix, const Matrix4x4& viewProjectionMatirxSprite);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// オフスクリーン外に描画
	/// </summary>
	void DrawUI();

	/// <summary>
	/// ImGui
	/// </summary>
	void ImGui();

	/// <summary>
	/// 衝突時に呼ばれる関数（オーバーライド）
	/// </summary>
	/// <param name="other">衝突相手のコライダー</param>
	void OnCollision(Collider* other) override;

	/// <summary>
	/// ワールド座標を取得（オーバーライド）
	/// </summary>
	Vector3 GetWorldPosition() override;

	/// <summary>
	/// 弾リストを取得
	/// </summary>
	const std::list<std::unique_ptr<PlayerBullet>>& GetBullets() const { return bullets_; }

	// HP管理
	float GetHP() const { return HP_; }
	float GetMaxHP() const { return maxHP_; }
	void SetHP(float hp) { HP_ = hp; }
	void TakeDamage(float damage);
	bool GetIsAlive() { return isAlive_; }
private:
	/// <summary>
	/// 移動処理（XZ平面）
	/// </summary>
	void ProcessMovement();

	/// <summary>
	/// 回転処理（向き制御）
	/// </summary>
	void ProcessRotation();

	/// <summary>
	/// 弾の発射処理
	/// </summary>
	void ProcessFire();

	/// <summary>
	/// 寿命の尽きた弾を削除
	/// </summary>
	void DeleteBullets();

	// ゲームオブジェクト
	std::unique_ptr<Object3D> gameObject_;

	// 弾リスト
	std::list<std::unique_ptr<PlayerBullet>> bullets_;
	//UI表示
	std::unique_ptr<PlayerUI> playerUI_;

	//ダメージ演出
	std::unique_ptr<DamageVignette> damageVignette_;

	// システム参照
	DirectXCommon* dxCommon_;
	Input* input_;

	// 移動関連
	Vector3 velocity_;					// 速度
	float acceleration_ = 1.5f;
	float limitRunSpeed_ = 7.0f;
	float attenuation_ = 0.5f;
	Vector2 limitArea_ = { 60.0f,60.0f };


	// 回転関連
	float rotationSpeed_ = 0.05f;

	// 弾発射関連
	float bulletSpeed_ = 0.6f;
	int fireInterval_ = 6;
	int fireTimer_ = 0;					// 発射タイマー

	// HP管理
	float maxHP_ = 100.0f;	// 最大HP
	float HP_;				// 現在HP

	bool isAlive_ = true;
};