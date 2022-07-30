#include "GPRenderer.h"
#include "GPData.h"
#include "ShaderProgram.h"

#include <array>
#include <cmath>

namespace gpgui {
namespace renderer {

using VertexData = data::VertexData;
using Color = data::Color;

class GPShader : public ShaderProgram {
public:
	GPShader() : ShaderProgram() {}

	virtual void GetAllUniformLocation() {}
};

const char vertexShader[] = R"(
	#version 330

	layout (location = 0) in vec2 Position;
	layout (location = 1) in int Color;

	flat out int pass_color;

	void main() {
		pass_color = Color;
		gl_Position = vec4(Position.x * 2.0 - 1.0, Position.y * 2.0 - 1.0, 0.0, 1.0);
	}
)";


const char fragmentShader[] = R"(
	#version 330

	flat in int pass_color;

	out vec4 color;

	void main() {
		color = vec4(pass_color >> 24 & 0xFF, pass_color >> 16 & 0xFF, pass_color >> 8 & 0xFF, pass_color & 0xFF) / 255.0;
	}
)";

enum KeyType : std::uint8_t {
	Major = 0,
	Minor
};

static data::DrawData keyData, stringsData;
static GPShader gpShader;
static std::array<std::uint8_t, 7> highlitedKeys; // not 6 to avoid visual glitches with the strings
static std::array<std::uint8_t, 6> highlitedStrings;
static int capo = 0;

constexpr int KEY_NUMBER = 52;
constexpr float KEYBOARD_HEIGHT = 0.3;
constexpr float TAB_HEIGHT = 0.3;

bool IsKeyHighlited(int key) {
	bool result = (highlitedKeys[key / 8] >> (7 - (key % 8))) & 0x1;
	return result;
}

void SetKeyHighlight(int key, bool highlight) {
	bool value = IsKeyHighlited(key);
	if (value == highlight)
		return;
	highlitedKeys[key / 8] ^= (1 << (7 - (key % 8)));
}

void SetTab(int tab, int fret) {
	highlitedStrings[tab] = static_cast<std::uint8_t>(fret);
}

void ClearKeyboard() {
	highlitedKeys.fill(0);
}

void ClearTab() {
	highlitedStrings.fill(data::EMPTY_TAB);
}

int GetKeyFromWhite(int whiteIndex) {
	int octave = whiteIndex / 7;
	int whiteKey = whiteIndex % 7;
	int offset = whiteKey * 2;
	if (whiteKey >= 2)
		offset--; // first hole
	if (whiteKey >= 5)
		offset--; // second hole
	return octave * 12 + offset;
}

int GetKeyFromBlack(int blackIndex) {
	int octave = blackIndex / 7;
	int blackKey = blackIndex % 7;
	int offset = blackIndex * 2;
	// too lazy to do something good
	switch (blackKey) {
	case 0: {
		offset = 11;
		octave--;
		break;
	}
	case 1: {
		offset = 1;
		break;
	}
	case 3: {
		offset = 4;
		break;
	}
	case 4: {
		offset = 6;
		break;
	}
	case 6: {
		offset = 9;
		break;
	}
	}
	return octave * 12 + offset;
}

void SetCapoPos(int capoPos) {
	capo = capoPos;
}

