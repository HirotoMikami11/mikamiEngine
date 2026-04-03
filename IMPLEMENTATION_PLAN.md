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

---

# Collision システム リファクタリング計画（完了）

> Phase 1〜4 は完了済み。以下フェーズから継続。

---

# Collision システム 拡張計画

## 目標

- `ColliderType` enum に `Count` を追加し `kColliderTypeCount` をハードコードから解放
- 旧 `Collider.h/.cpp` を削除
- `CollisionManager` に Enter / Stay / Exit の3段階コールバックを追加
- `TestPlayer` に SphereCollider を組み込み（音・色の動作確認）
- `TestWall` と `TestObject` を新規作成（DebugObject フォルダ）

---

## 設計方針

### ColliderType::Count 化

```cpp
enum class ColliderType {
    SPHERE  = 0,
    AABB    = 1,
    OBB     = 2,
    SPRITE  = 3,
    CAPSULE = 4,
    Count        // ← 追加：配列サイズに使用
};
// kColliderTypeCount を削除し static_cast<int>(ColliderType::Count) を直接使う
```

`CollisionManager.h` の配列定義も変更:
```cpp
CollisionFunc dispatchTable_[static_cast<int>(ColliderType::Count)][static_cast<int>(ColliderType::Count)] = {};
```

---

### Enter / Stay / Exit の実装方針

`CollisionManager` が `currentPairs_`（今フレーム）と `prevPairs_`（前フレーム）の  
`std::set<std::pair<ICollider*, ICollider*>>` を保持する。  
ポインタは常に `min(a, b) → max(a, b)` の順に正規化してペアを作る。

```
今フレームにある && 前フレームにない → OnCollisionEnter(other)
今フレームにある && 前フレームにもある → OnCollisionStay(other)
今フレームにない && 前フレームにある → OnCollisionExit(other)
```

`ICollider` に追加する仮想関数:
```cpp
virtual void OnCollisionEnter(ICollider* other) {}
virtual void OnCollisionStay(ICollider* other) {}
virtual void OnCollisionExit(ICollider* other) {}
```

既存の `OnCollision()` は**削除**し、3種に置き換える。

---

### ファイル配置（最終形）

```
Application/
└── CollisionManager/
    ├── Collision.h/.cpp           ← 移動済み（変更なし）
    ├── CollisionConfig.h          ← 変更なし
    ├── CollisionManager.h/.cpp    ← Enter/Stay/Exit 対応
    ├── Collider.h/.cpp            ← 削除対象（旧クラス）
    └── Collider/
        ├── ICollider.h            ← Count 追加・3コールバック追加
        ├── SphereCollider.h/.cpp
        ├── AABBCollider.h/.cpp
        ├── OBBCollider.h          ← スタブ
        └── SpriteCollider.h       ← スタブ

Application/
└── GameObject/
    ├── TestPlayer/
    │   ├── TestPlayer.h/.cpp      ← SphereCollider 組み込み・音・色
    └── DebugObject/               ← 新設フォルダ
        ├── TestWall/
        │   ├── TestWall.h/.cpp    ← AABBCollider・Json・ImGui
        └── TestObject/
            ├── TestObject.h/.cpp  ← SphereCollider・Json・ImGui
```

---

## Phase A: ICollider の修正（Count 追加・コールバック整理）

- [ ] `ColliderType` に `Count` を追加、`kColliderTypeCount` を削除
- [ ] `ICollider` の `OnCollision()` を削除、`OnCollisionEnter/Stay/Exit` を追加
- [ ] `CollisionManager.h` の配列定義を `ColliderType::Count` ベースに変更

## Phase B: CollisionManager に Enter / Stay / Exit ロジック追加

- [ ] `prevPairs_` / `currentPairs_` を `std::set<std::pair<ICollider*,ICollider*>>` で追加
- [ ] `Update()` でフレーム比較して3種のコールバックを振り分け
- [ ] 色変更は Stay のときだけ行う（Enter も含めてよい）

## Phase C: 旧 Collider.h/.cpp を削除

- [ ] `Collider.h` / `Collider.cpp` を削除

