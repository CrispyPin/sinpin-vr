#include "panel.h"

Panel::Panel(App *app, vr::HmdMatrix34_t start_pose, int index, int x, int y, int width, int height)
	: _app(app),
	  _index(index),
	  _x(x),
	  _y(y),
	  _width(width),
	  _height(height),
	  _grab_component(app)
{
	_name = "screen_view_" + std::to_string(index);
	_alpha = 1.0f;
	_meters = 1.0f;

	glGenTextures(1, &_gl_texture);
	glBindTexture(GL_TEXTURE_2D, _gl_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(
		GL_TEXTURE_2D, 0, GL_RGB,
		_width, _height, 0,
		GL_BGRA, GL_UNSIGNED_BYTE, 0);

	_texture.eColorSpace = vr::ColorSpace_Auto;
	_texture.eType = vr::TextureType_OpenGL;
	_texture.handle = (void *)(uintptr_t)_gl_texture;

	// create overlay
	{
		auto overlay_create_err = _app->vr_overlay->CreateOverlay(_name.c_str(), _name.c_str(), &_id);
		assert(overlay_create_err == 0);
		_app->vr_overlay->ShowOverlay(_id);
		uint8_t col[4] = {20, 50, 50, 255};
		_app->vr_overlay->SetOverlayRaw(_id, &col, 1, 1, 4);
		printf("Created overlay instance %d\n", _index);

		// (flipping uv on y axis because opengl and xorg are opposite)
		vr::VRTextureBounds_t bounds{0, 1, 1, 0};
		_app->vr_overlay->SetOverlayTextureBounds(_id, &bounds);
		_app->vr_overlay->SetOverlayTransformAbsolute(_id, _app->_tracking_origin, &start_pose);
	}
}

void Panel::Update()
{
	Render();
	UpdateCursor();

	_grab_component.Update(_id, &_meters);
}

void Panel::Render()
{
	glCopyImageSubData(
		_app->_gl_frame, GL_TEXTURE_2D, 0,
		_x, _y, 0,
		_gl_texture, GL_TEXTURE_2D, 0,
		0, 0, 0,
		_width, _height, 1);

	auto set_texture_err = _app->vr_overlay->SetOverlayTexture(_id, &_texture);
	assert(set_texture_err == 0);
}

void Panel::SetHidden(bool state)
{
	_hidden = state;
	if (state)
		_app->vr_overlay->HideOverlay(_id);
	else
		_app->vr_overlay->ShowOverlay(_id);
}

void Panel::UpdateCursor()
{
	auto global_pos = _app->GetCursorPosition();
	if (global_pos.x < _x || global_pos.x >= _x + _width || global_pos.y < _y || global_pos.y >= _y + _height)
	{
		_app->vr_overlay->ClearOverlayCursorPositionOverride(_id);
		return;
	}
	int local_x = global_pos.x - _x;
	int local_y = global_pos.y - _y;

	// TODO: make this work when aspect ratio is >1 (root window is taller than it is wide)
	float ratio = (float)_height / (float)_width;
	float top_edge = 0.5f - ratio / 2.0f;
	float x = local_x / (float)_width;
	float y = 1.0f - (local_y / (float)_width + top_edge);
	auto pos = vr::HmdVector2_t{x, y};
	_app->vr_overlay->SetOverlayCursorPositionOverride(_id, &pos);
}
