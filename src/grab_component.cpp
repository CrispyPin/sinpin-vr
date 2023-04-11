#include "grab_component.h"

GrabComponent::GrabComponent(App *app)
{
	_app = app;
	_is_held = false;
	_active_hand = 0;
}

void GrabComponent::Update(OverlayID id, float *meters)
{
	_id = id;

	if (!_is_held)
	{
		for (auto controller : _app->GetControllers())
		{
			if (_app->IsInputJustPressed(controller, _app->_input_handles.grab))
			{
				vr::HmdMatrix34_t overlay_pose;
				vr::ETrackingUniverseOrigin tracking_universe;
				_app->vr_overlay->GetOverlayTransformAbsolute(id, &tracking_universe, &overlay_pose);

				auto controller_pos = GetPos(_app->GetTrackerPose(controller));

				auto local_pos = glm::inverse(ConvertMat(overlay_pose)) * glm::vec4(controller_pos - GetPos(overlay_pose), 0);

				float grab_area_thickness = 0.3f;
				bool close_enough = glm::abs(local_pos.z) < grab_area_thickness;
				close_enough &= glm::abs(local_pos.x) < *meters / 2.0f;
				close_enough &= glm::abs(local_pos.y) < *meters / 2.0f;

				if (close_enough)
				{
					ControllerGrab(controller);
				}
			}
		}
	}
	else
	{
		if (!_app->GetControllerInputDigital(_active_hand, _app->_input_handles.grab).bState)
		{
			ControllerRelease();
		}
	}
}

bool GrabComponent::IsHeld()
{
	return _is_held;
}

TrackerID GrabComponent::ActiveHand()
{
	return _active_hand;
}

void GrabComponent::ControllerGrab(TrackerID controller)
{
	_is_held = true;
	_active_hand = controller;

	_app->vr_overlay->SetOverlayColor(_id, 0.6f, 0.8f, 0.8f);

	vr::HmdMatrix34_t abs_pose;
	vr::ETrackingUniverseOrigin tracking_universe;
	_app->vr_overlay->GetOverlayTransformAbsolute(_id, &tracking_universe, &abs_pose);

	auto abs_mat = ConvertMat(abs_pose);
	auto controller_mat = _app->GetTrackerPose(controller);

	vr::HmdMatrix34_t relative_pose = ConvertMat(glm::inverse(controller_mat) * (abs_mat));

	_app->vr_overlay->SetOverlayTransformTrackedDeviceRelative(_id, controller, &relative_pose);
}

void GrabComponent::ControllerRelease()
{
	_is_held = false;
	_active_hand = -1;

	_app->vr_overlay->SetOverlayColor(_id, 1.0f, 1.0f, 1.0f);

	vr::HmdMatrix34_t relative_pose;
	_app->vr_overlay->GetOverlayTransformTrackedDeviceRelative(_id, &_active_hand, &relative_pose);
	auto relative_mat = ConvertMat(relative_pose);

	auto controller_mat = _app->GetTrackerPose(_active_hand);

	vr::HmdMatrix34_t new_pose = ConvertMat(controller_mat * relative_mat);

	_app->vr_overlay->SetOverlayTransformAbsolute(_id, _app->_tracking_origin, &new_pose);
}