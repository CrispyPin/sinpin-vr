#pragma once
#define GL_GLEXT_PROTOTYPES

#include "grab_component.h"
#include "util.h"
#include <GLFW/glfw3.h>

const vr::HmdMatrix34_t DEFAULT_POSE = {{{1, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 0}}};

class App;

class Panel
{
  public:
	Panel(App *app, vr::HmdMatrix34_t start_pose, int index, int xmin, int xmax, int ymin, int ymax);

	void Update();
	void SetHidden(bool state);

  private:
	void Render();
	void UpdateCursor();

	App *_app;
	OverlayID _id;
	int _index;
	std::string _name;

	int _x, _y;
	int _width, _height;
	float _meters;
	float _alpha;
	bool _hidden;

	GrabComponent _grab_component;

	vr::Texture_t _texture;
	GLuint _gl_texture;
};