VertexData GetKeyboardData() {
	VertexData vertexData;
	vertexData.reserve(2048);

	constexpr Color WHITE{ 255, 255, 255 };
	constexpr Color BLACK{ 0, 0, 0 };
	constexpr Color GREEN{ 132, 255, 0 };
	constexpr Color DARK_GREEN{ 57, 190, 0 };

	float whiteColor = data::GetIntColor(WHITE);

	// white background
	VertexData backgroundData = data::GetRectData(0.0f, 0.0f, 1.0f, KEYBOARD_HEIGHT, whiteColor);
	vertexData.insert(vertexData.end(), backgroundData.begin(), backgroundData.end());

	// highlited white keys
	constexpr int WHITE_KEYS_COUNT = 31;
	static float greenColor = data::GetIntColor(GREEN);
	for (int i = 0; i < WHITE_KEYS_COUNT; i++) {
		int touche = GetKeyFromWhite(i);

		if (!IsKeyHighlited(touche))
			continue;

		float x = (float)i / (float)WHITE_KEYS_COUNT;
		float dx = (float)(i + 1) / (float)WHITE_KEYS_COUNT;
		float y = 0.0f;
		float dy = KEYBOARD_HEIGHT;

		VertexData rectData = data::GetRectData(x, y, dx, dy, greenColor);
		vertexData.insert(vertexData.end(), rectData.begin(), rectData.end());
	}


	// black borders
	constexpr float BORDER_THIKNESS = 0.001;
	constexpr int BORDER_COUNT = 31;
	static float color = data::GetIntColor(BLACK);

	for (int i = 1; i < BORDER_COUNT; i++) {
		float centerX = (float)(i) / (float)BORDER_COUNT;
		float x = centerX - BORDER_THIKNESS / 2.0;
		float dx = centerX + BORDER_THIKNESS / 2.0;

		VertexData rectData = data::GetRectData(x, 0.0f, dx, KEYBOARD_HEIGHT, color);
		vertexData.insert(vertexData.end(), rectData.begin(), rectData.end());
	}

	// black keys
	constexpr float KEY_THIKNESS = 0.02;
	constexpr float KEY_HEIGHT = KEYBOARD_HEIGHT * 5.0 / 8.0;
	static float blackColor = data::GetIntColor(BLACK);

	for (int i = 1; i < BORDER_COUNT; i++) {
		if (i % 7 != 2 && i % 7 != 5) {
			float centerX = (float)(i) / (float)BORDER_COUNT;
			float x = centerX - KEY_THIKNESS / 2.0;
			float dx = centerX + KEY_THIKNESS / 2.0;

			VertexData rectData = data::GetRectData(x, KEYBOARD_HEIGHT - KEY_HEIGHT, dx, KEYBOARD_HEIGHT, blackColor);
			vertexData.insert(vertexData.end(), rectData.begin(), rectData.end());
		}
	}

	// highlited black keys
	constexpr float KEY_HIGHLIGHT_BORDER = 0.002f;
	static float darkGreenColor = data::GetIntColor(DARK_GREEN);
	for (int i = 1; i < BORDER_COUNT; i++) {
		if (i % 7 != 2 && i % 7 != 5) {
			int touche = GetKeyFromBlack(i);

			if (!IsKeyHighlited(touche))
				continue;

			float centerX = (float)(i) / (float)BORDER_COUNT;
			float x = centerX - KEY_THIKNESS / 2.0;
			float dx = centerX + KEY_THIKNESS / 2.0;

			VertexData rectData = data::GetRectData(x + KEY_HIGHLIGHT_BORDER, KEYBOARD_HEIGHT - KEY_HEIGHT + KEY_HIGHLIGHT_BORDER, dx - KEY_HIGHLIGHT_BORDER, KEYBOARD_HEIGHT, darkGreenColor);
			vertexData.insert(vertexData.end(), rectData.begin(), rectData.end());
		}
	}

	return vertexData;
}

