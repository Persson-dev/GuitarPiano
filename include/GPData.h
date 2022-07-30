#pragma once

#include <vector>
#include <cstdint>

namespace gpgui {
namespace data {

typedef std::vector<float> VertexData;

struct Color {
	std::uint8_t red, green, blue, alpha = 255;
};

struct DrawData {
	std::uint32_t vao, vbo;
	std::size_t vertexCount;
};

static const std::uint8_t EMPTY_TAB = 0xF;
static const std::uint8_t EMPTY_NOTE = 0xFF;

VertexData GetCircleData(float centerX, float centerY, float radius, float color, int precision);
VertexData GetRectData(float x, float y, float dx, float dy, float color);

DrawData GetDrawData(const VertexData& vertexData);
void UpdateData(DrawData& buffer, const VertexData& newData);

void DrawVertexData(const DrawData& vertexData);

float GetIntColor(const Color& color);

} // namespace data
} // namespace gpgui
