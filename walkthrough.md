# Quantum Market Field Visualization

I have deployed the "Physics of AI" inspired visualization.

## Access
- URL: `http://localhost:8080/quantum.html`

## Visual Guide

### Left Panel: "The Neural State"
- A 2D network of pulsing nodes.
- Represents the high-dimensional, chaotic internal state of the ML models and data inputs.
- Purely symbolic of the "hidden layers" computing the potential.

### Right Panel: "The Potential Field"
A 3D dynamic surface representing the market's energy landscape.

1.  **The Shape (Regime Potential)**:
    - **Deep Channel/Valley**: Represents a strong **Trend**. The market flow is constrained and directed (stable High Energy).
    - **Flat Bowl**: Represents a **Range**. The market is trapped in a local minimum with no direction.

2.  **The Tilt (Momentum)**:
    - The entire surface tilts Left (Bearish) or Right (Bullish).
    - The "Ball" (Current State) rolls downhill based on this gradient.

3.  **The Texture (Volatility)**:
    - **Smooth Surface**: Low volatility, high certainty.
    - **Ripples/Waves**: High volatility, uncertain "Superposition" state.

4.  **The Ball (Observation)**:
    - Represents the collapsed state after measurement (Analysis).
    - **Green Glow**: Trend Regime.
    - **Yellow Glow**: Range Regime.
    - **Red Glow**: High Risk / Volatility.

## Controls
- Use the input box to change the Ticker (e.g., `ETH-USD`, `TSLA`).
- Click **OBSERVE STATE** to trigger a new analysis (collapse the wavefunction).
- Use **Mouse Drag** to rotate the 3D view and inspect the potential well structure.
