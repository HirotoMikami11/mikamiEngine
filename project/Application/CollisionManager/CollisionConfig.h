#pragma once
#include <cstdint>


///*-----------------------------------------------------------------------*///
///							衝突属性のフラグを定義する							///
///*-----------------------------------------------------------------------*///

// プレイヤー陣営
// 0b00000001
const uint32_t kCollisionAttributePlayer = 0b1;

// 敵陣営
// 0b00000010
const uint32_t kCollisionAttributeEnemy = 0b1 << 1;	//左シフト

//その他オブジェクト(柱など)
// 0b00000100
const uint32_t kCollisionAttributeObjects = 0b1 << 2;	//左シフト

// プレイヤーの弾
// 0b00001000
const uint32_t kCollisionAttributePlayerBullet = 0b1 << 3;	//左シフト