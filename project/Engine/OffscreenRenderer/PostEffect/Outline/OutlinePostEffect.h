#pragma once
#include "PostEffect/PostEffect.h"
#include "MyFunction.h"
#include "Logger.h"

/// <summary>
/// アウトライン（輪郭線）ポストエフェクト（OffscreenTriangle使用版）
/// 深度バッファを使用したエッジ検出
/// </summary>
class OutlinePostEffect : public PostEffect {
public:
	/// <summary>
	/// アウトライン専用のパラメータ
	/// </summary>
	struct OutlineParameters {
		Vector4 outlineColor = { 0.0f, 0.0f, 0.0f, 1.0f };	// アウトラインの色（デフォルトは黒）

		float depthThreshold = 0.5f;						// 深度差の閾値（この値より大きい差でエッジ判定）
		float normalThreshold = 0.4f;						// 法線差の閾値（将来の拡張用）
		float outlineThickness = 0.1f;						// アウトラインの太さ（ピクセル単位）
		float outlineStrength = 1.0f;						// アウトラインの強度（0.0～1.0）

		Vector2 screenSize = { 1280.0f, 720.0f };			// スクリーンサイズ（テクセルサイズ計算用）
		float padding[2] = { 0.0f, 0.0f };					// パディング（16バイトアライメント）
	};

	/// <summary>
	/// エフェクトプリセット
	/// </summary>
	enum class EffectPreset {
		OFF,			// エフェクトなし
		THIN_BLACK,		// 細い黒線
		THICK_BLACK,	// 太い黒線
		ANIME,			// アニメ風（太めの黒線）
		COLORED,		// カラーアウトライン（白）
		SUBTLE			// 控えめなアウトライン
	};

public:
	OutlinePostEffect() { name_ = "Outline Effect"; }
	~OutlinePostEffect() = default;

	// PostEffect インターフェース実装
	void Initialize(DirectXCommon* dxCommon) override;
	void Finalize() override;
	void Update(float deltaTime) override;
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle) override;

	/// <summary>
	/// 深度テクスチャも受け取る専用Apply
	/// </summary>
	/// <param name="inputSRV">入力カラーテクスチャのSRV</param>
	/// <param name="depthSRV">深度テクスチャのSRV</param>
	/// <param name="outputRTV">出力先のRTV</param>
	/// <param name="renderTriangle">描画用三角形</param>
	void Apply(D3D12_GPU_DESCRIPTOR_HANDLE inputSRV, D3D12_GPU_DESCRIPTOR_HANDLE depthSRV, D3D12_CPU_DESCRIPTOR_HANDLE outputRTV, OffscreenTriangle* renderTriangle);

	bool IsEnabled() const override { return isEnabled_; }
	void SetEnabled(bool enabled) override { isEnabled_ = enabled; }
	void ImGui() override;
	const std::string& GetName() const override { return name_; }

	// 深度が必要なのでtrueでオーバーライド
	bool RequiresDepthTexture() const override { return true; }

	// 固有メソッド

	/// <summary>
	/// プリセットを適用
	/// </summary>
	void ApplyPreset(EffectPreset preset);

	/// <summary>
	/// アウトライン色を設定
	/// </summary>
	void SetOutlineColor(const Vector4& color);

	/// <summary>
	/// 深度閾値を設定（エッジ検出の感度）
	/// </summary>
	void SetDepthThreshold(float threshold);

	/// <summary>
	/// アウトラインの太さを設定
	/// </summary>
	void SetOutlineThickness(float thickness);

	/// <summary>
	/// アウトラインの強度を設定
	/// </summary>
	void SetOutlineStrength(float strength);

	/// <summary>
	/// スクリーンサイズを設定
	/// </summary>
	void SetScreenSize(float width, float height);

	// ゲッター
	Vector4 GetOutlineColor() const { return parameters_.outlineColor; }
	float GetDepthThreshold() const { return parameters_.depthThreshold; }
	float GetOutlineThickness() const { return parameters_.outlineThickness; }
	float GetOutlineStrength() const { return parameters_.outlineStrength; }

private:
	void CreatePSO();
	void CreateParameterBuffer();
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath, const wchar_t* profile);
	void UpdateParameterBuffer();

private:
	// エフェクトの状態
	bool isInitialized_ = false;

	// アウトラインパラメータ
	OutlineParameters parameters_;

	// アウトラインエフェクト用PSO
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob_;
	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob_;

	// バッファ
	Microsoft::WRL::ComPtr<ID3D12Resource> parameterBuffer_;
	OutlineParameters* mappedParameters_ = nullptr;
};