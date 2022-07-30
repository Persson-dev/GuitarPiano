#include "GPData.h"

#include <GL/glew.h>

#include <cmath>
#include <cstring>

namespace gpgui {
namespace data {

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

VertexData GetRectData(float x, float y, float dx, float dy, float color) {
	return {
		dx, dy, color,
		x, dy, color,
		dx, y, color,

		x, dy, color,
		x, y, color,
		dx, y, color,
	};
}

VertexData GetCircleData(float centerX, float centerY, float radius, float color, int precision) {
	VertexData vertexData;
	vertexData.reserve(9 * precision);

	for (int i = 0; i < precision; i++) {
		float theta = 2.0f * M_PI * float(i) / float(precision); // get the current angle
		float thetaNext = 2.0f * M_PI * float(i + 1) / float(precision); // get the next angle

		float x = radius * cosf(theta);  // calculate the x component
		float y = radius * sinf(theta);  // calculate the y component

		float dx = radius * cosf(thetaNext);  // calculate the next x component
		float dy = radius * sinf(thetaNext);  // calculate the next y component

		vertexData.insert(vertexData.end(), {
			x + centerX, y + centerY, color,
			centerX, centerY, color,
			dx + centerX, dy + centerY, color,
		});
	}

	return vertexData;
}

float GetIntColor(const Color& color) {
	float out_color;
	int temp_color = color.red << 24 | color.green << 16 | color.blue << 8 | color.alpha;
	std::memcpy(&out_color, &temp_color, sizeof(float));
	return out_color;
}

DrawData GetDrawData(const VertexData& vertexData) {
	DrawData drawData;

	glGenVertexArrays(1, &drawData.vao);
	glGenBuffers(1, &drawData.vbo);

	drawData.vertexCount = vertexData.size() / 3;

	constexpr GLsizei stride = sizeof(float) * 3;

	glBindVertexArray(drawData.vao);
	glBindBuffer(GL_ARRAY_BUFFER, drawData.vbo);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)0);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 2));
	glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	return drawData;
}

void UpdateData(DrawData& buffer, const VertexData& newData) {
	glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
	glBufferData(GL_ARRAY_BUFFER, newData.size() * sizeof(float), newData.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// OMFG 1 hour of debugging to add this line
	buffer.vertexCount = newData.size() / 3;
}

void DrawVertexData(const DrawData& vertexData) {
	glBindVertexArray(vertexData.vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.vertexCount);
	glBindVertexArray(0);
}

} // namespace data
} // namespace gpgui
