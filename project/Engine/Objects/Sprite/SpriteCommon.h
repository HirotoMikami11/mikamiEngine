#pragma once
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Structures.h"

/// <summary>
/// スプライトの共通部分
/// </summary>
class SpriteCommon
{
public:
	//クラス内でのみ,namespace省略(エイリアステンプレート)
	template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	//シングルトン
	static SpriteCommon* GetInstance();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="dxCommon"></param>
	void Initialize(DirectXCommon* dxCommon);

	/// <summary>
	/// 共通部分設定
	/// </summary>
	void setCommonSpriteRenderSettings(ID3D12GraphicsCommandList* commandList);

private:

	// コンストラクタ
	SpriteCommon() = default;
	~SpriteCommon() = default;
	SpriteCommon(const SpriteCommon&) = delete;
	SpriteCommon& operator=(const SpriteCommon&) = delete;

	// 基本情報
	DirectXCommon* dxCommon_ = nullptr;


};

