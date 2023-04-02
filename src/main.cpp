#include <signal.h>
#include <assert.h>

#include <X11/Xutil.h>
#include <GLFW/glfw3.h>

#include "openvr/openvr.h"

auto TRACKING_UNIVERSE = vr::ETrackingUniverseOrigin::TrackingUniverseStanding;

#define FRAMERATE 30

uint16_t width;
uint16_t height;
bool should_exit = false;

Display *xdisplay;
Window root_window;

vr::IVRSystem *ovr_sys;
vr::IVROverlay *ovr_overlay;
vr::VROverlayHandle_t main_overlay;
vr::Texture_t vr_texture;

GLuint screen_texture;
GLFWwindow *gl_window;

void init_x11()
{
	xdisplay = XOpenDisplay(nullptr);
	assert(xdisplay != nullptr);
	printf("Created X11 display\n");
	root_window = XRootWindow(xdisplay, 0);
	XWindowAttributes attributes;
	XGetWindowAttributes(xdisplay, root_window, &attributes);
	width = attributes.width;
	height = attributes.height;
}

void init_glfw()
{
	assert(glfwInit() == true);
	glfwWindowHint(GLFW_VISIBLE, false);
	gl_window = glfwCreateWindow(width, height, "Overlay", nullptr, nullptr);
	assert(gl_window != nullptr);
	glfwMakeContextCurrent(gl_window);
	printf("Created GLFW context\n");

	glGenTextures(1, &screen_texture);
	glBindTexture(GL_TEXTURE_2D, screen_texture);

	vr_texture.eColorSpace = vr::EColorSpace::ColorSpace_Auto;
	vr_texture.eType = vr::ETextureType::TextureType_OpenGL;
	vr_texture.handle = (void *)(uintptr_t)screen_texture;
}

void init_vr()
{
	vr::EVRInitError init_err;
	ovr_sys = vr::VR_Init(&init_err, vr::EVRApplicationType::VRApplication_Overlay);
	assert(init_err == 0);
	ovr_overlay = vr::VROverlay();
	printf("Initialized openvr overlay\n");
}

void init_overlay()
{
	auto overlay_err = ovr_overlay->CreateOverlay("deskpot", "Desktop view", &main_overlay);
	assert(overlay_err == 0);
	ovr_overlay->ShowOverlay(main_overlay);
	ovr_overlay->SetOverlayWidthInMeters(main_overlay, 2.5f);
	uint8_t col[4] = {20, 50, 50, 255};
	ovr_overlay->SetOverlayRaw(main_overlay, &col, 1, 1, 4);
	printf("Created overlay instance\n");

	vr::HmdMatrix34_t transform;
	auto err = ovr_overlay->GetOverlayTransformAbsolute(main_overlay, &TRACKING_UNIVERSE, &transform);
	assert(err == 0);
	transform.m[1][1] = -1; // flip Y axis
	err = ovr_overlay->SetOverlayTransformAbsolute(main_overlay, TRACKING_UNIVERSE, &transform);
	assert(err == 0);
}

void render_desktop()
{
	auto frame = XGetImage(xdisplay, root_window, 0, 0, width, height, AllPlanes, ZPixmap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame->data);
	XDestroyImage(frame);

	auto set_err = ovr_overlay->SetOverlayTexture(main_overlay, &vr_texture);
	// if (set_err)
	// 	printf("error setting texture: %d\n", set_err);
	assert(set_err == 0);
}

void update_cursor()
{
	int pix_x, pix_y;
	{
		Window _t1;
		int _t2;
		unsigned int _t3;
		XQueryPointer(xdisplay, root_window, &_t1, &_t1, &pix_x, &pix_y, &_t2, &_t2, &_t3);
	}
	// TODO: make this work when aspect ratio is >1 (root window is taller than it is wide)
	float ratio = (float)height / (float)width;
	float top_edge = 0.5f - ratio / 2.0f;
	float x = pix_x / (float)width;
	float y = pix_y / (float)width + top_edge;
	auto pos = vr::HmdVector2_t{x, y};
	ovr_overlay->SetOverlayCursorPositionOverride(main_overlay, &pos);
}

void interrupted(int _sig)
{
	should_exit = true;
}

int main()
{
	signal(SIGINT, interrupted);

	init_x11();
	init_glfw();
	init_vr();
	init_overlay();

	while (!should_exit)
	{
		render_desktop();
		update_cursor();

		glfwSwapBuffers(gl_window);
		usleep(1000000 / FRAMERATE);
	}

	printf("\nShutting down\n");
	vr::VR_Shutdown();
	glfwDestroyWindow(gl_window);
	glfwTerminate();
	return 0;
}
