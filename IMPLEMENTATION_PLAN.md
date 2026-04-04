# BaseScene リファクタリング実装手順書

## AI 行動規約

**Claude はこの作業において以下を厳守する：**

- `git push` は一切行わない（ローカル作業のみ）
- `git commit` はユーザーが明示的に指示した場合のみ行う
- リモートリポジトリへの操作（push / force-push / PR作成 など）は禁止
- 外部サービスへのデータ送信は行わない
- 破壊的操作（ファイル大量削除・ブランチ削除など）は事前確認する

---

## 目的

- BaseScene が Update / Finalize などのライフサイクルを `final` で一元管理する
- 派生シーンは `On*()` フックのみを override する（SoheEngine パターン）
- コライダー登録責務を GameObjectManager から BaseScene::HandleCollisions() へ移動
- GameObjectManager への Constructor Injection（TestShooter 等）
- 衝突判定の流れをシーン側から完全に隠蔽する

---

## 現状と変更後の対比

### BaseScene のライフサイクル

| 現状 | 変更後 |
|---|---|
| 派生クラスが Update/Finalize 等を直接 override | BaseScene が final で制御、派生は On* を override |
| コライダー同期が GameObjectManager::Update() 内 | BaseScene::HandleCollisions() が責任を持つ |
| Finalize() は派生クラスが gameObjectManager_.Clear() を手動呼び出し | BaseScene::Finalize() final が必ず Clear() + CollisionManager リセットを保証 |
| コンストラクタは空、Initialize() でオブジェクト生成 | コンストラクタでオブジェクト生成、InitializeAll() がタグ順で Initialize を呼ぶ |

### GameObjectManager の責務変化

| 現状 | 変更後 |
|---|---|
| Update() 内で SyncColliders + ExtraRegistrar + CollisionManager::Update() まで実行 | Update() はオブジェクトの更新のみ（衝突なし） |
| SetCollisionManager() で CM への参照を保持 | CM への参照なし（BaseScene が直接管理） |
| ExtraColliderRegistrar コールバック | 削除（OnHandleCollisions() が代替） |
| InitializeAll() なし | InitializeAll() 追加（タグ順ソート後に Initialize） |
| AddAllCollidersToManager() なし | AddAllCollidersToManager() 追加（ICollider の dynamic_cast） |

---

## 実装ステップ

### Step 1: GameObjectManager 変更

**GameObjectManager.h**

```
削除:
  - void SetCollisionManager(CollisionManager* cm)
  - using ExtraColliderRegistrar = ...
  - void SetExtraColliderRegistrar(...)
  - CollisionManager* collisionManager_
  - ExtraColliderRegistrar extraColliderRegistrar_
  - private: void SyncColliders()

追加:
  + void InitializeAll()
      タグ順ソート後に全オブジェクトの Initialize() を呼ぶ
  + void AddAllCollidersToManager(CollisionManager* cm)
      ICollider を継承するオブジェクトを cm に追加（旧 SyncColliders の外部公開版）

変更:
  Update() コメントを「オブジェクト更新のみ」に修正
```

**GameObjectManager.cpp**

```
Update() から以下を削除:
  - if (collisionManager_) { SyncColliders(); if (extra)...; collisionManager_->Update(); }

Clear() から以下を削除:
  - if (collisionManager_) { collisionManager_->Initialize(); }
  ※ CollisionManager のリセットは BaseScene::Finalize() final が行う

InitializeAll() 実装:
  stable_sort (GetUpdateOrder() 昇順) → for each obj: obj->Initialize()

AddAllCollidersToManager() 実装:
  for each obj: if (auto* c = dynamic_cast<ICollider*>(obj.get())) cm->AddCollider(c)

SyncColliders() 削除
```

---

### Step 2: BaseScene.h / BaseScene.cpp 新設計

**BaseScene.h（宣言のみ、ロジックは .cpp）**

