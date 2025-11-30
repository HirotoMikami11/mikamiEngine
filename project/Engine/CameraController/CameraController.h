#pragma once
#include "BaseCamera.h"
#include "Camera.h"
#include "DebugCamera.h"
#include "CameraFunc/CameraShake.h"
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

/// <summary>
/// カメラ登録・管理するコントローラー
/// </summary>
class CameraController {
public:
	/// <summary>
	/// シングルトンインスタンス取得
	/// </summary>
	static CameraController* GetInstance();

	/// <summary>
	/// カメラシステムの初期化
	/// 標準カメラ（Normal、Debug）を自動登録
	/// </summary>
	/// <param name="dxCommon">DirectXCommonのポインタ</param>
	/// <param name="position">初期位置</param>
	/// <param name="rotation">初期回転（デフォルト：{0,0,0}）</param>
	void Initialize(DirectXCommon* dxCommon, const Vector3& position, const Vector3& rotation = { 0.0f, 0.0f, 0.0f });

	/// <summary>
	/// 終了処理（全カメラのリソース解放）
	/// </summary>
	void Finalize();

	/// <summary>
	/// 全カメラの更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ImGuiデバッグ表示
	/// </summary>
	void ImGui();

	// === カメラ登録システム ===

	/// <summary>
	/// カメラを登録
	/// </summary>
	/// <param name="cameraId">カメラ識別ID</param>
	/// <param name="camera">カメラインスタンス</param>
	void RegisterCamera(const std::string& cameraId, std::unique_ptr<BaseCamera> camera);

	/// <summary>
	/// カメラの登録解除
	/// </summary>
	/// <param name="cameraId">カメラ識別ID</param>
	void UnregisterCamera(const std::string& cameraId);

	/// <summary>
	/// アクティブカメラを手動設定
	/// </summary>
	/// <param name="cameraId">アクティブにするカメラID</param>
	void SetActiveCamera(const std::string& cameraId);

	/// <summary>
	/// アクティブカメラのIDを取得
	/// </summary>
	/// <returns>現在アクティブなカメラのID</returns>
	std::string GetActiveCameraId() const { return activeCameraId_; }

	/// <summary>
	/// 登録されているカメラの一覧を取得
	/// </summary>
	/// <returns>カメラID一覧</returns>
	std::vector<std::string> GetRegisteredCameraIds() const;

	// === カメラシェイク関連 ===

	/// <summary>
	/// カメラシェイクを開始
	/// </summary>
	/// <param name="duration">揺れ時間（秒）</param>
	/// <param name="amplitude">揺れ強度</param>
	void StartCameraShake(float duration, float amplitude);

	/// <summary>
	/// 複数回シェイクを開始（爆発演出用）
	/// </summary>
	/// <param name="duration">総時間（秒）</param>
	/// <param name="amplitude">最大強度</param>
	/// <param name="frequency">揺れ頻度（Hz）</param>
	void StartMultiCameraShake(float duration, float amplitude, float frequency = 8.0f);

	/// <summary>
	/// カメラシェイクを停止
	/// </summary>
	void StopCameraShake();

	/// <summary>
	/// シェイク中かどうか判定
	/// </summary>
	bool IsCameraShaking() const;

	// === デバッグ機能 ===

	/// <summary>
	/// デバッグカメラが使用中かどうか確認
	/// </summary>
	/// <returns>デバッグカメラ使用中ならtrue</returns>
	bool IsUsingDebugCamera() const { return activeCameraId_ == "debug"; }

	/// <summary>
	/// デバッグカメラのON/OFF切り替え
	/// </summary>
	void ToggleDebugCamera();

	/// <summary>
	/// 指定されたカメラが登録されているかチェック
	/// </summary>
	bool IsRegistered(const std::string& cameraId) const;

	// === カメラ情報取得 ===

	/// <summary>
	/// カメラ行列を取得（シェイク適用）
	/// </summary>
	Matrix4x4 GetCameraMatrix() const;

	/// <summary>
	/// ビュープロジェクション行列を取得（シェイク適用）
	/// </summary>
	Matrix4x4 GetViewProjectionMatrix() const;

	/// <summary>
	/// スプライト用ビュープロジェクション行列を取得（シェイクなし）
	/// </summary>
	Matrix4x4 GetViewProjectionMatrixSprite() const;

	/// <summary>
	/// アクティブカメラの位置を取得（シェイク適用）
	/// </summary>
	/// <returns>カメラの位置</returns>
	Vector3 GetPosition() const;

	/// <summary>
	/// アクティブカメラの位置を設定
	/// </summary>
	/// <param name="position">設定する位置</param>
	void SetPosition(const Vector3& position);

	/// <summary>
	/// アクティブカメラの前方向ベクトルを取得
	/// </summary>
	/// <returns>カメラの前方向ベクトル（正規化済み）</returns>
	Vector3 GetForward() const;

	/// <summary>
	/// アクティブカメラのCameraForGPUリソースを取得
	/// </summary>
	/// <returns>CameraForGPUリソース</returns>
	ID3D12Resource* GetCameraForGPUResource() const;

	/// <summary>
	/// 指定IDのカメラを取得
	/// </summary>
	BaseCamera* GetCamera(const std::string& cameraId) const;

private:
	CameraController() = default;
	~CameraController() = default;
	CameraController(const CameraController&) = delete;
	CameraController& operator=(const CameraController&) = delete;

	/// <summary>
	/// カメラ情報構造体
	/// エンジン搭載のカメラは消せない
	/// </summary>
	struct CameraInfo {
		std::unique_ptr<BaseCamera> camera; // カメラインスタンス
		bool isEngineCamera; // エンジン標準カメラかどうか
	};

	// 登録されたカメラの管理マップ
	std::unordered_map<std::string, CameraInfo> registeredCameras_;

	// 現在アクティブなカメラのID
	std::string activeCameraId_ = "normal";
	// デバッグカメラ切り替え前のカメラID（戻り先）
	std::string lastActiveCameraId_ = "normal";

	// カメラシェイク機能
	CameraShake cameraShake_;

	// 内部ヘルパー関数

	/// <summary>
	/// 現在アクティブなカメラを取得
	/// </summary>
	BaseCamera* GetActiveCamera() const;

	/// <summary>
	/// エンジン標準カメラの登録
	/// </summary>
	void RegisterBuiltInCameras(DirectXCommon* dxCommon, const Vector3& initialPosition, const Vector3& initialRotation);

	/// <summary>
	/// デバッグ入力処理（Shift+TABでの切り替え）
	/// </summary>
	void HandleDebugInput();

	/// <summary>
	/// シェイクオフセットを適用したビュー行列を取得
	/// </summary>
	Matrix4x4 GetViewMatrixWithShake() const;
};

