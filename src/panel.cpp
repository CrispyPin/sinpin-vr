#include "panel.h"
#include "app.h"
#include "overlay.h"

Panel::Panel(App *app, int index, int x, int y, int width, int height)
	: _app(app),
	  _index(index),
	  _x(x),
	  _y(y),
	  _width(width),
	  _height(height),
	  _overlay(app, "screen_view_" + std::to_string(index))
{
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
	_overlay.SetRatio(height / (float)width);
	_overlay.SetTextureToColor(50, 20, 50);
}

void Panel::Update()
{
	Render();
	UpdateCursor();

	_overlay.Update();
}

Overlay *Panel::GetOverlay()
{
	return &_overlay;
}

void Panel::Render()
{
	glCopyImageSubData(
		_app->_gl_frame, GL_TEXTURE_2D, 0,
		_x, _y, 0,
		_gl_texture, GL_TEXTURE_2D, 0,
		0, 0, 0,
		_width, _height, 1);

	_overlay.SetTexture(&_texture);
}

void Panel::SetHidden(bool state)
{
	_overlay.SetHidden(state);
}

void Panel::UpdateCursor()
{
	auto global_pos = _app->GetCursorPosition();
	if (global_pos.x < _x || global_pos.x >= _x + _width || global_pos.y < _y || global_pos.y >= _y + _height)
	{
		_app->vr_overlay->ClearOverlayCursorPositionOverride(_overlay.Id());
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
	_app->vr_overlay->SetOverlayCursorPositionOverride(_overlay.Id(), &pos);
}
