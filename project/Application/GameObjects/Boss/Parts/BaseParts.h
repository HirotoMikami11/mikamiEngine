#pragma once
#include "Object3D.h"
#include "Light.h"
#include "MyMath.h"

using namespace MyMath;

/// <summary>
/// Bossのパーツ基底クラス
/// </summary>
class BaseParts {
public:
	BaseParts() = default;
	virtual ~BaseParts() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	virtual void Initialize(DirectXCommon* dxCommon, const Vector3& position);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="viewProjectionMatrix">ビュープロジェクション行列</param>
	virtual void Update(const Matrix4x4& viewProjectionMatrix);

	/// <summary>
	/// 描画処理
	/// </summary>
	/// <param name="directionalLight">平行光源</param>
	virtual void Draw(const Light& directionalLight);

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

	// スケールの設定
	void SetScale(const Vector3& scale);

	// 色の設定（派生クラスでオーバーライド）
	virtual void SetColor(const Vector4& color);
	virtual void SetColor(uint32_t color);

protected:
	std::unique_ptr<Object3D> gameObject_;
	DirectXCommon* directXCommon_ = nullptr;
};