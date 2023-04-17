#include "app.h"
#include "controller.h"
#include "util.h"
#include <X11/extensions/Xrandr.h>
#include <cassert>
#include <glm/matrix.hpp>

App::App()
{
	_tracking_origin = vr::TrackingUniverseStanding;
	_frames_since_framebuffer = 999;

	InitOVR();
	InitX11();
	InitGLFW();
	InitRootOverlay();
	printf("\n");
	_controllers[0] = Controller(this, ControllerSide::Left);
	_controllers[1] = Controller(this, ControllerSide::Right);

	glGenTextures(1, &_gl_frame);
	glBindTexture(GL_TEXTURE_2D, _gl_frame);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	int monitor_count;
	XRRMonitorInfo *monitor_info = XRRGetMonitors(_xdisplay, _root_window, 1, &monitor_count);
	printf("found %d monitors:\n", monitor_count);

	float pixels_per_meter = 1920;
	float total_width_meters = _root_width / pixels_per_meter;
	float total_height_meters = _root_height / pixels_per_meter;

	for (int i = 0; i < monitor_count; i++)
	{
		XRRMonitorInfo mon = monitor_info[i];
		printf("screen %d: pos(%d, %d) %dx%d\n", i, mon.x, mon.y, mon.width, mon.height);

		_panels.push_back(Panel(this, i, mon.x, mon.y, mon.width, mon.height));

		float width = mon.width / pixels_per_meter;
		float pos_x = mon.x / pixels_per_meter + width / 2.0f - total_width_meters / 2.0f;
		float height = mon.height / pixels_per_meter;
		float pos_y = 1.2f + mon.y / pixels_per_meter - height / 2.0f + total_height_meters / 2.0f;
		VRMat start_pose = {{{1, 0, 0, pos_x}, {0, 1, 0, pos_y}, {0, 0, 1, 0}}};
		_panels[i].GetOverlay()->SetTransformWorld(&start_pose);
		_panels[i].GetOverlay()->SetWidth(width);
	}

	for (auto &panel : _panels)
	{
		_root_overlay.AddChildOverlay(panel.GetOverlay());
	}

	{ // initialize SteamVR input
		auto exe_path = std::filesystem::read_symlink("/proc/self/exe");
		_actions_path = exe_path.parent_path().string() + "/bindings/action_manifest.json";
		printf("actions path: %s\n", _actions_path.c_str());
		vr_input->SetActionManifestPath(_actions_path.c_str());

		auto action_err = vr_input->GetActionHandle("/actions/main/in/Grab", &_input_handles.grab);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/main/in/ToggleAll", &_input_handles.toggle);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/main/in/Distance", &_input_handles.distance);
		assert(action_err == 0);
		action_err = vr_input->GetActionSetHandle("/actions/main", &_input_handles.set);
		assert(action_err == 0);
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
	vr_input = vr::VRInput();
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

void App::InitRootOverlay()
{
	_root_overlay = Overlay(this, "root");
	_root_overlay.SetAlpha(0.5f);
	// clang-format off
	VRMat root_start_pose = {{
		{0.25f,	0.0f,	0.0f, 0},
		{0.0f,	0.25f,	0.0f, 0.8f},
		{0.0f,	0.0f,	1.0f, 0}
	}};
	// clang-format on
	_root_overlay.SetTransformWorld(&root_start_pose);
	_root_overlay.SetTextureToColor(110, 30, 190);
}

void App::Update()
{
	UpdateInput();
	if (!_hidden)
	{
		_root_overlay.Update();
		UpdateFramebuffer();
		for (auto &panel : _panels)
		{
			panel.Update();
		}
	}
	_frames_since_framebuffer += 1;
}

void App::UpdateInput()
{
	vr::VRActiveActionSet_t main;
	main.ulActionSet = _input_handles.set;
	main.ulRestrictedToDevice = 0;
	main.nPriority = 10;
	vr::EVRInputError err = vr_input->UpdateActionState(&main, sizeof(vr::VRActiveActionSet_t), 1);
	if (err)
	{
		printf("Error: (update action state): %d\n", err);
	}

	vr_sys->GetDeviceToAbsoluteTrackingPose(_tracking_origin, 0, _tracker_poses, MAX_TRACKERS);

	if (IsInputJustPressed(_input_handles.toggle))
	{
		_hidden = !_hidden;
		_root_overlay.SetHidden(_hidden);
		for (auto &panel : _panels)
		{
			panel.SetHidden(_hidden);
		}
		_controllers[0]->SetHidden(_hidden);
		_controllers[1]->SetHidden(_hidden);
	}
	_controllers[0]->Update();
	_controllers[1]->Update();
}

void App::UpdateFramebuffer()
{
	if (_frames_since_framebuffer < 2)
	{
		return;
	}
	_frames_since_framebuffer = 0;
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
	return ConvertMat(_tracker_poses[tracker].mDeviceToAbsoluteTracking);
}

vr::InputDigitalActionData_t App::GetInputDigital(vr::VRActionHandle_t action, vr::VRInputValueHandle_t controller)
{
	vr::InputDigitalActionData_t state;
	vr_input->GetDigitalActionData(action, &state, sizeof(vr::InputDigitalActionData_t), controller);
	return state;
}

vr::InputAnalogActionData_t App::GetInputAnalog(vr::VRActionHandle_t action, vr::VRInputValueHandle_t controller)
{
	vr::InputAnalogActionData_t state;
	vr_input->GetAnalogActionData(action, &state, sizeof(vr::InputAnalogActionData_t), controller);
	return state;
}

bool App::IsInputJustPressed(vr::VRActionHandle_t action, vr::VRInputValueHandle_t controller)
{
	auto data = GetInputDigital(action, controller);
	return data.bState && data.bChanged;
}

Ray App::IntersectRay(glm::vec3 origin, glm::vec3 direction, float max_len)
{
	Ray ray;
	ray.distance = max_len;
	ray.overlay = nullptr;

	auto r_root = _root_overlay.IntersectRay(origin, direction, max_len);
	if (r_root.distance < ray.distance)
	{
		ray = r_root;
	}

	for (auto &panel : _panels)
	{
		auto r_panel = panel.GetOverlay()->IntersectRay(origin, direction, max_len);
		if (r_panel.distance < ray.distance)
		{
			ray = r_panel;
		}
	}
	return ray;
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