```cpp
// public final メソッド（SceneManager が呼ぶ）
void Initialize()    final;   // InitializeAll() → OnInitialize()
void Update()        final;   // OnUpdate() → objectManager_.Update() → HandleCollisions()
void DrawOffscreen() final;   // objectManager_.DrawOffscreen() → OnDrawOffscreen()
void DrawBackBuffer()final;   // objectManager_.DrawBackBuffer() → OnDrawBackBuffer()
void ImGui()         final;   // objectManager_.ImGui() → OnImGui()
void Finalize()      final;   // OnFinalize() → objectManager_.Clear() → collisionManager_.Initialize()

// 既存のまま残す（final にしない）
virtual void ConfigureOffscreenEffects() {}

// protected virtual フック（派生クラスが必要に応じて override）
virtual void OnInitialize()       {}   // カメラ・ライト・Manager外オブジェクト生成
virtual void OnUpdate()           {}   // カメラ更新・Manager外オブジェクト更新
virtual void OnDrawOffscreen()    {}   // Manager外の3D描画
virtual void OnDrawBackBuffer()   {}   // Manager外のUI描画
virtual void OnImGui()            {}   // Manager外の ImGui
virtual void OnFinalize()         {}   // raw ポインタのnullptr化など
virtual void OnHandleCollisions() {}   // Manager外コライダーの追加登録

// private
void HandleCollisions();
//   collisionManager_.ClearColliderList()
//   gameObjectManager_.AddAllCollidersToManager(&collisionManager_)
//   OnHandleCollisions()
//   collisionManager_.Update()

// コンストラクタ変更
// BaseScene(const std::string& sceneName) : sceneName_(sceneName) {}
// ← SetCollisionManager/SetExtraColliderRegistrar の呼び出しを削除
```

**BaseScene.cpp（新規作成）**

```cpp
void BaseScene::Initialize() {
    gameObjectManager_.InitializeAll();
    OnInitialize();
}

void BaseScene::Update() {
    OnUpdate();
    gameObjectManager_.Update();
    HandleCollisions();
}

void BaseScene::DrawOffscreen() {
    gameObjectManager_.DrawOffscreen();
    OnDrawOffscreen();
}

void BaseScene::DrawBackBuffer() {
    gameObjectManager_.DrawBackBuffer();
    OnDrawBackBuffer();
}

void BaseScene::ImGui() {
    gameObjectManager_.ImGui();
    OnImGui();
}

void BaseScene::Finalize() {
    OnFinalize();
    gameObjectManager_.Clear();
    collisionManager_.Initialize();  // prevPairs / currentPairs のリセット保証
}

void BaseScene::HandleCollisions() {
    collisionManager_.ClearColliderList();
    gameObjectManager_.AddAllCollidersToManager(&collisionManager_);
    OnHandleCollisions();
    collisionManager_.Update();
}
```

---

### Step 3: TestShooter コンストラクタ注入

**TestShooter.h**

```
削除:
  - void SetDependencies(GameObjectManager*, TestPlayer*)

追加:
  + TestShooter(GameObjectManager* objectManager, TestPlayer* target)
    ※ コンストラクタで objectManager_ と target_ を初期化
```

**TestShooter.cpp**

```
削除:
  - SetDependencies() の実装

追加:
  + コンストラクタ実装
    TestShooter::TestShooter(GameObjectManager* objectManager, TestPlayer* target)
        : objectManager_(objectManager), target_(target) {}
```

---

### Step 4: DebugScene リファクタリング

**DebugScene.h**

```
削除:
  - void Initialize() override    ← BaseScene::Initialize() final に置き換え
  - void Update() override        ← OnUpdate() に
  - void DrawOffscreen() override ← OnDrawOffscreen() に
  - void DrawBackBuffer() override← OnDrawBackBuffer() に
  - void ImGui() override         ← OnImGui() に
  - void Finalize() override      ← OnFinalize() に
  - void UpdateGameObjects()      ← OnUpdate() に統合

追加:
  + void OnInitialize() override
  + void OnUpdate() override
  + void OnDrawOffscreen() override
  + void OnDrawBackBuffer() override
  + void OnImGui() override
  + void OnFinalize() override
```

**DebugScene.cpp**

```
コンストラクタで TestPlayer / TestWall / TestObject / TestShooter を生成・AddObject:
  auto player = std::make_unique<TestPlayer>();
  testPlayer_ = player.get();
  gameObjectManager_.AddObject(std::move(player));

  auto wall = std::make_unique<TestWall>();
  testWall_ = wall.get();
  gameObjectManager_.AddObject(std::move(wall));

  auto testObj = std::make_unique<TestObject>();
  testObject_ = testObj.get();
  gameObjectManager_.AddObject(std::move(testObj));

  // TestShooter は testPlayer_ への参照が必要なので最後に生成
  auto shooter = std::make_unique<TestShooter>(&gameObjectManager_, testPlayer_);
  testShooter_ = shooter.get();
  gameObjectManager_.AddObject(std::move(shooter));

OnInitialize():
  dxCommon_ / offscreenRenderer_ の取得
  cameraController_ 初期化
  terrain_ / gltfPlane_ / objPlane_ の生成・初期化（Manager外）
  ライト設定
  ConfigureOffscreenEffects()

OnUpdate():
  cameraController_->Update()
  vp 行列更新
  terrain_, gltfPlane_, objPlane_ の Update

OnDrawOffscreen():
  terrain_, gltfPlane_, objPlane_ の Draw
  ※ gameObjectManager_.DrawOffscreen() は BaseScene::DrawOffscreen() final が呼ぶ

OnDrawBackBuffer():
  ※ Manager内オブジェクトのみなので空でよい

OnImGui():
  terrain_, gltfPlane_, objPlane_ の ImGui

OnFinalize():
  testPlayer_ = nullptr
  testWall_   = nullptr
  testObject_ = nullptr
  testShooter_= nullptr
  ※ gameObjectManager_.Clear() は BaseScene::Finalize() final が呼ぶ

InitializeGameObjects() 削除（コンストラクタに統合）
UpdateGameObjects() 削除（OnUpdate() に統合）
```

