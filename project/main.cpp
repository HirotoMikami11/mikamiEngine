#include <Windows.h>
#include "Game.h"
// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// 全体の実行
	std::unique_ptr<Game> game = std::make_unique<Game>();
	game->Run();

	return 0;
}