VertexData GetStringsData() {
	VertexData vertexData;
	vertexData.reserve(2048);

	constexpr float TAB_OFFSET = KEYBOARD_HEIGHT;

	constexpr int STRING_COUNT = 6;

	constexpr Color WHITE{ 255, 255, 255 };
	constexpr Color GREY{ 237, 231, 223 };
	constexpr Color SILVER{ 176, 160, 148 };
	constexpr Color BROWN{ 80, 55, 55 };
	constexpr Color GREEN{ 132, 255, 0 };
	constexpr Color DARK_GREEN{ 57, 190, 0 };
	constexpr Color CAPO{ 95, 95, 95 };

	// brown background
	static const float brownColor = data::GetIntColor(BROWN);
	VertexData brownBackground = data::GetRectData(0.0f, TAB_OFFSET, 1.0f, TAB_OFFSET + TAB_HEIGHT, brownColor);
	vertexData.insert(vertexData.end(), brownBackground.begin(), brownBackground.end());

	// the 12 frets
	constexpr int FRET_COUNT = 12;
	constexpr float FRET_THIKNESS = 0.01;
	constexpr float FRET_START = 0.105;

	float silverColor = data::GetIntColor(SILVER);

	for (int i = 0; i < FRET_COUNT; i++) {
		float centerX = FRET_START;
		for (int j = 0; j < i; j++) {
			centerX += FRET_START / std::pow(2, (float)j / 12.0f);
		}
		float x = centerX - FRET_THIKNESS / 2;
		float dx = centerX + FRET_THIKNESS / 2;

		VertexData fretData = data::GetRectData(x, TAB_OFFSET, dx, TAB_OFFSET + TAB_HEIGHT, silverColor);
		vertexData.insert(vertexData.end(), fretData.begin(), fretData.end());
	}

	// the 6 strings
	constexpr float STRING_THIKNESS = 0.01;
	static const float darkGreenColor = data::GetIntColor(DARK_GREEN);
	static const float greyColor = data::GetIntColor(GREY);

	for (int i = 0; i < STRING_COUNT; i++) {
		float stringThikness = STRING_THIKNESS / (((float)i / 3.0f) + 1.0f);
		float centerY = TAB_OFFSET + (float)(i + 1) / (float)(STRING_COUNT + 1) * TAB_HEIGHT;
		float y = centerY - stringThikness / 2;
		float dy = centerY + stringThikness / 2;

		VertexData stringData = data::GetRectData(0.0f, y, 1.0f, dy, highlitedStrings[i] == data::EMPTY_TAB ? greyColor : darkGreenColor);
		vertexData.insert(vertexData.end(), stringData.begin(), stringData.end());
	}

	// drawing circles

	constexpr float circleRadius = 1.0f / (float)(FRET_COUNT + 3.0f) * TAB_HEIGHT - STRING_THIKNESS / 2.0f;

	auto getCircleCenterX = [](int fret) -> float {
		float circleCenterX = FRET_START;
		for (int i = 0; i < fret - 2; i++) {
			circleCenterX += FRET_START / std::pow(2, (float)i / 12.0f);
		}
		return circleCenterX + (FRET_START / std::pow(2, (float)((fret)-1.0f) / 12.0f)) / 2.0f;
	};

	auto getCircleCenterY = [](int string) -> float {
		float centerY = TAB_OFFSET + (float)(string + 1) / (float)(STRING_COUNT + 1) * TAB_HEIGHT;
		centerY -= (1 / (float)(STRING_COUNT + 1) * TAB_HEIGHT) / 2.0f;
		return centerY;
	};

	static const std::vector<int> circlesX = { 3, 5, 7, 9 };
	for (int i = 0; i < circlesX.size(); i++) {
		auto circleData = data::GetCircleData(getCircleCenterX(circlesX[i]), TAB_OFFSET + TAB_HEIGHT / 2.0f, circleRadius, data::GetIntColor(WHITE), 20);
		vertexData.insert(vertexData.end(), circleData.begin(), circleData.end());
	}

	static const std::vector<int> circlesY = { 1, 5 };
	for (int i = 0; i < circlesY.size(); i++) {
		auto circleData = data::GetCircleData(getCircleCenterX(12), getCircleCenterY(circlesY[i]), circleRadius, data::GetIntColor(WHITE), 20);
		vertexData.insert(vertexData.end(), circleData.begin(), circleData.end());
	}

	// fret highlight
	float greenColor = data::GetIntColor(GREEN);
	for (int i = 0; i < STRING_COUNT; i++) {

		if (highlitedStrings[i] == data::EMPTY_TAB || highlitedStrings[i] == 0)
			continue;

		int position = highlitedStrings[i];

		float centerY = TAB_OFFSET + (float)(i + 1) / (float)(STRING_COUNT + 1) * TAB_HEIGHT;
		float y = centerY - (1.0f / (float)(STRING_COUNT + 1) * TAB_HEIGHT) / 2;
		float dy = centerY + (1.0f / (float)(STRING_COUNT + 1) * TAB_HEIGHT) / 2;

		float centerX = FRET_START;
		for (int j = 0; j < position - 1; j++) {
			centerX += FRET_START / std::pow(2, (float)j / 12.0f);
		}
		float x = centerX - FRET_THIKNESS / 2 * 6;
		float dx = centerX - FRET_THIKNESS / 2;

		VertexData fretData = data::GetRectData(x, y, dx, dy, greenColor);
		vertexData.insert(vertexData.end(), fretData.begin(), fretData.end());
	}

	// draw capo
	static float capoColor = data::GetIntColor(CAPO);
	constexpr float CAPO_OFFSET = 0.01;

	if (capo != 0) {
		float centerX = FRET_START;
		for (int j = 0; j < capo - 1; j++) {
			centerX += FRET_START / std::pow(2, (float)j / 12.0f);
		}
		centerX -= CAPO_OFFSET;
		float x = centerX - FRET_THIKNESS / 2 * 6;
		float dx = centerX - FRET_THIKNESS / 2;

		VertexData fretData = data::GetRectData(x, TAB_OFFSET, dx, TAB_OFFSET + TAB_HEIGHT, capoColor);
		vertexData.insert(vertexData.end(), fretData.begin(), fretData.end());
	}

	return vertexData;
}

void InitRendering() {
	gpShader.LoadProgram(vertexShader, fragmentShader);

	ClearTab();

	keyData = data::GetDrawData(GetKeyboardData());
	stringsData = data::GetDrawData(GetStringsData());
}

void UpdateBuffers() {
	data::UpdateData(keyData, GetKeyboardData());
	data::UpdateData(stringsData, GetStringsData());
}

void DrawWidgets() {
	gpShader.Start();
	data::DrawVertexData(keyData);
	data::DrawVertexData(stringsData);
	gpShader.Stop();
}

} // namespace renderer
} // namespace gpgui
