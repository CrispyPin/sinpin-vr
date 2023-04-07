#include "app.h"
#include "panel.h"
#include "util.h"
#include <X11/extensions/Xrandr.h>
#include <cassert>

App::App()
{
	_tracking_origin = vr::TrackingUniverseStanding;

	InitOVR();
	InitX11();
	InitGLFW();
	printf("\n");

	glGenTextures(1, &_gl_frame);
	glBindTexture(GL_TEXTURE_2D, _gl_frame);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	int monitor_count;
	XRRMonitorInfo *monitor_info = XRRGetMonitors(_xdisplay, _root_window, 1, &monitor_count);
	printf("found %d monitors:\n", monitor_count);

	float pixels_per_meter = 1920;
	float x_min = -(monitor_info[0].x + monitor_info[0].width / 2.0f);

	for (int i = 0; i < monitor_count; i++)
	{
		XRRMonitorInfo mon = monitor_info[i];
		printf("screen %d: pos(%d, %d) %dx%d\n", i, mon.x, mon.y, mon.width, mon.height);

		float pos_x = (x_min + mon.x) / pixels_per_meter;
		float pos_y = 1.2f;
		vr::HmdMatrix34_t start_pose = {{{1, 0, 0, pos_x}, {0, 1, 0, pos_y}, {0, 0, 1, 0}}};
		_panels.push_back(Panel(this, start_pose, i, mon.x, mon.y, mon.width, mon.height));
	}
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
	// would normally be using VRApplication_Overlay, but Background allows it to quit if steamvr is not running, instead of opening steamvr.
	vr_sys = vr::VR_Init(&init_err, vr::VRApplication_Background);
	if (init_err == vr::VRInitError_Init_NoServerForBackgroundApp)
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
	// creating a 1x1 window, since it is hidden anyway
	_gl_window = glfwCreateWindow(1, 1, "Overlay", nullptr, nullptr);
	assert(_gl_window != nullptr);
	glfwMakeContextCurrent(_gl_window);
	printf("Created GLFW context\n");
}

void App::Update()
{
	auto frame = XGetImage(
		_xdisplay,
		_root_window,
		0, 0,
		_root_width, _root_height,
		AllPlanes,
		ZPixmap);
	glBindTexture(GL_TEXTURE_2D, _gl_frame);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, _root_width, _root_height, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame->data);
	XDestroyImage(frame);

	for (auto &panel : _panels)
	{
		panel.Update();
	}
}

std::vector<TrackerID> App::GetControllers()
{
	static const auto max_len = 64;
	TrackerID controllers[max_len];
	int controller_count = vr_sys->GetSortedTrackedDeviceIndicesOfClass(vr::TrackedDeviceClass_Controller, controllers, max_len);
	return std::vector<TrackerID>(controllers, controllers + controller_count);
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

vr::VRControllerState_t App::GetControllerState(TrackerID controller)
{
	vr::VRControllerState_t state;
	auto get_state_err = vr_sys->GetControllerState(controller, &state, sizeof(vr::VRControllerState_t));
	if (get_state_err == false)
		printf("failed to get state of controller %d\n", controller);
	return state;
}

bool App::IsGrabActive(TrackerID controller)
{
	vr::VRControllerState_t state;
	auto get_state_err = vr_sys->GetControllerState(controller, &state, sizeof(vr::VRControllerState_t));
	if (get_state_err == false)
		return false;

	auto trigger_mask = vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger);
	return state.ulButtonPressed & trigger_mask;
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
