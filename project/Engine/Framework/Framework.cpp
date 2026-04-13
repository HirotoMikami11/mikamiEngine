#include "Framework.h"
#include "LeakChecker.h"	//リークチェッカー

void Framework::Run() {

	//リークチェッカー
	D3DResourceLeakChecker leakChecker;

	//エンジンの取得と初期化
	engine_ = Engine::GetInstance();
	engine_->Initialize(L"LE2A_15_ミカミ_ヒロト");

	//初期化
	Initialize();

	//ウィンドウのxボタンが押されるまでループ
	while (engine_->ProcessMessage()) {

		///*-----------------------------------------------------------------------*///
		///									更新処理								   ///
		///*-----------------------------------------------------------------------*///

		//エンジン更新
		engine_->Update();
		//フレームワーク更新
		Update();

		// エンジンのImGui更新
		engine_->ImGui();
		// フレームワークのImGui更新
		ImGui();

		///*-----------------------------------------------------------------------*///
		///									描画処理								   ///
		///*-----------------------------------------------------------------------*///

		/// フレーム開始（コマンドリストとRendererリセット）
		engine_->BeginFrame();

		/// オフスクリーン開始
		/// Draw前に呼ぶことでパーティクル等の直接GPU命令が通る
		/// TODO:パーティクルもSubmit系にして、Draw内で一括描画にしたい
		engine_->BeginOffscreen();

		/// 全描画
		/// モデルとspriteのSubmit系を積む、パーティクルは直接コマンドを積むので現状はここで処理
		Draw();

		/// オフスクリーン描画終了
		engine_->EndOffscreen();

		/// バックバッファ描画（合成 → FlushUI → ImGui → Present）
		engine_->BeginBackBuffer();
		engine_->EndBackBuffer();
	}

	///*-----------------------------------------------------------------------*///
	///									終了処理								   ///
	///*-----------------------------------------------------------------------*///

	///終了処理
	Finalize();
	engine_->Finalize();
}
