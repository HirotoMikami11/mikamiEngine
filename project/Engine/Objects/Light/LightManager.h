#pragma once
#include <memory>
#include <unordered_map>
#include "DirectXCommon.h"
#include "Structures.h"
#include "Logger.h"

//ライト関連
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "RectLight.h"

/// <summary>
/// ライト統合管理クラス（シングルトン）
/// </summary>
class LightManager
{
public:
	static constexpr int MAX_POINT_LIGHTS = 32;
	static constexpr int MAX_SPOT_LIGHTS = 16;
	static constexpr int MAX_RECT_LIGHTS = 8;

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
	/// スポットライトを追加
	/// </summary>
	/// <param name="position">位置</param>
	/// <param name="rotation">回転（Euler角、度数法）</param>
	/// <param name="color">色</param>
	/// <param name="intensity">強度</param>
	/// <param name="distance">最大距離</param>
	/// <param name="decay">減衰率</param>
	/// <param name="angle">スポット角度（外側の境界、度数法）</param>
	/// <param name="falloffStart">フォールオフ開始角度（内側の境界、度数法）※ angle > falloffStart</param>
	/// <returns>追加されたライトのポインタ（失敗時はnullptr）</returns>
	SpotLight* AddSpotLight(
		const Vector3& position,
		const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		float intensity = 1.0f,
		float distance = 15.0f,
		float decay = 2.0f,
		float angle = 30.0f,
		float falloffStart = 20.0f);

	/// <summary>
	/// エリアライト（矩形ライト）を追加
	/// </summary>
	/// <param name="position">矩形の中心位置</param>
	/// <param name="rotation">矩形の回転（Euler角、度数法）</param>
	/// <param name="color">色</param>
	/// <param name="intensity">強度</param>
	/// <param name="width">矩形の幅</param>
	/// <param name="height">矩形の高さ</param>
	/// <param name="decay">減衰率</param>
	/// <returns>追加されたライトのポインタ（失敗時はnullptr）</returns>
	RectLight* AddRectLight(
		const Vector3& position,
		const Vector3& rotation = { 0.0f, 0.0f, 0.0f },
		const Vector4& color = { 1.0f, 1.0f, 1.0f, 1.0f },
		float intensity = 1.0f,
		float width = 2.0f,
		float height = 2.0f,
		float decay = 2.0f);

	/// <summary>
	/// ポイントライトを削除
	/// </summary>
	/// <param name="light">削除するライトのポインタ</param>
	void RemovePointLight(PointLight* light);

	/// <summary>
	/// スポットライトを削除
	/// </summary>
	/// <param name="light">削除するライトのポインタ</param>
	void RemoveSpotLight(SpotLight* light);

	/// <summary>
	/// エリアライトを削除
	/// </summary>
	/// <param name="light">削除するライトのポインタ</param>
	void RemoveRectLight(RectLight* light);

	/// <summary>
	/// 全ポイントライトをクリア
	/// </summary>
	void ClearPointLights();

	/// <summary>
	/// 全スポットライトをクリア
	/// </summary>
	void ClearSpotLights();

	/// <summary>
	/// 全エリアライトをクリア
	/// </summary>
	void ClearRectLights();

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
	/// スポットライト数を取得
	/// </summary>
	int GetSpotLightCount() const { return static_cast<int>(spotLights_.size()); }

	/// <summary>
	/// エリアライト数を取得
	/// </summary>
	int GetRectLightCount() const { return static_cast<int>(rectLights_.size()); }

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
	std::unordered_map<uint32_t, std::unique_ptr<SpotLight>> spotLights_;
	std::unordered_map<uint32_t, std::unique_ptr<RectLight>> rectLights_;
	uint32_t nextLightID_ = 1;  // ID生成用（単調増加）

	// GPU送信用リソース
	Microsoft::WRL::ComPtr<ID3D12Resource> lightingResource_;
	LightingData* lightingData_ = nullptr;
};