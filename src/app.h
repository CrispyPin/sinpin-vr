#pragma once
#define GL_GLEXT_PROTOTYPES

#include "overlay.h"
#include "panel.h"
#include "util.h"
#include <GLFW/glfw3.h>
#include <X11/Xutil.h>
#include <filesystem>
#include <vector>

struct CursorPos
{
	int x, y;
};

struct InputHandles
{
	vr::VRActionSetHandle_t set;
	vr::VRActionHandle_t toggle;
	vr::VRActionHandle_t distance;
	vr::VRActionHandle_t grab;
};

class App
{
  public:
	App();
	~App();
	void Update();

	std::vector<TrackerID> GetControllers();
	glm::mat4 GetTrackerPose(TrackerID tracker);
	vr::InputDigitalActionData_t GetControllerInputDigital(TrackerID controller, vr::VRActionHandle_t action);
	vr::InputAnalogActionData_t GetControllerInputAnalog(TrackerID controller, vr::VRActionHandle_t action);
	bool IsInputJustPressed(TrackerID controller, vr::VRActionHandle_t action);
	CursorPos GetCursorPosition();

	Display *_xdisplay;
	Window _root_window;
	GLFWwindow *_gl_window;
	GLuint _gl_frame;

	int _root_width;
	int _root_height;

	vr::ETrackingUniverseOrigin _tracking_origin;
	std::filesystem::path _actions_path;

	vr::IVRSystem *vr_sys;
	vr::IVROverlay *vr_overlay;
	vr::IVRInput *vr_input;

	InputHandles _input_handles;
	vr::TrackedDevicePose_t _tracker_poses[vr::k_unMaxTrackedDeviceCount];

	Overlay _root_overlay;
	std::vector<Panel> _panels;
	bool _hidden = false;

  private:
	void InitX11();
	void InitOVR();
	void InitGLFW();
	void InitRootOverlay();

	void UpdateFramebuffer();
	void UpdateInput();
};