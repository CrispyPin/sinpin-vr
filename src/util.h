#pragma once

#include "../lib/openvr.h"
#include <glm/glm.hpp>

typedef vr::TrackedDeviceIndex_t TrackerID;
typedef vr::VROverlayHandle_t OverlayID;
typedef vr::HmdMatrix34_t VRMat;

const VRMat VRMatIdentity{{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}}};
const int MAX_TRACKERS = vr::k_unMaxTrackedDeviceCount;

class Overlay;
class Panel;
struct Ray
{
	Overlay *overlay;
	float distance;
	glm::vec3 local_pos;
	Panel *hit_panel;
};

struct Color
{
	float r;
	float g;
	float b;
};

inline void PrintVec(glm::vec3 v)
{
	printf("(%.2f, %.2f, %.2f)\n", v.x, v.y, v.z);
}

inline void PrintMat(VRMat m)
{
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]);
}

inline void PrintMat(glm::mat4x4 m)
{
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[0][0], m[0][1], m[0][2], m[0][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[1][0], m[1][1], m[1][2], m[1][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[2][0], m[2][1], m[2][2], m[2][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[3][0], m[3][1], m[3][2], m[3][3]);
}

inline glm::mat4x4 ConvertMat(VRMat mat)
{
	auto m = mat.m;
	return glm::mat4x4(
		m[0][0], m[1][0], m[2][0], 0,
		m[0][1], m[1][1], m[2][1], 0,
		m[0][2], m[1][2], m[2][2], 0,
		m[0][3], m[1][3], m[2][3], 1);
}

inline VRMat ConvertMat(glm::mat4x4 m)
{
	// clang-format off
	return VRMat{{
		{m[0][0], m[1][0], m[2][0], m[3][0]},
		{m[0][1], m[1][1], m[2][1], m[3][1]},
		{m[0][2], m[1][2], m[2][2], m[3][2]}
	}};
	// clang-format on
}

inline glm::vec3 GetPos(glm::mat4x4 mat)
{
	return glm::vec3(mat[3][0], mat[3][1], mat[3][2]);
}

inline glm::vec3 GetPos(VRMat mat)
{
	return glm::vec3(mat.m[0][3], mat.m[1][3], mat.m[2][3]);
}
