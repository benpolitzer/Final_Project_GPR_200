#pragma once
#include "../ew/ewMath/mat4.h"
#include "../ew/ewMath/vec3.h"

namespace bp {
	const float CONVERT_TO_RADIANS = 3.14 / 180;

	//Identity matrix
	inline ew::Mat4 Identity() {
		return ew::Mat4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);
	};
	//Scale on x,y,z axes
	inline ew::Mat4 Scale(ew::Vec3 s) {
		return ew::Mat4(
			s.x, 0, 0, 0,
			0, s.y, 0, 0,
			0, 0, s.z, 0,
			0, 0, 0, 1
		);
	};
	//Rotation around X axis (pitch) in radians
	inline ew::Mat4 RotateX(float rad) {
		return ew::Mat4(
			1, 0, 0, 0,
			0, cos(rad), -sin(rad), 0,
			0, sin(rad), cos(rad), 0,
			0, 0, 0, 1
		);
	};
	//Rotation around Y axis (yaw) in radians
	inline ew::Mat4 RotateY(float rad) {
		return ew::Mat4(
			cos(rad), 0, sin(rad), 0,
			0, 1, 0, 0,
			-sin(rad), 0, cos(rad), 0,
			0, 0, 0, 1
		);
	};
	//Rotation around Z axis (roll) in radians
	inline ew::Mat4 RotateZ(float rad) {
		return ew::Mat4(
			cos(rad), -sin(rad), 0, 0,
			sin(rad), cos(rad), 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
		);
	};
	//Translate x,y,z
	inline ew::Mat4 Translate(ew::Vec3 t) {
		return ew::Mat4(
			1, 0, 0, t.x,
			0, 1, 0, t.y,
			0, 0, 1, t.z,
			0, 0, 0, 1
		);
	};

	struct Transform {
		ew::Vec3 position = ew::Vec3(0.0f, 0.0f, 0.0f);
		ew::Vec3 pivotPoint = ew::Vec3(0.0f, 0.0f, 0.0f);
		ew::Vec3 rotation = ew::Vec3(0.0f, 0.0f, 0.0f); //Euler angles (degrees)
		ew::Vec3 scale = ew::Vec3(1.0f, 1.0f, 1.0f);
		ew::Mat4 getModelMatrix() const {
			ew::Mat4 result = Identity();

			return result = result
				* bp::Translate(position)
				* RotateX(rotation.x * CONVERT_TO_RADIANS)
				* RotateY(rotation.y * CONVERT_TO_RADIANS)
				* RotateZ(rotation.z * CONVERT_TO_RADIANS)
				* bp::Scale(scale);
		}
	};

	//Creates a right handed view space
	//eye = eye (camera) position
	//target = position to look at
	//up = up axis, usually(0,1,0)
	inline ew::Mat4 LookAt(ew::Vec3 eye, ew::Vec3 target, ew::Vec3 up) {
		ew::Vec3 zAxis = ew::Normalize(eye - target);
		ew::Vec3 xAxis = ew::Normalize(ew::Cross(up, zAxis));
		ew::Vec3 yAxis = ew::Cross(zAxis, xAxis);

		ew::Mat4 viewMatrix(
			xAxis.x, xAxis.y, xAxis.z, -ew::Dot(xAxis, eye),
			yAxis.x, yAxis.y, yAxis.z, -ew::Dot(yAxis, eye),
			zAxis.x, zAxis.y, zAxis.z, -ew::Dot(zAxis, eye),
			0.0, 0.0, 0.0, 1.0
		);

		return viewMatrix;
	};

	//Orthographic projection
	inline ew::Mat4 Orthographic(float height, float aspect, float near, float far) {
		float top = height / 2.0f;
		float bottom = -top;
		float right = height * aspect / 2.0f;
		float left = -right;

		ew::Mat4 orthographicMatrix(
			2.0f / (right - left), 0, 0, -(right + left) / (right - left),
			0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
			0, 0, -2.0f / (far - near), -(far + near) / (far - near),
			0, 0, 0, 1
		);

		return orthographicMatrix;
	};

	//Perspective projection
	//fov = vertical aspect ratio (radians)
	inline ew::Mat4 Perspective(float fov, float aspect, float near, float far) {
		float tanHalfFOV = tanf(fov / 2.0f);

		ew::Mat4 perspectiveMatrix(
			1.0f / (aspect * tanHalfFOV), 0, 0, 0,
			0, 1.0f / tanHalfFOV, 0, 0,
			0, 0, (near + far) / (near - far), 2.0f * far * near / (near - far),
			0, 0, -1, 0
		);

		return perspectiveMatrix;
	};
}