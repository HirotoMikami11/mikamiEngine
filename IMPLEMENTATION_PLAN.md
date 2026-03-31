# GameObject システム 実装計画

## AI 行動規約

**Claude はこの作業において以下を厳守する：**

- `git push` は一切行わない（ローカル作業のみ）
- `git commit` はユーザーが明示的に指示した場合のみ行う
- リモートリポジトリへの操作（push / force-push / PR作成 など）は禁止
- 外部サービスへのデータ送信は行わない
- 破壊的操作（ファイル大量削除・ブランチ削除など）は事前確認する

---

## 方針

- Phase 単位で実装 → ビルド → 確認を繰り返す
- 各 Phase 完了時にチェックリストを埋めて記録する
- Scene 内で明示的に初期化してから GameObjectManager に追加する
- `JsonSettings`（既存）を保存層、`JsonBinder` をバインド層として役割分担する

---

## クラス構成（全体）

```
Engine/Utility/
└── JsonBinder.h / .cpp     ← JsonSettings ラッパー（バインド + ImGui 自動化）

Engine/Core/JsonSettings/
└── JsonSettings.cpp        ← LoadFile のクラッシュ修正（assert → Logger）

Application/GameObject/
├── GameObjectTypes.h       ← ObjectTag + kObjectUpdateOrder テーブル
├── GameObject.h / .cpp     ← 基底クラス
├── GameObjectManager.h / .cpp
└── TestPlayer/
    ├── TestPlayer.h
    └── TestPlayer.cpp      ← JsonBinder を使うように書き換え
```

---

## JsonBinder 設計

### 役割分担

| クラス | 役割 |
|--------|------|
| `JsonSettings` | JSON ファイルへの保存・読み込み（保存層） |
| `JsonBinder` | 変数バインド + ImGui 自動化（バインド層） |

### 基本 Bind（個別パラメータ）

```cpp
JsonBinder binder("TestPlayer");
// → CreateGroup + LoadFile を自動実行（ファイルなしなら何もしない）

binder.Bind("MoveSpeed", &moveSpeed_, 5.0f);
// 1. AddItem でデフォルト値登録（JSON に保存済みならスキップ）
// 2. 現在値を moveSpeed_ に適用
// 3. DragFloat の ImGui ラムダを登録

binder.ImGui();  // 登録された全項目を一括表示、変更時に SetValue 自動呼び出し
binder.Save();   // SaveFile を呼ぶ
```

### 対応型と ImGui コントロール

| 型 | メソッド | ImGui コントロール |
|----|----------|-------------------|
| `float` | `Bind` | `DragFloat` |
| `int32_t` | `Bind` | `DragInt` |
| `bool` | `Bind` | `Checkbox` |
| `Vector2` | `Bind` | `DragFloat2` |
| `Vector3` | `Bind` | `DragFloat3` |
| `Vector4` | `Bind` | `DragFloat4` |
| `Vector4`（色） | `BindColor` | `ColorEdit4` |

### 複合型 Bind（CollapsingHeader で区分け）

```cpp
binder.BindTransform3D("Transform", &model_->GetTransform());
binder.BindMaterial("Material", &model_->GetMaterial());
binder.Bind("MoveSpeed", &moveSpeed_, 5.0f);
binder.ImGui();
```

表示イメージ：
```
▼ Transform         ← CollapsingHeader
    Position [x][y][z]
    Rotation [x][y][z]
    Scale    [x][y][z]
▼ Material          ← CollapsingHeader
    Color    [rgba ColorEdit4]
    LightingMode [None / Lambert / HalfLambert / PhongSpecular]
MoveSpeed [DragFloat]
[Save]
```

### JSON 保存構造

| バインド | JsonSettings パス | キー |
|---------|-------------------|------|
| `Bind("MoveSpeed", ...)` | `{ "TestPlayer" }` | `"MoveSpeed"` |
| `BindTransform3D("Transform", ...)` | `{ "TestPlayer", "Transform" }` | `"Position"` / `"Rotation"` / `"Scale"` |
| `BindMaterial("Material", ...)` | `{ "TestPlayer", "Material" }` | `"Color"` / `"LightingMode"` |

