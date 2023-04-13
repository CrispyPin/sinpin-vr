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
	float Width();
	float Alpha();

	void SetWidth(float meters);
	void SetHidden(bool state);
	void SetAlpha(float alpha);
	void SetTexture(vr::Texture_t *texture);

	void SetTransformTracker(TrackerID tracker, VRMat *transform);
	void SetTransformWorld(VRMat *transform);

	// void SetTargetTracker(TrackerID tracker);
	void SetTargetWorld();

	std::function<void(TrackerID)> _GrabBeginCallback;
	std::function<void(TrackerID)> _GrabEndCallback;

	void ControllerRelease();
	void ControllerGrab(TrackerID controller);

  private:
	glm::mat4x4 GetTransformAbsolute();

	bool _initialized;

	App *_app;
	OverlayID _id;

	std::string _name;
	bool _is_held;
	bool _hidden;
	float _width_m;
	float _alpha;
	TrackerID _active_hand;

	Target _target;
};