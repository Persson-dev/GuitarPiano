#pragma once

namespace gpgui {
namespace renderer {

void InitRendering();

void DrawWidgets();
void DrawStrings();

bool IsKeyHighlited(int key);
void SetKeyHighlight(int key, bool highlight);
void ClearKeyboard();

void UpdateBuffers();

void ClearTab();
void SetTab(int tab, int fret);

void SetCapoPos(int capo);

} // namespace renderer
} // namespace gpgui
