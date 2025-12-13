#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// 紙が燃えるポストエフェクト（OffscreenTriangle使用版）
/// 画面外から炎が中心に向かって進み、燃えた部分は黒くなる
/// </summary>
class BurnPostEffect : public PostEffect {
public:
	/// <summary>
	/// 燃焼エフェクト専用のパラメータ
	/// </summary>
	struct BurnParameters {
		Vector4 burnColor = { 1.0f, 0.4f, 0.1f, 1.0f };		// 炎の主色（オレンジ）
		Vector4 edgeColor = { 1.0f, 0.8f, 0.2f, 1.0f };		// 燃焼境界の色（黄色）

		float progress = 0.0f;								// 燃焼の進行度（0.0～1.0）
		float edgeWidth = 0.05f;							// 燃焼境界の幅
		float noiseScale = 5.0f;							// ノイズのスケール
		float noiseStrength = 0.2f;							// ノイズの強度

		Vector2 burnCenter = { 0.5f, 0.5f };				// 燃焼の中心点（UV座標）
		float time = 0.0f;									// アニメーション用の時間
		float burnSpeed = 1.0f;								// 燃焼の速度（アニメーション用）

		float padding[2] = { 0.0f, 0.0f };					// パディング（16バイトアライメント）
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,			// エフェクトなし
		SLOW_BURN,		// ゆっくり燃える
		NORMAL_BURN,	// 通常の燃焼
		FAST_BURN,		// 速く燃える
		INTENSE_BURN,	// 激しい燃焼（ノイズ強め）
		GENTLE_BURN		// 穏やかな燃焼（ノイズ弱め）
	};

public:
	BurnPostEffect() { name_ = "Burn Effect"; }
	~BurnPostEffect() = default;

	// PostEffect インターフェース実装
	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) override;
	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }

	// 固有メソッド

	/// <summary>
	/// プリセットを適用
	/// </summary>
	void ApplyPreset(EffectPreset preset);

	/// <summary>
	/// 燃焼の進行度を設定（0.0～1.0）
	/// </summary>
	void SetProgress(float progress);

	/// <summary>
	/// 炎の色を設定
	/// </summary>
	void SetBurnColor(const Vector4& color);

	/// <summary>
	/// 燃焼境界の色を設定
	/// </summary>
	void SetEdgeColor(const Vector4& color);

	/// <summary>
	/// 燃焼境界の幅を設定
	/// </summary>
	void SetEdgeWidth(float width);

	/// <summary>
	/// ノイズのスケールを設定
	/// </summary>
	void SetNoiseScale(float scale);

	/// <summary>
	/// ノイズの強度を設定
	/// </summary>
	void SetNoiseStrength(float strength);

	/// <summary>
	/// 燃焼の中心点を設定（UV座標）
	/// </summary>
	void SetBurnCenter(const Vector2& center);

	/// <summary>
	/// 燃焼速度を設定
	/// </summary>
	void SetBurnSpeed(float speed);

	/// <summary>
	/// 燃焼をリセット（progress = 0に戻す）
	/// </summary>
	void Reset();

	/// <summary>
	/// 燃焼を開始（自動的にprogressを増加）
	/// </summary>
	void StartBurn();

	/// <summary>
	/// 燃焼を停止
	/// </summary>
	void StopBurn();

	// ゲッター
	float GetProgress() const { return parameters_.progress; }
	Vector4 GetBurnColor() const { return parameters_.burnColor; }
	Vector4 GetEdgeColor() const { return parameters_.edgeColor; }
	float GetEdgeWidth() const { return parameters_.edgeWidth; }
	float GetNoiseScale() const { return parameters_.noiseScale; }
	float GetNoiseStrength() const { return parameters_.noiseStrength; }
	Vector2 GetBurnCenter() const { return parameters_.burnCenter; }
	bool IsAutoPlaying() const { return isAutoPlaying_; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;
	bool isAutoPlaying_ = false;	// 自動再生中かどうか

	// 燃焼パラメータ
	BurnParameters parameters_;

	// 燃焼エフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	BurnParameters* mappedParameters_ = nullptr;
};