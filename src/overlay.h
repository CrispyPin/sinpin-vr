#pragma once

#include "util.h"
#include <functional>
#include <string>

class App;

enum class TargetType
{
	World,
	Tracker,
};

struct Target
{
	TargetType type;
	TrackerID id;
	VRMat transform;
};

class Overlay
{
  public:
	Overlay();
	Overlay(App *app, std::string name);
	void Update();

	OverlayID Id();
	bool IsHeld();
	bool IsHidden();
	TrackerID ActiveHand();
	float Alpha();
	float Width();
	float Ratio();

	void SetWidth(float meters);
	void SetHidden(bool state);
	void SetAlpha(float alpha);
	void SetRatio(float ratio);
	void SetTexture(vr::Texture_t *texture);
	void SetTextureToColor(uint8_t r, uint8_t g, uint8_t b);

	glm::mat4x4 GetTransformAbsolute();

	void SetTransformTracker(TrackerID tracker, const VRMat *transform);
	void SetTransformWorld(const VRMat *transform);

	// void SetTargetTracker(TrackerID tracker);
	void SetTargetWorld();

	float IntersectRay(glm::vec3 origin, glm::vec3 direction, float max_len);

	std::function<void(TrackerID)> _GrabBeginCallback;
	std::function<void(TrackerID)> _GrabEndCallback;

	void ControllerRelease();
	void ControllerGrab(TrackerID controller);

  private:
	bool _initialized;

	App *_app;
	OverlayID _id;

	std::string _name;
	bool _is_held;
	bool _hidden;
	float _width_m;
	float _alpha;
	float _ratio;
	TrackerID _active_hand;

	Target _target;
};