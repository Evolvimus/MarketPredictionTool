# Implementation Plan - Quantum Chatbot & Visuals

Enhance the `quantum.html` visualization with better visibility and add an integrated AI Chatbot to explain the market state.

## 1. 3D Visualization Upgrade
- **Problem**: "Das Modell rechts erkennt man null" (Right model is invisible).
- **Solution**:
  - Change material to `MeshPhongMaterial` with `flatShading: true` for a low-poly, tech look.
  - Increase opacity and line width of wireframe.
  - Add a "Ground Grid" for depth reference.
  - Add brighter lighting.

## 2. Integrated AI Chatbot
- **Frontend**:
    - Add a floating Chat Widget (Bottom Right).
    - Auto-send current `marketState` (Momentum, Trend, Volatility) with the user's question.
- **Backend**:
    - [MODIFY] `src/ollama_client.hpp/cpp`: Add `ask_question(context, question)` method.
    - [MODIFY] `src/server.cpp`: Add `/api/chat_quantum` endpoint.
    - **Prompt Engineering**: The AI will act as a "Quantum Physicist" explaining the market field.

## Proposed Changes

### [MODIFY] [src/ollama_client.hpp](file:///Users/nikola/predict/src/ollama_client.hpp)
- Add `std::string ask_question(const std::string &system_prompt, const std::string &user_message);`

### [MODIFY] [src/ollama_client.cpp](file:///Users/nikola/predict/src/ollama_client.cpp)
- Implement `ask_question` using the existing CURL logic but with custom prompts.

### [MODIFY] [src/server.cpp](file:///Users/nikola/predict/src/server.cpp)
- Add `/api/chat_quantum` endpoint.
- Extract `marketState` from request, build a prompt describing the "Field", and call `ask_question`.

### [MODIFY] [public/quantum.html](file:///Users/nikola/predict/public/quantum.html)
- Improve Three.js materials/lights.
- Add Chat UI (HTML/CSS).
- Add JS logic to talk to `/api/chat_quantum`.

## User Review
> [!NOTE]
> The chatbot will know strictly about the displayed data (Momentum X, Trend Y, Volatility Z) and explain it in terms of "Potential Energy" and "Stability", avoiding generic trading advice.
