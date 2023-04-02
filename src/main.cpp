#include <signal.h>
#include <assert.h>

#include <X11/extensions/Xcomposite.h>
#include <X11/Xutil.h>
#include <GLFW/glfw3.h>

#include "openvr/openvr.h"

static vr::ETrackingUniverseOrigin TRACKING_UNIVERSE = vr::ETrackingUniverseOrigin::TrackingUniverseStanding;

#define WIDTH 1024
#define HEIGHT 1024

Display *xdisplay;
Window root_window;

vr::IVRSystem *ovr_sys;
vr::IVROverlay *ovr_overlay;
vr::VROverlayHandle_t handle;

GLuint overlaytexture;
GLFWwindow *gl_window;

void cleanup(int _sig = 0);

int main(int argc, char **argv)
{
	signal(SIGINT, cleanup);
	{
		xdisplay = XOpenDisplay(nullptr);
		assert(xdisplay != nullptr);
		printf("Created X11 display\n");
		root_window = XRootWindow(xdisplay, 0);
	}

	{
		assert(glfwInit() == true);
		glfwWindowHint(GLFW_VISIBLE, false);
		gl_window = glfwCreateWindow(WIDTH, HEIGHT, "Overlay", nullptr, nullptr);
		assert(gl_window != nullptr);
		glfwMakeContextCurrent(gl_window);
		printf("Created GLFW context\n");
	}

	{
		vr::EVRInitError init_err;
		ovr_sys = vr::VR_Init(&init_err, vr::EVRApplicationType::VRApplication_Overlay);
		assert(init_err == 0);
		ovr_overlay = vr::VROverlay();
		printf("Initialized openvr overlay\n");
	}

	{
		auto overlay_err = ovr_overlay->CreateOverlay("deskpot", "Desktop view", &handle);
		assert(overlay_err == 0);
		ovr_overlay->ShowOverlay(handle);
		ovr_overlay->SetOverlayTextureColorSpace(handle, vr::EColorSpace::ColorSpace_Gamma);
		ovr_overlay->SetOverlayWidthInMeters(handle, 0.5f);
		uint8_t col[4] = {20, 100, 100, 255};
		ovr_overlay->SetOverlayRaw(handle, &col, 1, 1, 4);
		printf("Created overlay instance\n");
	}

	{
		vr::HmdMatrix34_t transform;
		auto err = ovr_overlay->GetOverlayTransformAbsolute(handle, &TRACKING_UNIVERSE, &transform);
		assert(err == 0);
		transform.m[1][1] = -1;
		err = ovr_overlay->SetOverlayTransformAbsolute(handle, TRACKING_UNIVERSE, &transform);
		assert(err == 0);
	}

	{
		glGenTextures(1, &overlaytexture);
		glBindTexture(GL_TEXTURE_2D, overlaytexture);
	}

	vr::Texture_t vr_texture;
	vr_texture.eColorSpace = vr::EColorSpace::ColorSpace_Auto;
	vr_texture.eType = vr::ETextureType::TextureType_OpenGL;

	while (1)
	{
		auto frame = XGetImage(xdisplay, root_window, 0, 0, WIDTH, HEIGHT, AllPlanes, ZPixmap);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame->data);
		XDestroyImage(frame);

		{
			vr_texture.handle = (void *)(uintptr_t)overlaytexture;
			auto set_err = ovr_overlay->SetOverlayTexture(handle, &vr_texture);
			if (set_err)
			{
				printf("error setting texture: %d\n", set_err);
				assert(set_err == 0);
			}
		}
		glfwSwapBuffers(gl_window);

		ovr_overlay->WaitFrameSync(20);
	}
	cleanup();
	return 0;
}

void cleanup(int _sig)
{
	printf("\nShutting down\n");
	vr::VR_Shutdown();
	glfwDestroyWindow(gl_window);
	glfwTerminate();
	exit(0);
}