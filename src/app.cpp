#include "app.h"
#include "panel.h"
#include "util.h"
#include <cassert>

App::App()
{
	_tracking_origin = vr::ETrackingUniverseOrigin::TrackingUniverseStanding;

	InitOVR();
	InitX11();
	InitGLFW();

	_panels.push_back(Panel(this, 0, 0, 0, _root_width, _root_height));
}

App::~App()
{
	vr::VR_Shutdown();
	glfwDestroyWindow(_gl_window);
	glfwTerminate();
}

void App::InitX11()
{
	_xdisplay = XOpenDisplay(nullptr);
	assert(_xdisplay != nullptr);
	printf("Created X11 display\n");
	_root_window = XRootWindow(_xdisplay, 0);
	XWindowAttributes attributes;
	XGetWindowAttributes(_xdisplay, _root_window, &attributes);
	_root_width = attributes.width;
	_root_height = attributes.height;
}

void App::InitOVR()
{
	vr::EVRInitError init_err;
	vr_sys = vr::VR_Init(&init_err, vr::EVRApplicationType::VRApplication_Background);
	if (init_err == vr::EVRInitError::VRInitError_Init_NoServerForBackgroundApp)
	{
		printf("SteamVR is not running\n");
		exit(1);
	}
	else if (init_err != 0)
	{
		printf("Could not initialize OpenVR session. Error code: %d\n", init_err);
		exit(1);
	}
	printf("Initialized OpenVR\n");
	vr_overlay = vr::VROverlay();
}

void App::InitGLFW()
{
	assert(glfwInit() == true);
	glfwWindowHint(GLFW_VISIBLE, false);
	// TODO this is creating a 1x1 window, should it be bigger?
	_gl_window = glfwCreateWindow(1, 1, "Overlay", nullptr, nullptr);
	assert(_gl_window != nullptr);
	glfwMakeContextCurrent(_gl_window);
	printf("Created GLFW context\n");
}

void App::Update()
{
	for (auto &panel : _panels)
	{
		panel.Update();
	}
}

glm::mat4 App::GetTrackerPose(TrackerID tracker)
{
	vr::VRControllerState_t state;
	vr::TrackedDevicePose_t tracked_pose;
	vr_sys->GetControllerStateWithPose(
		_tracking_origin,
		tracker,
		&state,
		sizeof(vr::VRControllerState_t),
		&tracked_pose);
	return ConvertMat(tracked_pose.mDeviceToAbsoluteTracking);
}

bool App::IsGrabActive(vr::TrackedDeviceIndex_t controller)
{
	vr::VRControllerState_t state;
	auto get_state_err = vr_sys->GetControllerState(controller, &state, sizeof(vr::VRControllerState_t));
	if (get_state_err == false)
	{
		printf("Error getting controller state: %d\n", get_state_err);
		return false;
	}
	// printf("got state\n");

	auto trigger_mask = vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_SteamVR_Trigger);
	auto b_mask = vr::ButtonMaskFromId(vr::EVRButtonId::k_EButton_IndexController_B);
	auto mask = trigger_mask | b_mask;
	return (state.ulButtonPressed & mask) == mask;
}

CursorPos App::GetCursorPosition()
{
	Window curr_root;
	Window curr_win;
	CursorPos pos;
	CursorPos pos_local;
	unsigned int buttons;
	XQueryPointer(
		_xdisplay,
		_root_window,
		&curr_root,
		&curr_win,
		&pos.x, &pos.y,
		&pos_local.x, &pos_local.y,
		&buttons);
	return pos;
}
