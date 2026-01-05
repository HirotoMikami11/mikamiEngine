#pragma once
#include "ParticleEditor.h"
#include "Parts/BaseParts.h"
#include <vector>
#include <string>

// 前方宣言
class Boss;

/// <summary>
/// ボスの各パーツから砂煙パーティクルを発生させるクラス
/// 各パーツの下面座標にエミッタを配置し、移動に合わせて更新する
/// </summary>
class BossSmokeEmitter {
public:
	BossSmokeEmitter();
	~BossSmokeEmitter();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="boss">ボスへのポインタ</param>
	void Initialize(Boss* boss);

	/// <summary>
	/// 更新処理
	/// 各パーツの位置に合わせてエミッタの位置を更新
	/// </summary>
	void Update();

	/// <summary>
	/// 全てのパーティクルインスタンスを破棄
	/// </summary>
	void Finalize();

	/// <summary>
	/// ImGui表示
	/// </summary>
	void ImGui();

	void SetAllEmittersEnabled(bool enabled);
private:
	/// <summary>
	/// パーティクルインスタンスを作成
	/// </summary>
	void CreateParticleInstances();

	/// <summary>
	/// パーツの下面座標を計算
	/// </summary>
	/// <param name="part">対象のパーツ</param>
	/// <returns>パーツの下面のワールド座標</returns>
	Vector3 CalculateBottomPosition(BaseParts* part);

	/// <summary>
	/// エミッタの位置を更新
	/// </summary>
	/// <param name="instance">パーティクルインスタンス</param>
	/// <param name="position">新しい位置</param>
	void UpdateEmitterPosition(ParticlePresetInstance* instance, const Vector3& position);

	// ボスへの参照
	Boss* boss_;

	// ParticleEditorへの参照
	ParticleEditor* particleEditor_;

	// 各パーツ用のパーティクルインスタンス
	ParticlePresetInstance* headSmokeInstance_;
	std::vector<ParticlePresetInstance*> bodySmokeInstances_;
	ParticlePresetInstance* tailSmokeInstance_;

	// エミッタ名
	const std::string kEmitterName_ = "WalkSmoke_Emitter";

	// パーツ数（ボスから取得するため、初期化時に設定）
	size_t bodyPartsCount_;

	// デバッグ用：エミッタ位置を表示するか
	bool showDebugPositions_;
};