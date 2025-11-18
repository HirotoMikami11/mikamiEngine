#pragma once
#include "BossSplineTrack.h"
#include "BossSplineMovement.h"
#include "BossSplineDebugger.h"
#include "CSVUtility.h"
#include <vector>
#include <string>

/// <summary>
/// Bossスプライン制御点の編集エディタ
/// ImGuiを使用した直感的な操作UI
/// </summary>
class BossMoveEditor {
public:
	BossMoveEditor() = default;
	~BossMoveEditor() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="track">BossSplineTrackへのポインタ</param>
	/// <param name="movement">BossSplineMovementへのポインタ</param>
	/// <param name="debugger">BossSplineDebuggerへのポインタ</param>
	void Initialize(BossSplineTrack* track, BossSplineMovement* movement, BossSplineDebugger* debugger);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGui表示（統合エディタUI）
	/// </summary>
	void ImGui();

	/// <summary>
	/// CSVファイルから制御点を読み込み
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	/// <returns>成功時true</returns>
	bool LoadFromCSV(const std::string& filename);

	/// <summary>
	/// CSVファイルに制御点を保存
	/// </summary>
	/// <param name="filename">ファイルパス</param>
	/// <returns>成功時true</returns>
	bool SaveToCSV(const std::string& filename);

	/// <summary>
	/// 現在の制御点リストを取得
	/// </summary>
	const std::vector<Vector3>& GetControlPoints() const { return controlPoints_; }

	/// <summary>
	/// 変更があったかチェック
	/// </summary>
	bool IsDirty() const { return isDirty_; }

private:
	void ShowMainControls();
	void ShowControlPointsList();
	void ShowFileOperations();
	void ShowMovementControls();
	void ShowDebugControls();

	/// <summary>
	/// 制御点を追加
	/// </summary>
	void AddControlPoint(const Vector3& position);

	/// <summary>
	/// 制御点を削除
	/// </summary>
	/// <param name="index">削除する制御点のインデックス</param>
	void RemoveControlPoint(int index);

	/// <summary>
	/// 制御点を更新
	/// </summary>
	/// <param name="index">更新する制御点のインデックス</param>
	/// <param name="newPosition">新しい座標</param>
	void UpdateControlPoint(int index, const Vector3& newPosition);

	/// <summary>
	/// 制御点を選択
	/// </summary>
	/// <param name="index">選択する制御点のインデックス</param>
	void SelectControlPoint(int index);

	/// <summary>
	/// TrackとMovementに制御点を適用
	/// </summary>
	void ApplyControlPoints();

	/// <summary>
	/// デフォルトの制御点を作成
	/// </summary>
	void CreateDefaultPoints();

	/// <summary>
	/// 変更フラグを立てる
	/// </summary>
	void MarkDirty();

	/// <summary>
	/// インデックスが有効かチェック
	/// </summary>
	bool IsValidIndex(int index) const;

	// システム参照
	BossSplineTrack* track_ = nullptr;
	BossSplineMovement* movement_ = nullptr;
	BossSplineDebugger* debugger_ = nullptr;

	// 制御点リスト
	std::vector<Vector3> controlPoints_;

	// 選択状態
	int selectedPointIndex_ = -1;

	// ファイル操作
	std::string csvFilePath_ = "resources/CSV/BossMove/Move_1.csv";
	char csvFilePathBuffer_[256] = "resources/CSV/BossMove/Move_1.csv";

	// 新規制御点の座標
	Vector3 newPointPosition_ = { 0.0f, 1.5f, 0.0f };

	// 変更フラグ
	bool isDirty_ = false;

	// 移動テスト用
	float previewProgress_ = 0.0f;
	bool isPreviewPlaying_ = false;
};