---

### Step 5: DemoScene リファクタリング

**DemoScene.h**

```
削除: Initialize / Update / DrawOffscreen / DrawBackBuffer / ImGui / Finalize override
追加: OnInitialize / OnUpdate / OnDrawOffscreen / OnDrawBackBuffer / OnImGui / OnFinalize override
削除: InitializeGameObjects(), UpdateGameObjects()
```

**DemoScene.cpp**

```
コンストラクタ: 既存のまま（Manager外オブジェクトは OnInitialize で生成するため空でよい）

OnInitialize():
  現在の Initialize() の全内容を移動
  ※ Manager内オブジェクトは存在しないため gameObjectManager_ への AddObject はなし

OnUpdate():
  現在の Update() + UpdateGameObjects() の内容を移動

OnDrawOffscreen():
  現在の DrawOffscreen() の内容を移動

OnDrawBackBuffer():
  現在の DrawBackBuffer() の内容を移動

OnImGui():
  現在の ImGui() の内容を移動（#ifdef USEIMGUI ガードも含む）

OnFinalize():
  現在の Finalize() の内容を移動（particleSystem_->Clear() など）
```

---

### Step 6: GameScene リファクタリング

DemoScene と同様に、各メソッドを On* に改名。  
現状はほぼ空実装なので変更量は最小。

---

## 変更ファイル一覧

| ファイル | 変更種別 |
|---|---|
| `GameObjectManager.h` | 変更 |
| `GameObjectManager.cpp` | 変更 |
| `BaseScene.h` | 変更 |
| `BaseScene.cpp` | **新規作成** |
| `DebugScene.h` | 変更 |
| `DebugScene.cpp` | 変更 |
| `DemoScene.h` | 変更 |
| `DemoScene.cpp` | 変更 |
| `GameScene.h` | 変更 |
| `GameScene.cpp` | 変更 |
| `TestShooter.h` | 変更 |
| `TestShooter.cpp` | 変更 |
| `mikamiEngine.vcxproj` | 変更（BaseScene.cpp 追加） |

---

## 注意事項

### Initialize() の呼び出し順序

```
BaseScene::Initialize() final
  └─ gameObjectManager_.InitializeAll()   ← タグ順（Bullet=40 > Player=20）
       └─ 各 GameObject::Initialize()
  └─ OnInitialize()                       ← カメラ・ライト・Manager外オブジェクト生成
```

Manager内オブジェクトの Initialize() は OnInitialize() よりも **先** に呼ばれる。  
カメラの初期化は OnInitialize() で行うが、GameObject の Initialize() は  
カメラを使わない（`CameraController::GetInstance()` はシングルトンで常にアクセス可能）ため問題なし。

### Update() の呼び出し順序

```
BaseScene::Update() final
  └─ OnUpdate()              ← カメラ更新・Manager外オブジェクト更新（VP行列確定）
  └─ gameObjectManager_.Update()  ← タグ順でオブジェクト Update（VP行列を使う）
  └─ HandleCollisions()      ← 衝突判定
       └─ ClearColliderList()
       └─ AddAllCollidersToManager()
       └─ OnHandleCollisions()   ← Manager外コライダー追加（必要なシーンのみ override）
       └─ collisionManager_.Update()
```

OnUpdate() が先なのは、カメラ更新（VP行列確定）をゲームオブジェクトの Update() より前に行う必要があるため。

### Finalize() の呼び出し順序

```
BaseScene::Finalize() final
  └─ OnFinalize()              ← raw ポインタの nullptr 化（オブジェクトはまだ生存中）
  └─ gameObjectManager_.Clear()    ← 全 GameObject::Finalize() → メモリ解放
  └─ collisionManager_.Initialize() ← prevPairs / currentPairs のリセット
```

OnFinalize() はオブジェクトが**まだ生存している**タイミングで呼ばれるため、  
raw ポインタ経由の最終処理が必要なシーンにとって安全。

### DemoScene の特殊性

DemoScene は Manager 内オブジェクトが一切存在しない。  
コンストラクタは既存のまま空でよく、全オブジェクトは OnInitialize() で生成する。  
HandleCollisions() は毎フレーム空実行となるが、パフォーマンス影響はない。