## Phase D: TestPlayer に SphereCollider 組み込み

- [ ] `SphereCollider` を継承、`GetWorldPosition()` を実装
- [ ] `OnCollisionEnter` → `PlayerHit` 音再生
- [ ] `OnCollisionStay` → モデル色を赤に
- [ ] `OnCollisionExit` → `Explosion` 音再生、色を戻す
- [ ] `radius` を JsonBinder でバインド

## Phase E: DebugObject フォルダ作成・TestWall / TestObject 実装

**TestWall**
- `AABBCollider` を継承
- Object3D（Box）でビジュアル
- Transform・AABB サイズを JsonBinder で管理
- ImGui で数値変更可能

**TestObject**
- `SphereCollider` を継承
- Object3D（Sphere）でビジュアル
- Transform・radius を JsonBinder で管理
- ImGui で数値変更可能

- [ ] `DebugObject/TestWall/TestWall.h/.cpp` 作成
- [ ] `DebugObject/TestObject/TestObject.h/.cpp` 作成

## Phase F: Scene への登録・動作確認

- [ ] `DebugScene` か `GameScene` で TestPlayer / TestWall / TestObject を登録
- [ ] CollisionManager に3オブジェクトを AddCollider
- [ ] 動作確認：TestPlayer が TestWall / TestObject に触れたときに音と色変化

## 目標

- `CollisionManager::CheckCollisionPair()` の型ごとの if-else 分岐を撤廃
- 衝突判定数学ロジックを `MyFunction` から独立した `Collision` クラスに分離
- `Collider` を `ICollider` 基底 + 型別派生クラスに分割（OBB・Sprite 追加を見据えて）
- 型追加時の変更箇所を `RegisterCollisionHandlers()` の1箇所に限定する

## 設計方針

- dispatch table: `CollisionFunc dispatchTable_[TYPE_COUNT][TYPE_COUNT]`（2次元配列 + 関数ポインタ）
- `ColliderType` enum は連番を維持する（配列インデックスとして使用するため）
- マスクチェック → 型正規化 → テーブル参照 → 数学計算 の順で処理

## ファイル構成（最終形）

```
Engine/
└── Collision/
    ├── Collision.h      ← 新設: IsCollision 静的メソッド群
    └── Collision.cpp    ← 新設: MyFunction から IsCollision 系を移植

Application/
└── CollisionManager/
    ├── ICollider.h/.cpp        ← Collider.h を改名・共通部のみ残す
    ├── SphereCollider.h/.cpp   ← 新設: radius_ を持つ
    ├── AABBCollider.h/.cpp     ← 新設: aabb_ を持つ
    ├── OBBCollider.h           ← 新設: スタブ（未実装）
    ├── SpriteCollider.h        ← 新設: スタブ（未実装）
    ├── CollisionManager.h/.cpp ← dispatch table に書き換え
    └── CollisionConfig.h       ← 変更なし
```

## Phase 1: Collision クラスの新設

**対象ファイル**
- 新規作成: `Engine/Collision/Collision.h`
- 新規作成: `Engine/Collision/Collision.cpp`

**作業内容**
- `Collision` クラスを static メソッドのみで構成
- `MyFunction.h` の `IsCollision` 系・`FixAABBMinMax` の宣言をコピー
- `MyFunction.cpp` の実装を移植

**Collision クラスのメソッド一覧**
```cpp
class Collision {
public:
    static bool IsCollision(const SphereMath& a, const SphereMath& b);
    static bool IsCollision(const SphereMath& sphere, const PlaneMath& plane);
    static bool IsCollision(const SphereMath& sphere, const AABB& aabb);
    static bool IsCollision(const AABB& a, const AABB& b);
    static bool IsCollision(const AABB& aabb, const Segment& seg);
    static bool IsCollision(const AABB& aabb, const Vector3& point);
    static bool IsCollision(const Segment& seg, const PlaneMath& plane);
    static bool IsCollision(const TriangleMath& tri, const Segment& seg);
};
```

- [ ] `Collision.h` 作成
- [ ] `Collision.cpp` 作成（MyFunction から実装を移植）

