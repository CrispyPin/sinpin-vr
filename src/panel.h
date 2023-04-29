#pragma once
#include "overlay.h"
#define GL_GLEXT_PROTOTYPES

#include "util.h"
#include <GLFW/glfw3.h>

const VRMat DEFAULT_POSE = {{{1, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 0}}};

class App;
class Overlay;

class Panel
{
  public:
	Panel(App *app, int index, int xmin, int xmax, int ymin, int ymax);

	void Update();
	void SetHidden(bool state);
	void ResetTransform();

	int Width()
	{
		return _width;
	}
	int Height()
	{
		return _height;
	}

	void SetCursor(int x, int y);

	Ray IntersectRay(glm::vec3 origin, glm::vec3 direction, float max_len);

	Overlay *GetOverlay();

  private:
	void Render();
	void UpdateCursor();

	App *_app;
	int _index;

	int _x, _y;
	int _width, _height;

	Overlay _overlay;

	vr::Texture_t _texture;
	GLuint _gl_texture;
};