#include "resources/Shader/FullscreenTriangle/FullscreenTriangle.hlsli"

// 燃焼エフェクトパラメータ構造体
struct BurnParameters
{
    float32_t4 burnColor; // 炎の主色（オレンジ系）
    float32_t4 edgeColor; // 燃焼境界の色（黄色系、炎の外縁）
    
    float32_t progress; // 燃焼の進行度（0.0f～1.0f）
    float32_t edgeWidth; // 燃焼境界の幅（炎のグラデーション範囲）
    float32_t noiseScale; // ノイズのスケール（燃焼パターンの細かさ）
    float32_t noiseStrength; // ノイズの強度（燃焼パターンの不規則性）
    
    float32_t2 burnCenter; // 燃焼の中心点（UV座標、デフォルトは画面中央）
    float32_t time; // アニメーション用の時間（炎の揺らぎ）
    float32_t burnSpeed; // 燃焼の速度（自動再生時の進行速度）
    
    float32_t2 padding; // パディング（16バイト境界合わせ）
};

// 簡易パーリンノイズ風のハッシュ関数
// 2次元座標から擬似ランダムな値（0.0f～1.0f）を生成
float32_t Hash(float32_t2 p)
{
    p = frac(p * float32_t2(123.34f, 456.21f));
    p += dot(p, p + 45.32f);
    return frac(p.x * p.y);
}

// スムーズなノイズ生成関数
// バイリニア補間を使用して滑らかなノイズを生成
float32_t SmoothNoise(float32_t2 uv)
{
    float32_t2 i = floor(uv);
    float32_t2 f = frac(uv);
    
    // グリッドの4つの角の値を取得
    float32_t a = Hash(i);
    float32_t b = Hash(i + float32_t2(1.0f, 0.0f));
    float32_t c = Hash(i + float32_t2(0.0f, 1.0f));
    float32_t d = Hash(i + float32_t2(1.0f, 1.0f));
    
    // スムーズな補間（smoothstep関数相当）
    float32_t2 u = f * f * (3.0f - 2.0f * f);
    
    // バイリニア補間
    return lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);
}

// フラクタルノイズ（複数のオクターブを重ねる）
// より複雑で自然な見た目のノイズを生成
float32_t FractalNoise(float32_t2 uv, float32_t scale, int octaves)
{
    float32_t value = 0.0f;
    float32_t amplitude = 0.5f;
    float32_t frequency = scale;
    
    // オクターブを重ねて詳細度を追加
    for (int i = 0; i < octaves; i++)
    {
        value += amplitude * SmoothNoise(uv * frequency);
        frequency *= 2.0f; // 周波数を2倍に（より細かいディテール）
        amplitude *= 0.5f; // 振幅を半分に（影響度を減らす）
    }
    
    return value;
}

// 中心からの距離を計算（円形の燃焼パターン用）
// UV座標空間での距離を返す
float32_t CalculateDistanceFromCenter(float32_t2 uv, float32_t2 center)
{
    float32_t2 delta = uv - center;
    return length(delta);
}

// 燃焼パターンを計算
// 戻り値: 0.0f = 燃えていない、1.0f = 完全に燃えた
float32_t CalculateBurnPattern(float32_t2 uv, BurnParameters params)
{
    // 1. 中心からの距離を計算
    float32_t distance = CalculateDistanceFromCenter(uv, params.burnCenter);
    
    // 2. フラクタルノイズを生成して不規則な燃焼パターンを作成
    float32_t noise = FractalNoise(uv, params.noiseScale, 3);
    
    // 3. ノイズで距離を歪ませる（炎の不規則な形状を表現）
    float32_t distortedDistance = distance + (noise - 0.5f) * params.noiseStrength;
    
    // 4. 時間に基づくアニメーション（炎の揺らぎを表現）
    float32_t timeNoise = FractalNoise(uv * 2.0f + float32_t2(params.time * 0.5f, params.time * 0.3f), 3.0f, 2);
    distortedDistance += (timeNoise - 0.5f) * 0.05f;
    
    // 5. progressに基づいて燃焼の閾値を計算
    // progress=0.0f: 閾値が非常に小さい（画面外）→ 何も燃えない
    // progress=1.0f: 閾値が非常に大きい（画面全体）→ すべて燃える
    // 最大距離を√2（画面の対角線の長さ）として計算
    float32_t maxDistance = 1.414f;
    float32_t burnThreshold = params.progress * (maxDistance + params.edgeWidth * 2.0f);
    
    // 6. 燃焼パターン値を計算
    // 戻り値の意味：
    // - 0.0f: 燃えていない（元の画像）
    // - 0.0f～1.0f: 燃焼境界（炎の色）
    // - 1.0f: 完全に燃えた（黒）
    float32_t burnValue = saturate((burnThreshold - distortedDistance) / params.edgeWidth);
    
    return burnValue;
}
