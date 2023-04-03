#include "../openvr/openvr.h"
#include <glm/glm.hpp>

inline void print_matrix(vr::HmdMatrix34_t m)
{
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m.m[0][0], m.m[0][1], m.m[0][2], m.m[0][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m.m[1][0], m.m[1][1], m.m[1][2], m.m[1][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m.m[2][0], m.m[2][1], m.m[2][2], m.m[2][3]);
}

inline void print_matrix(glm::mat4x4 m)
{
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[0][0], m[0][1], m[0][2], m[0][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[1][0], m[1][1], m[1][2], m[1][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[2][0], m[2][1], m[2][2], m[2][3]);
	printf("[%.2f, %.2f, %.2f, %.2f]\n", m[3][0], m[3][1], m[3][2], m[3][3]);
}

inline glm::mat4x4 convert_mat(vr::HmdMatrix34_t mat)
{
	auto m = mat.m;
	return glm::mat4x4(
		m[0][0], m[1][0], m[2][0], 0,
		m[0][1], m[1][1], m[2][1], 0,
		m[0][2], m[1][2], m[2][2], 0,
		m[0][3], m[1][3], m[2][3], 1);
}

inline vr::HmdMatrix34_t convert_mat(glm::mat4x4 m)
{
	// clang-format off
	return vr::HmdMatrix34_t{{
		{m[0][0], m[1][0], m[2][0], m[3][0]},
		{m[0][1], m[1][1], m[2][1], m[3][1]},
		{m[0][2], m[1][2], m[2][2], m[3][2]}
	}};
	// clang-format on
}