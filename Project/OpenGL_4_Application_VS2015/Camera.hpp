//
//  Camera.hpp
//
//  Created by CGIS on 28/10/2016.
//  Copyright © 2016 CGIS. All rights reserved.
//

#ifndef Camera_hpp
#define Camera_hpp

#include <stdio.h>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

namespace gps {

	enum MOVE_DIRECTION { MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT };

	class Camera
	{
	public:
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		void ProcessMouseScroll(float yoffset);
		Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget);
		glm::mat4 getViewMatrix();
		glm::vec3 getCameraTarget();
		void setCameraTarget(glm::vec3 cameraTarget);
		glm::vec3 getCameraDirection();
		void setCameraDirection(glm::vec3 cameraTarget);
		void move(MOVE_DIRECTION direction, float speed, int cameraMode);
		void changeMode(int cameraMode);
		void rotate(float yaw, float pitch);
		void setCameraPosition(glm::vec3 cameraPosition);
		glm::vec3 getCameraPosition();
	private:
		glm::vec3 cameraPosition;
		glm::vec3 cameraTarget;
		glm::vec3 cameraDirection;
		glm::vec3 cameraRightDirection;
		glm::vec3 cameraVerticalDirection;
		glm::vec3 up;
	};

}

#endif /* Camera_hpp */
