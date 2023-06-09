#include "app.h"
#include "controller.h"
#include "util.h"
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/Xrandr.h>
#include <cassert>
#include <glm/matrix.hpp>

const VRMat root_start_pose = {{{1, 0, 0, 0}, {0, 1, 0, 0.8f}, {0, 0, 1, 0}}}; // 0.8m above origin

const int FRAME_INTERVAL = 4; // number of update loops until the frame buffer is updated
const float TRANSPARENCY = 0.6f;

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

	_pixels_per_meter = 1920;
	_total_width_meters = _root_width / _pixels_per_meter;
	_total_height_meters = _root_height / _pixels_per_meter;

	for (int i = 0; i < monitor_count; i++)
	{
		XRRMonitorInfo mon = monitor_info[i];
		printf("screen %d: pos(%d, %d) %dx%d\n", i, mon.x, mon.y, mon.width, mon.height);

		_panels.push_back(Panel(this, i, mon.x, mon.y, mon.width, mon.height));
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

		auto action_err = vr_input->GetActionHandle("/actions/edit/in/grab", &_input_handles.edit.grab);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/main/in/toggle_visibility", &_input_handles.main.toggle_hidden);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/in/activate_cursor", &_input_handles.cursor.activate);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/main/in/edit_mode", &_input_handles.main.edit_mode);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/main/in/reset", &_input_handles.main.reset);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/edit/in/distance", &_input_handles.edit.distance);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/in/mouse_left", &_input_handles.cursor.mouse_left);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/in/mouse_right", &_input_handles.cursor.mouse_right);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/in/mouse_middle", &_input_handles.cursor.mouse_middle);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/in/scroll", &_input_handles.cursor.scroll);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/in/toggle_transparent", &_input_handles.cursor.toggle_transparent);
		assert(action_err == 0);
		action_err = vr_input->GetActionHandle("/actions/cursor/out/scroll_haptic", &_input_handles.cursor.scroll_haptic);
		assert(action_err == 0);
		action_err = vr_input->GetActionSetHandle("/actions/main", &_input_handles.main_set);
		assert(action_err == 0);
		action_err = vr_input->GetActionSetHandle("/actions/edit", &_input_handles.edit_set);
		assert(action_err == 0);
		action_err = vr_input->GetActionSetHandle("/actions/cursor", &_input_handles.cursor_set);
		assert(action_err == 0);
	}
}

App::~App()
{
	vr::VR_Shutdown();
	glfwDestroyWindow(_gl_window);
	glfwTerminate();
	XCloseDisplay(_xdisplay);
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
	_root_overlay.SetWidth(0.25f);
	_root_overlay.SetTransformWorld(&root_start_pose);
	_root_overlay.SetTextureToColor(110, 30, 190);
	_root_overlay.SetHidden(true);
}

void App::Update(float dtime)
{
	UpdateInput(dtime);
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

void App::UpdateInput(float dtime)
{
	vr::VRActiveActionSet_t active_sets[2];
	active_sets[0].ulActionSet = _input_handles.main_set;
	active_sets[0].ulRestrictedToDevice = 0;
	active_sets[0].nPriority = 10;
	int set_count = 1;
	if (!_hidden)
	{
		set_count = 2;
		active_sets[1].ulRestrictedToDevice = 0;
		active_sets[1].nPriority = 10;
		active_sets[1].ulActionSet = _input_handles.cursor_set;
		if (_edit_mode)
			active_sets[1].ulActionSet = _input_handles.edit_set;
	}
	vr::EVRInputError err = vr_input->UpdateActionState(active_sets, sizeof(vr::VRActiveActionSet_t), set_count);
	if (err)
		printf("Error updating action state: %d\n", err);

	vr_sys->GetDeviceToAbsoluteTrackingPose(_tracking_origin, 0, _tracker_poses, MAX_TRACKERS);

	if (IsInputJustPressed(_input_handles.main.toggle_hidden))
	{
		_hidden = !_hidden;
		for (auto &panel : _panels)
		{
			panel.SetHidden(_hidden);
		}
		UpdateUIVisibility();
	}
	if (IsInputJustPressed(_input_handles.cursor.toggle_transparent))
	{
		_transparent = !_transparent;
		if (_transparent)
		{
			for (auto &panel : _panels)
				panel.GetOverlay()->SetAlpha(TRANSPARENCY);
		}
		else
		{
			for (auto &panel : _panels)
				panel.GetOverlay()->SetAlpha(1);
		}
	}
	if (!_hidden)
	{
		if (IsInputJustPressed(_input_handles.main.reset))
		{
			_root_overlay.SetTransformWorld(&root_start_pose);
			_root_overlay.SetWidth(0.25f);
			for (auto &panel : _panels)
			{
				panel.ResetTransform();
			}
		}
		if (IsInputJustPressed(_input_handles.main.edit_mode))
		{
			_edit_mode = !_edit_mode;
			UpdateUIVisibility();
			if (_edit_mode && _active_cursor.has_value())
			{
				_active_cursor.value()->_cursor_active = false;
				_active_cursor = {};
			}
		}
	}
	_controllers[0]->Update(dtime);
	_controllers[1]->Update(dtime);
}

void App::UpdateUIVisibility()
{
	bool state = _hidden || !_edit_mode;
	_root_overlay.SetHidden(state);
}

void App::UpdateFramebuffer()
{
	if (_frames_since_framebuffer < FRAME_INTERVAL)
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
		auto r_panel = panel.IntersectRay(origin, direction, max_len);
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

void App::SetCursor(int x, int y)
{
	// I don't know what the return value of XWarpPointer means, it seems to be 1 on success.
	XWarpPointer(_xdisplay, _root_window, _root_window, 0, 0, _root_width, _root_height, x, y);
}

void App::SendMouseInput(unsigned int button, bool state)
{
	XTestFakeButtonEvent(_xdisplay, button, state, 0);
}