→ JsonSettings の subGroups を活用してネスト保存

### 内部構造

```cpp
class JsonBinder {
    std::string groupName_;
    std::vector<std::function<void()>> drawItems_;  // 登録順に描画
};
```

- `Bind()` / `BindTransform3D()` 等は呼ぶたびに `drawItems_` にラムダを push_back
- `ImGui()` は `drawItems_` を順番に実行 + 末尾に Save ボタン
- 通常 Bind → ラムダ 1 個、CollapsingHeader 系 → ラムダ 1 個（内部でヘッダ + 複数項目）

---

## Phase 一覧

| Phase | 内容 | 状態 |
|-------|------|------|
| 1 | `GameObjectTypes.h` + `GameObject.h/cpp` | ✅ 完了 |
| 2 | `GameObjectManager.h/cpp` | ✅ 完了 |
| 3 | `BaseScene.h` に `GameObjectManager` を追加 | ✅ 完了 |
| 4 | `TestPlayer.h/cpp`（WASD移動、JsonSettings 直接使用版） | ✅ 完了 |
| 5 | `DebugScene` に TestPlayer 配置 | ✅ 完了 |
| 6 | `vcxproj` にファイル・インクルードパスを追加 | ✅ 完了 |
| 7 | `JsonSettings::LoadFile` クラッシュ修正（assert → Logger） | ✅ 完了 |
| 8 | `JsonBinder.h/cpp` 作成 | ✅ 完了 |
| 9 | `TestPlayer` を JsonBinder 使用に書き換え + vcxproj 追加 | ✅ 完了 |
| 10 | FinalPass バッファ追加 + ImGui Game View 表示（エンジンエディタ化） | 🔲 未着手 |

---

## Phase 詳細（未着手分）

### Phase 10: FinalPass バッファ追加 + ImGui Game View 表示

**目的：** `#ifdef USEIMGUI` 時にゲーム画面（3D + スプライト + ポストエフェクト）を ImGui ウィンドウ内に表示する。
ドッキング機能により、Game View をウィンドウ内で自由に配置できる。

**前提の問題：**
- 現状の `StartDrawBackBuffer()` はスプライト等をスワップチェーン（バックバッファ）に直接書く
- オフスクリーンテクスチャの SRV には 3D 描画しか含まれず、スプライトが映らない

**解決策：FinalPass バッファ（RTV + SRV 兼用テクスチャ）を追加**

```
USEIMGUI 時の描画フロー（変更後）:

[StartDrawBackBuffer]
  FinalPass: SRV→RenderTarget バリア
  OMSetRenderTargets → FinalPass
  DrawOffscreenTexture() → FinalPass に 3D 合成

[game->DrawBackBuffer()]
  スプライト・UI → FinalPass に描画

[EndDrawBackBuffer]
  FinalPass: RenderTarget→SRV バリア
  dxCommon_->PreDraw() → スワップチェーン RT
  ImGui 描画（Game View ウィンドウ内に ImGui::Image）
  dxCommon_->PostDraw / EndFrame

非 USEIMGUI 時: 変更なし（スワップチェーンへ直接）
```

**変更ファイル：**
- `Engine/Core/GraphicsConfig.h` — `kRTVHeapSize` を 5→6（FinalPass RTV 分を追加）
- `Engine/Engine.h` — FinalPass メンバ追加（`#ifdef USEIMGUI` ガード）
- `Engine/Engine.cpp` — `CreateFinalPassBuffer()` / `StartDrawBackBuffer()` / `EndDrawBackBuffer()` / `ImGui()` / `Finalize()` 変更

**確認項目：**
- [ ] `USEIMGUI` 定義時に "Game View" ウィンドウが表示される
- [ ] Game View に 3D・スプライト・ポストエフェクトがすべて映る
- [ ] ドッキングでウィンドウを自由に移動・配置できる
- [ ] `USEIMGUI` なしのリリースパスで動作が変わらない
- [ ] アスペクト比を保って表示される

