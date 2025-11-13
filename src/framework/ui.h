
#pragma once

#include "framework.h"
#include "includes.h"

namespace UI {
	bool addButton(Vector2 position, Vector2 size, const char* texture_path);
	void renderProgressBar(Vector2 position, Vector2 size, float progress, const Vector4& color);

};
