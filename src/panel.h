#define GL_GLEXT_PROTOTYPES
#include "util.h"
#include <GLFW/glfw3.h>
#include <string>

const vr::HmdMatrix34_t DEFAULT_POSE = {{{1, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 1, 0}}};

class App;

class Panel
{
  public:
	Panel(App *app, vr::HmdMatrix34_t start_pose, int index, int xmin, int xmax, int ymin, int ymax);

	void Update();

  private:
	void Render();
	void UpdateCursor();
	void ControllerGrab(TrackerID);
	void ControllerRelease();

	App *_app;
	OverlayID _id;
	int _index;
	std::string _name;

	TrackerID _active_hand;
	bool _is_held;

	int _x, _y;
	int _width, _height;
	float _alpha;

	vr::Texture_t _texture;
	GLuint _gl_texture;
};