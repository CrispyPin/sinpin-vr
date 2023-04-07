#define GL_GLEXT_PROTOTYPES
#include "util.h"
#include <GLFW/glfw3.h>
#include <X11/Xutil.h>
#include <vector>

class Panel;

struct CursorPos
{
	int x, y;
};

class App
{
  public:
	App();
	~App();
	void Update();

	std::vector<TrackerID> GetControllers();
	glm::mat4 GetTrackerPose(TrackerID tracker);
	vr::VRControllerState_t GetControllerState(TrackerID controller);
	bool IsGrabActive(TrackerID controller);
	CursorPos GetCursorPosition();

	Display *_xdisplay;
	Window _root_window;
	GLFWwindow *_gl_window;
	GLuint _gl_frame;

	int _root_width;
	int _root_height;

	vr::ETrackingUniverseOrigin _tracking_origin;

	vr::IVRSystem *vr_sys;
	vr::IVROverlay *vr_overlay;

	std::vector<Panel> _panels;

  private:
	void InitX11();
	void InitOVR();
	void InitGLFW();
};