#pragma once
#include <string>
#include "Engine.h"

class Framework {
public:
	Framework() = default;
	virtual ~Framework() = default;

	/// <summary>
	/// 全体の実行関数
	/// </summary>
	void Run();

protected:
	/// <summary>
	/// 初期化
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画Submit（Renderer へのSubmitのみ。GPU命令はEngine側で発行）
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// ImGui表示
	/// </summary>
	virtual void ImGui() = 0;

	/// <summary>
	/// 終了処理
	/// </summary>
	virtual void Finalize() = 0;

	//エンジンクラス
	Engine* engine_ = nullptr;
};
