#pragma once

#include "util.h"
#include <functional>
#include <string>
#include <vector>

class App;
struct Ray;
class Controller;

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
	Controller *ActiveHand();
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
	Target *GetTarget();

	void SetTransformTracker(TrackerID tracker, const VRMat *transform);
	void SetTransformWorld(const VRMat *transform);

	void SetTargetTracker(TrackerID tracker);
	void SetTargetWorld();

	Ray IntersectRay(glm::vec3 origin, glm::vec3 direction, float max_len);

	void ControllerRelease();
	void ControllerGrab(Controller *controller);

	void AddChildOverlay(Overlay *child);
	void RemoveChildOverlay(Overlay *child);

  private:
	bool _initialized;

	App *_app;
	OverlayID _id;

	std::string _name;
	bool _hidden;
	float _width_m;
	float _alpha;
	float _ratio;
	Controller *_holding_controller;

	std::vector<Overlay *> _children;

	Target _target;
};