---

### Phase 7: JsonSettings::LoadFile クラッシュ修正

**変更ファイル：**
- `Engine/Core/JsonSettings/JsonSettings.cpp`

**変更内容：**
```
// 変更前
MessageBoxW(...);
assert(false);   ← ファイルなしでクラッシュ
return;

// 変更後
Logger::Log("[JsonSettings] ファイルが見つかりません: " + filePath);
return;          ← 静かに返るだけ（初回起動時の正常ケース）
```

**確認項目：**
- [ ] JSON ファイルが存在しない状態で起動してもクラッシュしない
- [ ] Logger にメッセージが出力される

---

### Phase 8: JsonBinder.h/cpp 作成

**作成ファイル：**
- `Engine/Utility/JsonBinder.h`
- `Engine/Utility/JsonBinder.cpp`

**公開 API：**

```cpp
// コンストラクタ（CreateGroup + LoadFile を自動実行）
explicit JsonBinder(const std::string& groupName);

// 基本型
void Bind(const std::string& key, float*    ptr, float    defaultValue, float speed = 0.1f);
void Bind(const std::string& key, int32_t*  ptr, int32_t  defaultValue, float speed = 1.0f);
void Bind(const std::string& key, bool*     ptr, bool     defaultValue);
void Bind(const std::string& key, Vector2*  ptr, const Vector2& defaultValue, float speed = 0.1f);
void Bind(const std::string& key, Vector3*  ptr, const Vector3& defaultValue, float speed = 0.1f);
void Bind(const std::string& key, Vector4*  ptr, const Vector4& defaultValue, float speed = 0.1f);
void BindColor(const std::string& key, Vector4* ptr, const Vector4& defaultValue);

// 複合型（CollapsingHeader）
void BindTransform3D(const std::string& sectionName, Transform3D* transform);
void BindMaterial(const std::string& sectionName, Material* material);

// 表示・保存
void ImGui();   // 全項目一括表示 + Save ボタン
void Save();    // JsonSettings::SaveFile を呼ぶ
```

**確認項目：**
- [ ] Bind した変数に JSON の値が自動適用される
- [ ] ImGui() で全項目が表示される
- [ ] 変更すると JsonSettings に即反映される
- [ ] Save() で JSON ファイルに書き出される
- [ ] BindTransform3D が CollapsingHeader で表示される
- [ ] BindMaterial が CollapsingHeader + LightingMode Combo で表示される

---

### Phase 9: TestPlayer を JsonBinder 使用に書き換え

**変更ファイル：**
- `Application/GameObject/TestPlayer/TestPlayer.h`
- `Application/GameObject/TestPlayer/TestPlayer.cpp`
- `mikamiEngine.vcxproj`（JsonBinder.cpp の ClCompile エントリ追加）

**書き換えイメージ（before → after）：**

```cpp
// --- Before（手書き） ---
// Initialize(): CreateGroup + AddItem × 4 + LoadFromJson + ApplyJsonValues
// ImGui():      型ごとに手書き ImGui + SetValue × 4 + Save ボタン（計 30 行以上）

// --- After（JsonBinder） ---
// Initialize():
binder_ = std::make_unique<JsonBinder>("TestPlayer");
binder_->BindTransform3D("Transform", &model_->GetTransform());
binder_->BindMaterial("Material", &model_->GetMaterial());
binder_->Bind("MoveSpeed", &moveSpeed_, 5.0f);

// ImGui():
binder_->ImGui();  // ← 1 行だけ
```

**不要になるもの：**
- `LoadFromJson()` メソッド（削除）
- `ApplyJsonValues()` メソッド（削除）
- ImGui の手書き `DragFloat3` / `ColorEdit4` / `SetValue` / Save ボタン（削除）

**確認項目：**
- [ ] 動作が書き換え前と変わらない
- [ ] BindTransform3D で Position/Rotation/Scale が操作できる
- [ ] BindMaterial で Color と LightingMode が操作できる
- [ ] 再起動後も値が維持される
