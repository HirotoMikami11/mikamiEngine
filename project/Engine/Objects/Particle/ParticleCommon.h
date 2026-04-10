#pragma once
#include "DirectXCommon.h"
#include "PSOFactory.h"
#include "RootSignatureBuilder.h"
#include "MyFunction.h"
#include "Structures.h"

/// <summary>
/// パーティクル描画の共通設定を管理するクラス。
/// PSO を自前で生成し、setCommonRenderSettings() で適用する
/// </summary>
class ParticleCommon
{
public:
	template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

	static ParticleCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// PSO を生成
	/// </summary>
	/// <param name="dxCommon">DirectXCommon のポインタ</param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// GPU リソースを明示的に解放する。
	/// Engine::Finalize() の dxCommon_->Finalize() より前に呼ぶこと。
	/// </summary>
	void Finalize();

	/// <summary>
	/// パーティクル用の RootSignature・PSO・トポロジを CommandList にセットする。
	/// ParticleSystem::Draw() の先頭で呼ぶこと。
	/// </summary>
	void setCommonRenderSettings();

private:
	ParticleCommon() = default;
	~ParticleCommon() = default;
	ParticleCommon(const ParticleCommon&) = delete;
	ParticleCommon& operator=(const ParticleCommon&) = delete;

	/// <summary>
	/// PSOFactory を使って RootSignature と PSO を生成する
	/// </summary>
	void InitializePSO();

	// DirectXCommon（コマンドリスト取得に使用）
	DirectXCommon* dxCommon_ = nullptr;

	// 自前で生成した PSO（DirectXCommon の PSO は使用しない）
	PSOFactory::PSOInfo pso_;
};
