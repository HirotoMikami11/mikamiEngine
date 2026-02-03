#include "Transform3D.h"
#include "GameTimer.h"

void Transform3D::Initialize(DirectXCommon* dxCommon)
{
	// トランスフォーム用のリソースを作成
	transformResource_ = CreateBufferResource(dxCommon->GetDevice(), sizeof(TransformationMatrix));

	// トランスフォームデータにマップ
	transformResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformData_));

	// デフォルト設定で初期化
	SetDefaultTransform();
}

void Transform3D::UpdateMatrix(const Matrix4x4& viewProjectionMatrix)
{
	//																			//
	//					ワールド行列の計算（階層構造対応）						//
	//																			//

	// 1. ローカル変換行列を計算（SRT）Transformから行列を生成
	Matrix4x4 localMatrix = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);

	// 2. モデルオフセット行列を適用
	//glTFのrootNode.localMatrixなど、モデル空間での初期姿勢を適用(OBJの場合は単位行列なので影響なし)
	//計算順序: ModelOffset * LocalTransform
	//モデルの初期姿勢が保持されたままTransformが適用される
	Matrix4x4 modelSpaceMatrix = Matrix4x4Multiply(modelOffset_, localMatrix);

	// 3. 親のワールド行列を適用（階層構造）
	// 親オブジェクトがある場合、親のワールド行列を掛け算(親子関係反映)
	if (parent_) {
		transformData_->World = Matrix4x4Multiply(modelSpaceMatrix, parent_->GetWorldMatrix());
	} else {
		transformData_->World = modelSpaceMatrix;
	}

	//																			//
	//					WVP行列と法線変換行列の計算									//
	//																			//

	// 4. ビュープロジェクション行列を掛け算してWVP行列を計算
	transformData_->WVP = Matrix4x4Multiply(transformData_->World, viewProjectionMatrix);

	// 5. 法線変換用の逆転置行列を計算(非均等スケールがかかっている場合でも法線が正しく変換される)
	transformData_->WorldInverseTranspose = Matrix4x4Transpose(Matrix4x4Inverse(transformData_->World));
}

void Transform3D::SetDefaultTransform() {

	// デフォルト値に設定
	transform_.scale = { 1.0f, 1.0f, 1.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.translate = { 0.0f, 0.0f, 0.0f };

	// モデルオフセットを単位行列に初期化（変換なし）
	modelOffset_ = MakeIdentity4x4();

	// GPU側のデータも単位行列で初期化
	transformData_->World = MakeIdentity4x4();
	transformData_->WVP = MakeIdentity4x4();
	transformData_->WorldInverseTranspose = MakeIdentity4x4();
}

void Transform3D::AddPosition(const Vector3& Position)
{
	//GameTimerからゲーム内デルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float gameDeltaTime = gameTimer.GetDeltaTime();

	transform_.translate.x += Position.x * gameDeltaTime;
	transform_.translate.y += Position.y * gameDeltaTime;
	transform_.translate.z += Position.z * gameDeltaTime;
}

void Transform3D::AddRotation(const Vector3& rotation)
{
	//GameTimerからゲーム内デルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float gameDeltaTime = gameTimer.GetDeltaTime();
	transform_.rotate.x += rotation.x * gameDeltaTime;
	transform_.rotate.y += rotation.y * gameDeltaTime;
	transform_.rotate.z += rotation.z * gameDeltaTime;

}

void Transform3D::AddScale(const Vector3& Scale)
{
	//GameTimerからゲーム内デルタタイムを取得
	GameTimer& gameTimer = GameTimer::GetInstance();
	float gameDeltaTime = gameTimer.GetDeltaTime();
	transform_.scale.x += Scale.x * gameDeltaTime;
	transform_.scale.y += Scale.y * gameDeltaTime;
	transform_.scale.z += Scale.z * gameDeltaTime;
}