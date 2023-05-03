#pragma once
#define GL_GLEXT_PROTOTYPES

#include "controller.h"
#include "overlay.h"
#include "panel.h"
#include "util.h"
#include <GLFW/glfw3.h>
#include <X11/Xutil.h>
#include <filesystem>
#include <optional>
#include <vector>

struct CursorPos
{
	int x, y;
};

struct InputHandles
{
	struct
	{
		vr::VRActionHandle_t toggle_hidden;
		vr::VRActionHandle_t edit_mode;
		vr::VRActionHandle_t reset;
	} main;
	vr::VRActionSetHandle_t main_set;
	struct
	{
		vr::VRActionHandle_t activate;
		vr::VRActionHandle_t mouse_left;
		vr::VRActionHandle_t mouse_right;
		vr::VRActionHandle_t mouse_middle;
		vr::VRActionHandle_t scroll;
		vr::VRActionHandle_t scroll_haptic;
	} cursor;
	vr::VRActionSetHandle_t cursor_set;
	struct
	{
		vr::VRActionHandle_t distance;
		vr::VRActionHandle_t grab;
	} edit;
	vr::VRActionSetHandle_t edit_set;
};

class App
{
  public:
	App();
	~App();
	void Update(float dtime);

	std::vector<TrackerID> GetControllers();
	glm::mat4 GetTrackerPose(TrackerID tracker);
	vr::InputDigitalActionData_t GetInputDigital(vr::VRActionHandle_t action, vr::VRInputValueHandle_t controller = 0);
	vr::InputAnalogActionData_t GetInputAnalog(vr::VRActionHandle_t action, vr::VRInputValueHandle_t controller = 0);
	bool IsInputJustPressed(vr::VRActionHandle_t action, vr::VRInputValueHandle_t controller = 0);
	CursorPos GetCursorPosition();

	Ray IntersectRay(glm::vec3 origin, glm::vec3 direction, float max_len);
	void SetCursor(int x, int y);
	void SendMouseInput(unsigned int button, bool state);

	Display *_xdisplay;
	Window _root_window;
	GLFWwindow *_gl_window;
	GLuint _gl_frame;
	int _frames_since_framebuffer;

	int _root_width;
	int _root_height;
	float _pixels_per_meter;
	float _total_height_meters;
	float _total_width_meters;

	vr::ETrackingUniverseOrigin _tracking_origin;
	std::filesystem::path _actions_path;

	vr::IVRSystem *vr_sys;
	vr::IVROverlay *vr_overlay;
	vr::IVRInput *vr_input;

	InputHandles _input_handles;
	vr::TrackedDevicePose_t _tracker_poses[MAX_TRACKERS];
	std::optional<Controller> _controllers[2];

	Overlay _root_overlay;
	std::vector<Panel> _panels;
	bool _hidden = false;
	bool _edit_mode = false;
	std::optional<Controller *> _active_cursor;

  private:
	void InitX11();
	void InitOVR();
	void InitGLFW();
	void InitRootOverlay();

	void UpdateFramebuffer();
	void UpdateInput(float dtime);
	void UpdateUIVisibility();
};