//
//  Camera.cpp
//  Lab5
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#include "Camera.hpp"
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"


namespace gps {

	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float SPEED = 125.5;
	const float SENSITIVITY = 0.1f;
	const float ZOOM = 45.0f;
	// Euler Angles
	float Yaw = YAW;
	float Pitch = PITCH;
	// Camera options
	float MovementSpeed = 125.5;
	float MouseSensitivity = 0.1f;
	float Zoom = 45.0f;
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}

	void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		printf("pitch %f", Pitch);

		// Update Front, Right and Up Vectors using the updated Euler angles
		rotate(Yaw, Pitch);
	}

	void Camera::ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

	Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget)
	{
		this->cameraPosition = cameraPosition;
		this->cameraTarget = cameraTarget;
		this->cameraDirection = glm::normalize(cameraTarget - cameraPosition);
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
	}

	glm::mat4 Camera::getViewMatrix()
	{
		return glm::lookAt(cameraPosition, cameraPosition + cameraTarget, up);
		//return glm::lookAt(cameraPosition, cameraPosition + cameraDirection , glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::getCameraTarget()
	{
		return cameraTarget;
	}

	void Camera::setCameraTarget(glm::vec3 cameraTarget) {
		this->cameraTarget = cameraTarget;
	}


	glm::vec3 Camera::getCameraDirection()
	{
		return cameraDirection;
	}

	void Camera::setCameraDirection(glm::vec3 cameraDirection) {
		this->cameraDirection = cameraDirection;
	}

	bool verifyCond(glm::vec3 vec) {
		if (vec.z > 25 || vec.x < -26.1 || vec.z < -24.4 || vec.x > 23.5)
			return false;
		if (vec.x < -9.7 && vec.z < 2.9 && vec.z > -12.7)
			return false;
		if ((vec.x < -16.8 &&  7.9 < vec.z && vec.z < 13.7) ||
			(-16.8 < vec.x && vec.x < -9.09 && 7.83 < vec.z && vec.z < 11.7) ||
			(-9.09 < vec.x && vec.x < -4.97 && 5.1 < vec.z && vec.z < 10.65))
			return false;
		if ((3.17 < vec.x && vec.x < 19.12 && vec.z < -3.71) ||
			(((3.17 < vec.x && vec.x < 5.91) || (17.11 < vec.x && vec.x < 19.12)) && vec.z < -4.31) ||
			(((5.91 < vec.x && vec.x < 6.4) || (16.16 < vec.x && vec.x < 17.11)) && vec.z < -3.78) ||
			(((6.4 < vec.x && vec.x < 7.96) || (14.39 < vec.x && vec.x < 16.16)) && vec.z < -2.34) ||
			(7.96 < vec.x && vec.x < 14.39 && vec.z < -1.38) ||
			(19.12 < vec.x && vec.x < 20.07 && vec.z < -5.81) ||
			(20.07 < vec.x && vec.z < -7.94))
			return false;
		if ((7.15 < vec.x && vec.x < 9.72 && 21.42 < vec.z) ||
			(9.72 < vec.x && vec.x < 20.42 && 23.04 < vec.z) ||
			(20.42 < vec.x && 15.64 < vec.z))
			return false;
		if (11.96 < vec.x && vec.x < 15.06 && 17.12 < vec.z && vec.z < 20.41)
			return false;
		if (14.02 < vec.x && vec.x < 16.22 && 15.81 < vec.z && vec.z < 18.26)
			return false;
		return true;
	}

	void Camera::move(MOVE_DIRECTION direction, float speed, int cameraMode)
	{
		glm::vec3 oldCameraPosition = cameraPosition;
		//printf("Stanga fata - x: %f, z: %f \n", cameraPosition.x, cameraPosition.z);
		
		switch (direction) {
		case MOVE_FORWARD:
			if (cameraMode == 1)
				cameraPosition += glm::vec3(cameraTarget.x, 0, cameraTarget.z) * speed;
			else
				cameraPosition += cameraTarget * speed;
			if (!verifyCond(cameraPosition) && cameraMode == 1)
				cameraPosition = oldCameraPosition;
			break;

		case MOVE_BACKWARD:
			if (cameraMode == 1)
				cameraPosition -= glm::vec3(cameraTarget.x, 0, cameraTarget.z) * speed;
			else
				cameraPosition -= cameraTarget * speed;
			if (!verifyCond(cameraPosition) && cameraMode == 1)
				cameraPosition = oldCameraPosition;
			break;

		case MOVE_RIGHT:
			cameraPosition += cameraRightDirection * speed;
			if (!verifyCond(cameraPosition) && cameraMode == 1)
				cameraPosition = oldCameraPosition;
			break;

		case MOVE_LEFT:
			cameraPosition -= cameraRightDirection * speed;
			if (!verifyCond(cameraPosition) && cameraMode == 1)
				cameraPosition = oldCameraPosition;
			break;

		}
	}

	void Camera::changeMode(int cameraMode) {
		if (cameraMode == 1) {
			cameraPosition.y = 0;
			if (!verifyCond(cameraPosition)) {
				cameraPosition.x = 0;
				cameraPosition.z = 23;
			}
		}
	}

	void Camera::rotate(float yaw, float pitch) {
		glm::vec3 dir;
		dir.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		dir.y = sin(glm::radians(pitch));
		dir.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		//printf("%f %f %f\n", dir.x, dir.y, dir.z);
		this->cameraTarget = glm::normalize(dir);
		//this->cameraTarget.y = 0.1;
		this->cameraRightDirection = glm::normalize(glm::cross(this->cameraTarget, glm::vec3(0.0f, 1.0f, 0.0f)));
		//this->cameraRightDirection.y = 0;
		this->up = glm::normalize(glm::cross(cameraRightDirection, cameraTarget));
		glm::lookAt(cameraTarget, glm::vec3(0.0, 0.0, 0.0), up);
	}

	void Camera::setCameraPosition(glm::vec3 cameraPosition) {
		this->cameraPosition = cameraPosition;
	}

	glm::vec3 Camera::getCameraPosition() {
		return this->cameraPosition;
	}
}
