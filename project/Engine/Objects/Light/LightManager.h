#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <memory>
#include <unordered_map>
#include "DirectXCommon.h"
#include "Structures.h"
#include "DirectionalLight.h"
#include "PointLight.h"
#include "Logger.h"

/// <summary>
/// ライト統合管理クラス（シングルトン）
/// </summary>
class LightManager
{
public:
	static constexpr int MAX_POINT_LIGHTS = 32;

	// シングルトン
	static LightManager* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// デフォルト状態にリセット（平行光源のみ）
	/// シーン初期化時に自動で呼ばれる
	/// </summary>
	void ResetToDefault();

	/// <summary>
	/// LightingDataをGPUに送る
	/// </summary>
	void Update();

	/// <summary>
	/// ポイントライトを追加
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="color">色</param>
	/// <param name="intensity">強度</param>
	/// <param name="radius">影響範囲</param>
	/// <param name="decay">減衰率</param>
	/// <returns>追加されたライトのポインタ（失敗時はnullptr）</returns>
	PointLight* AddPointLight(
		const Vector3& position,
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		float intensity = 1.0f,
		float radius = 10.0f,
		float decay = 2.0f);

	/// <summary>
	/// ポイントライトを削除
	/// </summary>
	/// <param name="light">削除するライトのポインタ</param>
	void RemovePointLight(PointLight* light);

	/// <summary>
	/// 全ポイントライトをクリア
	/// </summary>
	void ClearPointLights();

	/// <summary>
	/// 平行光源を取得（編集可能）
	/// </summary>
	DirectionalLight& GetDirectionalLight() { return directionalLight_; }
	const DirectionalLight& GetDirectionalLight() const { return directionalLight_; }

	/// <summary>
	/// ポイントライト数を取得
	/// </summary>
	int GetPointLightCount() const { return static_cast<int>(pointLights_.size()); }

	/// <summary>
	/// LightingDataリソースを取得（GPU送信用）
	/// </summary>
	ID3D12Resource* GetLightingResource() const { return lightingResource_.Get(); }

	/// <summary>
	/// ImGui用の編集UI（使用中のライトのみ表示）
	/// </summary>
	void ImGui();


	void Finalize();

private:
	// コンストラクタ（シングルトン）
	LightManager() = default;
	~LightManager() = default;
	LightManager(const LightManager&) = delete;
	LightManager& operator=(const LightManager&) = delete;

	/// <summary>
	/// LightingDataを更新
	/// </summary>
	void UpdateLightingData();

private:
	DirectXCommon* dxCommon_ = nullptr;

	// ライトデータ
	DirectionalLight directionalLight_;
	std::unordered_map<uint32_t, std::unique_ptr<PointLight>> pointLights_;
	uint32_t nextLightID_ = 1;  // ID生成用（単調増加）

	// GPU送信用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingResource_;
	LightingData* lightingData_ = nullptr;
};