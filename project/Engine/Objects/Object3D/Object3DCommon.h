#pragma once
#include "DirectXCommon.h"
#include "MyFunction.h"
#include "Structures.h"

/// <summary>
/// modelの共通部分
/// </summary>
class Object3DCommon
{
public:
	//クラス内でのみ,namespace省略(エイリアステンプレート)
	template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;


	//シングルトン
	static Object3DCommon* GetInstance();

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
	Object3DCommon() = default;
	~Object3DCommon() = default;
	Object3DCommon(const Object3DCommon&) = delete;
	Object3DCommon& operator=(const Object3DCommon&) = delete;

	// 基本情報
	DirectXCommon* dxCommon_ = nullptr;


};