## Phase 2: ICollider + 派生クラスの作成

**対象ファイル**
- `Collider.h/.cpp` → `ICollider.h/.cpp` に改名・リファクタリング
- 新規作成: `SphereCollider.h/.cpp`
- 新規作成: `AABBCollider.h/.cpp`
- 新規作成: `OBBCollider.h`（スタブ）
- 新規作成: `SpriteCollider.h`（スタブ）

**ICollider（基底）が持つもの**
- `OnCollision()`, `GetWorldPosition()`, `DebugLineAdd()` の仮想メソッド
- `colliderType_`, `collisionAttribute_`, `collisionMask_`, 色系メンバー

**SphereCollider が追加で持つもの**
- `radius_`
- `GetRadius()`, `SetRadius()`
- `DebugLineAdd()` のオーバーライド（DrawSphere）

**AABBCollider が追加で持つもの**
- `aabb_`
- `GetAABB()`, `SetAABB()`, `SetAABBSize()`
- `DebugLineAdd()` のオーバーライド（DrawAABB）

- [ ] `ICollider.h/.cpp` 作成
- [ ] `SphereCollider.h/.cpp` 作成
- [ ] `AABBCollider.h/.cpp` 作成
- [ ] `OBBCollider.h` 作成（スタブ）
- [ ] `SpriteCollider.h` 作成（スタブ）
- [ ] 旧 `Collider.h/.cpp` を削除（または `ICollider.h` への転送ヘッダーとして残す）

## Phase 3: CollisionManager を dispatch table に書き換え

**対象ファイル**
- `CollisionManager.h`
- `CollisionManager.cpp`

**CollisionManager.h の変更点**
```cpp
// 追加
static constexpr int kColliderTypeCount = 5; // enum の種類数
using CollisionFunc = bool(*)(ICollider*, ICollider*);

// メンバー変更
std::list<ICollider*> colliders_;
CollisionFunc dispatchTable_[kColliderTypeCount][kColliderTypeCount] = {};

// 追加メソッド
void RegisterCollisionHandlers();
```

**CheckCollisionPair() の処理順**
1. マスクフィルタ（ビット演算、最安）
2. 型正規化（`typeA > typeB` なら swap）
3. `dispatchTable_[typeA][typeB]` を参照
4. `nullptr` なら未実装ペアとしてスキップ
5. `func(a, b)` で判定 → hit なら `OnCollision()` + 色変更

**RegisterCollisionHandlers() で登録するペア**
| インデックス | ペア | 関数 |
|---|---|---|
| [SPHERE][SPHERE] | Sphere vs Sphere | Collision::IsCollision(sphere, sphere) |
| [SPHERE][AABB] | Sphere vs AABB | Collision::IsCollision(sphere, aabb) |
| [AABB][AABB] | AABB vs AABB | Collision::IsCollision(aabb, aabb) |

- [ ] `CollisionManager.h` に型エイリアス・配列を追加
- [ ] `RegisterCollisionHandlers()` を実装
- [ ] `CheckCollisionPair()` を dispatch table 方式に書き換え
- [ ] コンストラクタで `RegisterCollisionHandlers()` を呼ぶ

## Phase 4: MyFunction から IsCollision 系を削除

**対象ファイル**
- `Engine/MyMath/MyFunction.h`
- `Engine/MyMath/MyFunction.cpp`

**削除する宣言・実装**
- `IsCollision(SphereMath, SphereMath)`
- `IsCollision(SphereMath, PlaneMath)`
- `IsCollision(Segment, PlaneMath)`
- `IsCollision(TriangleMath, Segment)`
- `IsCollision(AABB, AABB)`
- `IsCollision(AABB, SphereMath)`
- `IsCollision(AABB, Segment)`
- `IsCollision(AABB, Vector3)`
- `FixAABBMinMax()`（Collision.h へ移動）
- `MakeCollisionPoint()` は数学ユーティリティとして MyFunction に残す

- [ ] `MyFunction.h` から削除
- [ ] `MyFunction.cpp` から削除
- [ ] ビルドエラーがないか確認


---