#pragma once
#include "../ew/ewMath/mat4.h"
#include "../ew/ewMath/vec3.h"
#include "../bp/transformations.h"

namespace bp {

	struct Camera {
		ew::Vec3 position = ew::Vec3(0, 0, 5); //Camera body position
		ew::Vec3 target = ew::Vec3(0, 0, 0); //Position to look at

		float fov = 60; //Vertical field of view in degrees
		float aspectRatio; //Screen width / Screen height
		float nearPlane = 0.1; //Near plane distance (+Z)
		float farPlane = 100; //Far plane distance (+Z)
		bool orthographic = false; //Perspective or orthographic?
		float orthoSize = 6; //Height of orthographic frustum

		ew::Mat4 ViewMatrix() { //World->View
			return bp::LookAt(position, target, ew::Vec3(0, 1, 0));
		}

		ew::Mat4 ProjectionMatrix() { //View->Clip
			if (orthographic) {
				return bp::Orthographic(orthoSize, aspectRatio, nearPlane, farPlane);
			}
			else {
				return bp::Perspective(fov * bp::CONVERT_TO_RADIANS, aspectRatio, nearPlane, farPlane);
			}
		}
	};

	struct CameraControls {
		double prevMouseX, prevMouseY; //Mouse position from previous frame
		float yaw = 0, pitch = 0; //Degrees
		float mouseSensitivity = 0.1f; //How fast to turn with mouse
		bool firstMouse = true; //Flag to store initial mouse position
		float moveSpeed = 5.0f; //How fast to move with arrow keys (M/S)
	};
}