# Task: Debugging Analysis History Loading

## Goal
Fix the "Fehler beim Laden" (Error Loading) issue in the Analysis History section.

## Todo List
- [x] Un-nest `updateSurface` in `quantum.html` <!-- id: 70 -->
- [x] Fix Three.js accessor error in `quantum.html` <!-- id: 71 -->
- [x] Implement Chatbot backend <!-- id: 72 -->
- [/] Debug Analysis History loading <!-- id: 82 -->
    - [ ] Identify frontend fetch endpoint for history <!-- id: 83 -->
    - [ ] Verify backend endpoint `/api/history` (or similar) implementation <!-- id: 84 -->
    - [ ] Check `analyses.json` validity <!-- id: 85 -->
    - [ ] Check parsing logic in `AnalysisStorage` <!-- id: 86 -->
