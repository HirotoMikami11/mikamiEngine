#pragma once
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Structures.h"

/// <summary>
/// スプライトの共通部分
/// </summary>
class ParticleCommon
{
public:
	//クラス内でのみ,namespace省略(エイリアステンプレート)
	template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	//シングルトン
	static ParticleCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通部分設定
	/// </summary>
	void setCommonRenderSettings(ID3D12GraphicsCommandList* commandList);

private:

	// コンストラクタ
	ParticleCommon() = default;
	~ParticleCommon() = default;
	ParticleCommon(const ParticleCommon&) = delete;
	ParticleCommon& operator=(const ParticleCommon&) = delete;

	// 基本情報
	DirectXCommon* directXCommon_ = nullptr;


};

