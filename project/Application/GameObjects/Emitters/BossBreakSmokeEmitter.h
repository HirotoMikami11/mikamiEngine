#pragma once
#include "ParticleEditor.h"
#include "Parts/BaseParts.h"
#include <vector>
#include <string>

// 前方宣言
class Boss;

/// <summary>
/// ボスの非アクティブなパーツから破壊煙パーティクルを発生させるクラス
/// 最大2個のエミッターを使用し、非アクティブなパーツの面からランダムに煙を噴出
/// </summary>
class BossBreakSmokeEmitter {
public:

	/// <summary>
	/// パーツの面の種類
	/// </summary>
	enum class PartsFace {
		Top,		// 上面
		Front,		// 前面 (+Z)
		Back,		// 後面 (-Z)
		Left,		// 左面 (-X)
		Right,		// 右面 (+X)
		Count		// 面の総数
	};

	/// <summary>
	/// エミッターデータ構造体
	/// </summary>
	struct EmitterData {
		ParticlePresetInstance* instance = nullptr;	// パーティクルインスタンス
		float timer = 0.0f;								// 再起動までの待機時間
		BaseParts* targetPart = nullptr;				// 対象パーツ
		PartsFace targetFace = PartsFace::Top;			// 対象の面
		bool isWaiting = false;							// 待機中かどうか
	};



	BossBreakSmokeEmitter();
	~BossBreakSmokeEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="boss">ボスへのポインタ</param>
	void Initialize(Boss* boss);

	/// <summary>
	/// 更新処理
	/// 非アクティブなパーツを検出し、エミッターを配置・更新
	/// </summary>
	/// <param name="deltaTime">デルタタイム</param>
	void Update();

	/// <summary>
	/// 全てのパーティクルインスタンスを破棄
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

private:
	/// <summary>
	/// パーティクルインスタンスを作成
	/// </summary>
	void CreateParticleInstances();

	/// <summary>
	/// 非アクティブなBodyパーツを取得
	/// </summary>
	/// <returns>非アクティブなBodyパーツのリスト</returns>
	std::vector<BaseParts*> GetInactiveBodyParts();

	/// <summary>
	/// エミッターを更新
	/// </summary>
	/// <param name="emitterData">エミッターデータ</param>
	/// <param name="inactiveParts">非アクティブなパーツリスト</param>
	/// <param name="deltaTime">デルタタイム</param>
	void UpdateEmitter(EmitterData& emitterData, const std::vector<BaseParts*>& inactiveParts);

	/// <summary>
	/// エミッターを起動（位置・方向を設定して放出開始）
	/// </summary>
	/// <param name="emitterData">エミッターデータ</param>
	/// <param name="inactiveParts">非アクティブなパーツリスト</param>
	void ActivateEmitter(EmitterData& emitterData, const std::vector<BaseParts*>& inactiveParts);

	/// <summary>
	/// パーツの指定面の中心位置を計算
	/// </summary>
	/// <param name="part">対象パーツ</param>
	/// <param name="face">面の種類</param>
	/// <returns>面の中心位置（ワールド座標）</returns>
	Vector3 CalculateFacePosition(BaseParts* part, PartsFace face);

	/// <summary>
	/// 指定面の法線ベクトルを取得
	/// </summary>
	/// <param name="face">面の種類</param>
	/// <returns>法線ベクトル</returns>
	Vector3 GetFaceNormal(PartsFace face);

	/// <summary>
	/// ランダムな面を選択
	/// </summary>
	/// <returns>選択された面</returns>
	PartsFace GetRandomFace();

	/// <summary>
	/// 既に使用されているパーツかチェック
	/// </summary>
	/// <param name="part">チェックするパーツ</param>
	/// <param name="excludeIndex">除外するエミッターのインデックス（-1で全チェック）</param>
	/// <returns>使用されている場合true</returns>
	bool IsPartAlreadyUsed(BaseParts* part, int excludeIndex = -1);

	// ボスへの参照
	Boss* boss_;

	// ParticleEditorへの参照
	ParticleEditor* particleEditor_;

	// エミッターデータ（2個）
	static const int kEmitterCount = 2;
	EmitterData emitters_[kEmitterCount];

	// エミッター名
	const std::string kEmitterName_ = "BreakSmoke_Emitter";

	// 再起動待機時間（秒）
	const float kRestartWaitTime_ = 5.0f;

	// デバッグ用：エミッタ情報を表示するか
	bool showDebugInfo_;
};