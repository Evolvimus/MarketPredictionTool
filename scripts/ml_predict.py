import sys
import json
import numpy as np
from sklearn.ensemble import RandomForestClassifier

def sigmoid(x):
    return 1 / (1 + np.exp(-x))

def calibrate_probability(score):
    """
    Maps a raw bias score (e.g. 0.4 - 0.8) to a calibrated probability.
    Uses a centered sigmoid for more realistic 'edge' detection.
    """
    # Centered at 0.5, scaled to make 0.7 feel like a solid probability
    return sigmoid((score - 0.5) * 8)

def classify_regime(features):
    """
    Modell 1: Adaptive Market Regime Classifier
    Uses combined state vectors from C++.
    """
    # Raw states from C++ [-1, 1] or [0, 1]
    trend_state = features.get("trend_state", 0)
    vol_state = features.get("volatility_state", 0)
    mom_state = features.get("momentum_state", 0)
    
    regime = "range"
    confidence = 0.5
    
    # Simple Logic on robust State Vectors
    if vol_state > 0.7:
        regime = "high_vol"
        confidence = vol_state
    elif abs(trend_state) > 0.4:
        regime = "trend"
        confidence = min(0.95, abs(trend_state) * 1.5)
    else:
        regime = "range"
        confidence = 1.0 - abs(trend_state)
    
    return {
        "regime": regime, 
        "confidence": round(confidence, 2),
        "continuous_score": round(trend_state, 2)
    }

def predict_direction(features, regime_info):
    """
    Modell 2: Directional Model with Probability Calibration
    Uses Market State Vectors.
    """
    regime = regime_info["regime"]
    
    # Base states
    mom_state = features.get("momentum_state", 0) # [-1, 1]
    trend_state = features.get("trend_state", 0)  # [-1, 1]
    
    # 1. Base Score (Neutral 0.5)
    # Map [-1, 1] to [0, 1]
    raw_score = 0.5 + (mom_state * 0.3) + (trend_state * 0.2)
    
    # Regime adjustments
    if regime == "trend":
        # In trend, follow the trend state more
        raw_score += (trend_state * 0.2)
    elif regime == "range":
        # In range, look for mean reversion (reversal of momentum if overextended)
        # simplistic: if mom is extreme, expect revert? 
        # For now, let's keep it momentum based but dampen it.
        pass
        
    # Clamp
    raw_score = max(0.0, min(1.0, raw_score))

    # Calibration
    calibrated_prob = calibrate_probability(raw_score)
    
    # Decision
    direction = "neutral"
    if calibrated_prob > 0.60: direction = "long"
    elif calibrated_prob < 0.40: direction = "short"
    
    # Probability map to >0.5
    final_prob = calibrated_prob if direction == "long" else (1.0 - calibrated_prob)
    if direction == "neutral": final_prob = 0.5

    # Expected R (Risk:Reward) based on Regime
    reward_risk_ratio = 1.0
    if regime == "trend":
        reward_risk_ratio = 2.5
    elif regime == "range":
        reward_risk_ratio = 1.5
    elif regime == "high_vol":
        reward_risk_ratio = 3.0 # High risk high reward

    # EV Calculation
    # EV = (Win% * Reward) - (Loss% * Risk)
    # Risk is 1.0 unit. Reward is RR * 1.0
    win_rate = final_prob
    ev = (win_rate * reward_risk_ratio) - ((1.0 - win_rate) * 1.0)
    
    # Signal Strength (Volatility adjusted bias)
    # Signal Strength = EV * Confidence
    sig_strength = ev * regime_info["confidence"]

    return {
        "direction": direction,
        "probability": round(final_prob, 2),
        "expected_r": reward_risk_ratio,
        "expected_value": round(ev, 2),
        "signal_strength": round(sig_strength, 2),
        "confidence": round(regime_info["confidence"], 2)
    }

def main():
    try:
        input_data = json.load(sys.stdin)
        features = input_data.get("features", {})
        
        regime_result = classify_regime(features)
        dir_result = predict_direction(features, regime_result)
        
        output = {
            "regime_model": regime_result,
            "directional_model": dir_result
        }
        
        print(json.dumps(output))
    except Exception as e:
        print(json.dumps({"error": str(e)}))

if __name__ == "__main__":
    main()
