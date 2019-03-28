//
//  main.cpp
//  OpenGL Shadows
//
//  Created by CGIS on 05/12/16.
//  Copyright � 2016 CGIS. All rights reserved.
//

#define GLEW_STATIC

#include <iostream>
#include "glm/glm.hpp"//core glm functionality
#include "glm/gtc/matrix_transform.hpp"//glm extension for generating common transformation matrices
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "GLEW/glew.h"
#include "GLFW/glfw3.h"
#include <string>
#include "Shader.hpp"
#include "Camera.hpp"
#include "SkyBox.hpp"
#include <irrKlang.h>
using namespace irrklang;

ISoundEngine *SoundEngine = createIrrKlangDevice();

#define TINYOBJLOADER_IMPLEMENTATION

#include "Model3D.hpp"
#include "Mesh.hpp"

int glWindowWidth = 1280;
int glWindowHeight = 720;
int retina_width, retina_height;
GLFWwindow* glWindow = NULL;
const double PI = 3.14;
const GLuint SHADOW_WIDTH = 8192, SHADOW_HEIGHT = 8192;

glm::mat4 model;
GLuint modelLoc;
glm::mat4 view;
GLuint viewLoc;
glm::mat4 projection;
GLuint projectionLoc;
glm::mat3 normalMatrix;
GLuint normalMatrixLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::vec3 VentilatorPos = glm::vec3(1.415, 4.61, -22.55);
glm::vec3 lightDir;
GLuint lightDirLoc;
glm::vec3 lightColor;
GLuint lightColorLoc;
glm::vec3 lightDir2;
GLuint lightDirLoc2;

gps::Camera myCamera(glm::vec3(5.0f, 0.0f, 4.5f), glm::vec3(0.0f, 0.0f, 0.0f));

bool pressedKeys[1024];

GLfloat angle = -135;
GLfloat lightAngle;
GLfloat VentilatorAngle = 0.0;
GLfloat weathervaneAngle = 0.0;
GLfloat tireAngle = 0.0;
GLfloat angleRotation = 0.0f;

///gps::Model3D myModel;
gps::Model3D ground;
gps::Model3D windmillPart1;
gps::Model3D windmillPart2;
gps::Model3D windmillPart3;
gps::Model3D jeep;
gps::Model3D tireFL;
gps::Model3D tireFR;
gps::Model3D tireBL;
gps::Model3D tireBR;

gps::Model3D lightCube;
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;
GLuint textureID;

std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

float fogDensity = 0.0;
GLuint fogLoc;

GLenum viewMode = GL_FILL;
double startTime = 0.0;
double currentTime;
double finishTime = glfwGetTime();

GLfloat cameraSpeed;

glm::vec3 newPosition = glm::vec3(0.0, -1.0, 0.0);
glm::vec3 newPositionTireFL = newPosition + glm::vec3(0.41, 0.3, 1.35);
glm::vec3 newPositionTireFR = newPosition + glm::vec3(-0.58, 0.3, 1.35);
glm::vec3 newPositionTireBL = newPosition + glm::vec3(0.41, 0.3, -0.525);
glm::vec3 newPositionTireBR = newPosition + glm::vec3(-0.58, 0.3, -0.525);

GLfloat rlAngle = 0.0;
GLfloat jeepSpeed = 0.003;
GLfloat wheelSpeed = 2.0;
GLfloat VentilatorSpeed = 1.0;

float lastX, lastY;
bool firstMouse = true;
bool driveMode = false;
bool freeMode = false;
bool ok = true;
bool play = false;
bool unlockCar = true;
bool closeCar = false;
bool stop = false;
GLfloat yaw2 = 0.0f;
GLfloat yaw = -90.0f;
GLfloat pitch = 0.0;

glm::mat3 texture_transform;
int frame = 0;
int cameraMode = 1;
bool startApp = true;
bool night = false;
bool inCar = false;
bool mute = false;
bool shadow = true;

int viewM = 0;

float startCamera = 0.0;
float oldYaw;
float oldPitch;


void jeepCamera() {

	if (!freeMode) {

		glm::vec3 camPosition = newPosition;
		float xoffset = 4 * sin((angleRotation * PI) / 180);
		float yoffset = 4 * cos((angleRotation * PI) / 180);
		camPosition.y += 2.0f;
		camPosition.x -= xoffset;
		camPosition.z -= yoffset;
		printf("%f", yaw2);
		myCamera.setCameraPosition(camPosition);

		if (ok == true) {
			yaw2 = -270.0 - angleRotation;
			startCamera = yaw2;
			yaw = yaw2;
			printf("%f = %f", yaw, yaw2);
			pitch = -10.5f;
			ok = false;
		}
		else
			yaw2 = -270.0f - angleRotation + (yaw - startCamera);
		//yaw = - 270.0f - angleRotation;

		printf("yaw2 %f, angleRotation %f, yaw %f \n", yaw2, angleRotation, yaw);
		myCamera.rotate(yaw2, pitch);
	}
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

void windowResize(GLFWwindow* window, int width, int height)
{
	fprintf(stdout, "window resized to width: %d , and height: %d\n", width, height);
	//TODO
	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	myCustomShader.useShaderProgram();

	//set projection matrix
	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	//send matrix data to shader
	GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	//set Viewport transform
	glViewport(0, 0, retina_width, retina_height);
}

glm::vec3 cam = glm::vec3(0.0, 0.0, 0.0);
bool presentationStart = false;
bool finishedFirst = false;
bool finishedSecond = false;
bool finishedThird = false;
bool finishedLast = false;
GLfloat i = 24.5;
GLfloat j = 25.0;
GLfloat yawAnimation = -141.29;
int ins = 1;

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (startApp == true) {
		SoundEngine->play2D("sounds/wind.mp3", GL_TRUE);
		startApp = false;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			pressedKeys[key] = true;
		else if (action == GLFW_RELEASE)
			pressedKeys[key] = false;
	}

	if (key == GLFW_KEY_C && action == GLFW_PRESS && driveMode == false) {
		if (cameraMode == 1)
			cameraMode = 2;
		else
			cameraMode = 1;
		myCamera.changeMode(cameraMode);
	}
	if (key == GLFW_KEY_B && action == GLFW_PRESS) {
		viewM = (viewM + 1) % 3;
		if (viewM == 0)
			viewMode = GL_FILL;
		else
			if (viewM == 1)
				viewMode = GL_LINE;
			else
				if (viewM == 2)
					viewMode = GL_POINT;
		
		glPolygonMode(GL_FRONT_AND_BACK, viewMode);
	}
	if (key == GLFW_KEY_M && action == GLFW_PRESS) {
		if (mute == false) {
			SoundEngine->stopAllSounds();
			mute = true;
		}
		else {
			SoundEngine->play2D("sounds/wind.mp3", GL_TRUE);
			mute = false;
		}
	}
	if (driveMode && key == GLFW_KEY_C && action == GLFW_PRESS) {
		if (freeMode == false) {
			freeMode = true;
		}
		else {
			freeMode = false;
			jeepCamera();
		}
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		if (presentationStart == true)
			presentationStart = false;
		else
			presentationStart = true;
		if (presentationStart == true) {
			cam = myCamera.getCameraPosition();
			i = 24.5;
			j = 25.0;
			oldPitch = pitch;
			oldYaw = yaw;
			finishedFirst = false;
			finishedSecond = false;
			finishedThird = false;
			finishedLast = false;
			yawAnimation = -141.29;
			pitch = 3.7;
		}
		else
		{
			myCamera.setCameraPosition(cam);
			myCamera.rotate(oldYaw, oldPitch);
		}
	}
	if (key == GLFW_KEY_N && action == GLFW_PRESS) {
		if (night == false) {
			night = true;
			lightDir = glm::vec3(0.0, -14.0, 0.0);
		}
		else {
			night = false;
			lightDir = glm::vec3(22.0, 10.0, -5.0);
		}
	}
	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		printf("cm x: %f, cr x: %f, cm z: %f, cr z: %f\n", myCamera.getCameraPosition().x, newPosition.x, myCamera.getCameraPosition().z, newPosition.z);
		if (inCar == false && (newPosition.x - 3 <= myCamera.getCameraPosition().x && myCamera.getCameraPosition().x <= newPosition.x + 3) && (newPosition.z - 3 <= myCamera.getCameraPosition().z && myCamera.getCameraPosition().z <= newPosition.z + 3)) {
			inCar = true;
			closeCar = true;
			if (unlockCar == true && mute == false) {
				unlockCar = false;
				SoundEngine->play2D("sounds/DoorClose.mp3", GL_FALSE);
			}
			driveMode = true;
			jeepCamera();
			ok = true;
			freeMode = false;
		}
		else
			if (inCar == true && driveMode == true)
			{
				inCar = false;
				driveMode = false;
				unlockCar = true;
				if (closeCar == true && mute == false) {
					closeCar = false;
					SoundEngine->play2D("sounds/DoorClose.mp3", GL_FALSE);
				}
				float newAngle = (angleRotation * PI) / 180;
				myCamera.setCameraPosition(newPosition + glm::vec3(1.5 * sin(newAngle - glm::radians(290.68)), 1.0, 1.5 * cos(newAngle - glm::radians(290.68))));
			}
	}
	if (key == GLFW_KEY_G && action == GLFW_PRESS) {
		if (shadow == true) {
			shadow = false;
			lightDir = glm::vec3(150.0, 150.0, -150.0);
		}
		else {
			shadow = true;
			lightDir = glm::vec3(22.0, 10.0, -5.0);
		}
	}
	if (key == GLFW_KEY_F11 && action == GLFW_PRESS) {
		if (glWindowWidth == 1280) {
			glWindowWidth = 1920;
			glWindowHeight = 1080;
			glfwSetWindowSize(glWindow, glWindowWidth, glWindowHeight);
			glfwSetWindowPos(glWindow, 0, 0);
		}
		else{
			glWindowWidth = 1280;
			glWindowHeight = 720;
			glfwSetWindowSize(glWindow, glWindowWidth, glWindowHeight);
			glfwSetWindowPos(glWindow, 0, 27);
		}
	}
	if (key == GLFW_KEY_I && action == GLFW_PRESS) {
		if (ins == 1)
			ins = 6;
		else
			ins = 1;
		projection = glm::perspective(glm::radians(45.0f), (float)retina_width * ins / (float)retina_height, 0.1f, 1000.0f);
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	}
}

void presentationAnimation(){
	if (presentationStart == true) {
		if (finishedFirst == false) {
			i -= 0.3 * 50 * currentTime;
			yawAnimation += 0.5488 * 50 * currentTime;
			if (i <= -26.5)
				finishedFirst = true;
		}
		if (finishedFirst == true && finishedSecond == false) {
			j -= 0.3 * 50 * currentTime;
			yawAnimation += 0.4981 * 50 * currentTime;
			if (j <= -24.5)
				finishedSecond = true;
		}
		if (finishedSecond == true && finishedThird == false) {
			i += 0.3 * 50 * currentTime;
			yawAnimation += 0.5697 * 50 * currentTime;
			if (i >= 24)
				finishedThird = true;
		}
		if (finishedThird == true && finishedLast == false) {
			j += 0.3 * 50 * currentTime;
			yawAnimation += 0.5631 * 50 * currentTime;
			if (j >= 25)
				finishedLast = true;
		}
		if (finishedLast == true) {
			presentationStart = false;
			myCamera.setCameraPosition(cam);
		}
		if (finishedLast == false) {
			myCamera.setCameraPosition(glm::vec3(i, 2.0, j));
			printf("pitch: %f", pitch);
			//printf("x: %f y: %f z: %f \n", myCamera.getCameraTarget().x, myCamera.getCameraTarget().y, myCamera.getCameraTarget().z);
			myCamera.rotate(yawAnimation, 3.7f);
		}
	}
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

		if (startApp == true) {
			SoundEngine->play2D("sounds/wind.mp3", GL_TRUE);
			startApp = false;
		}

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		printf("Yaw: %f \n", yaw);

		myCamera.rotate(yaw, pitch);
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
	if (( 3.17 < vec.x && vec.x < 19.12 && vec.z < -3.71) || 
		(((3.17 < vec.x && vec.x < 5.91) || (17.11 < vec.x && vec.x < 19.12)) && vec.z < -4.30) ||
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
bool forwardCond() {
	if (verifyCond(newPositionTireFR) && verifyCond(newPositionTireFL))
		return true;
	return false;
}

bool backwardCond() {
	if (verifyCond(newPositionTireBR) && verifyCond(newPositionTireBL))
		return true;
	return false;
}

void secondLightDir()
{
	myCustomShader.useShaderProgram();

	fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	glUniform1f(fogLoc, fogDensity);

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");


	lightDir2 = glm::vec3(newPosition.x, 0, newPosition.z);
	lightDirLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir2");
	glUniform3fv(lightDirLoc2, 1, glm::value_ptr(lightDir2));

	lightShader.useShaderProgram();
}

void processMovement()
{
	float newAngle = (angleRotation * PI) / 180;

	if (presentationStart == false) {
		if (pressedKeys[GLFW_KEY_Q]) {
			if (weathervaneAngle >= 180) {
				weathervaneAngle = -180;
				VentilatorPos = glm::vec3(1.415, 4.61, -22.55);
			}
			else {
				weathervaneAngle++;
				if (weathervaneAngle < 90 || weathervaneAngle > 270)
					VentilatorPos += glm::vec3(-0.0001 * 65 * currentTime, 0, 0);
				else
					VentilatorPos += glm::vec3(0.0001 * 65 * currentTime, 0, 0);
			}
		}

		if (pressedKeys[GLFW_KEY_E]) {
			printf("Unghi de rotatie este de: %f \n", weathervaneAngle);
			if (weathervaneAngle <= -180) {
				weathervaneAngle = 180;
				VentilatorPos = glm::vec3(1.415, 4.61, -22.55);
			}
			else {
				weathervaneAngle--;
				if (weathervaneAngle > -90 && weathervaneAngle < 90)
					VentilatorPos += glm::vec3(0.0001 * 65 * currentTime, 0, 0);
				else
					VentilatorPos += glm::vec3(-0.0001 * 65 * currentTime, 0, 0);
			}
		}

		if (pressedKeys[GLFW_KEY_W] && (!driveMode || freeMode)) {
			myCamera.move(gps::MOVE_FORWARD, cameraSpeed, cameraMode);
		}
		if (pressedKeys[GLFW_KEY_S] && (!driveMode || freeMode)) {
			myCamera.move(gps::MOVE_BACKWARD, cameraSpeed, cameraMode);
		}


		if (pressedKeys[GLFW_KEY_A] && (!driveMode || freeMode)) {
			myCamera.move(gps::MOVE_LEFT, cameraSpeed, cameraMode);
		}

		if (pressedKeys[GLFW_KEY_D] && (!driveMode || freeMode)) {
			myCamera.move(gps::MOVE_RIGHT, cameraSpeed, cameraMode);
		}


		if (pressedKeys[GLFW_KEY_J]) {

			lightAngle += 0.3f * 65 * currentTime;
			if (lightAngle > 360.0f)
				lightAngle -= 360.0f;
			glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
			myCustomShader.useShaderProgram();
			glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
		}

		if (pressedKeys[GLFW_KEY_L]) {
			lightAngle -= 0.3f * 65 * currentTime;
			if (lightAngle < 0.0f)
				lightAngle += 360.0f;
			glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
			myCustomShader.useShaderProgram();
			glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
		}



		if (pressedKeys[GLFW_KEY_F]) {
			if (pressedKeys[GLFW_KEY_KP_ADD]) {

				fogDensity += 0.00075 * 65 * currentTime;
				myCustomShader.useShaderProgram();
				fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
				glUniform1f(fogLoc, fogDensity);
				skyboxShader.useShaderProgram();
				fogLoc = glGetUniformLocation(skyboxShader.shaderProgram, "fogDensity");
				glUniform1f(fogLoc, fogDensity);
				if (VentilatorSpeed < 5.0)
					VentilatorSpeed += 0.01 * 65 * currentTime;
			}
			if (pressedKeys[GLFW_KEY_KP_SUBTRACT]) {

				fogDensity -= 0.00075 * 65 * currentTime;
				myCustomShader.useShaderProgram();
				fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
				glUniform1f(fogLoc, fogDensity);
				skyboxShader.useShaderProgram();
				fogLoc = glGetUniformLocation(skyboxShader.shaderProgram, "fogDensity");
				glUniform1f(fogLoc, fogDensity);
				if (VentilatorSpeed > 1.0)
					VentilatorSpeed -= 0.01 * 65 * currentTime;
			}
		}

		if (pressedKeys[GLFW_KEY_UP] && pressedKeys[GLFW_KEY_DOWN]) {
			jeepSpeed = 0.0;
			wheelSpeed = 0.0;
		}
		else
		{
			if (pressedKeys[GLFW_KEY_LEFT] && driveMode) {
				jeepCamera();
				if (pressedKeys[GLFW_KEY_UP] && forwardCond() && stop == false)
				{
					angleRotation += 0.5f * 65 * currentTime;
					if (angleRotation > 360.0f)
						angleRotation -= 360.0f;
				}
				else
					if (pressedKeys[GLFW_KEY_DOWN] && backwardCond() && stop == false)
					{
						angleRotation -= 1.0 * 65 * currentTime;
						if (angleRotation < 0.0f)
							angleRotation += 360.0f;
					}
				if (rlAngle < 30.0)
					rlAngle += 1.0 * 65 * currentTime;
			}
			else
				if (rlAngle > 0)
					rlAngle -= 1.0 * 65 * currentTime;

			if (pressedKeys[GLFW_KEY_RIGHT] && driveMode) {
				jeepCamera();
				if (pressedKeys[GLFW_KEY_UP] && forwardCond() && stop == false)
				{
					angleRotation -= 0.5f * 65 * currentTime;
					if (angleRotation < 0.0f)
						angleRotation += 360.0f;
				}
				else
					if (pressedKeys[GLFW_KEY_DOWN] && backwardCond() && stop == false)
					{
						angleRotation += 1.0 * 65 * currentTime;
						if (angleRotation > 360.0f)
							angleRotation -= 360.0f;
					}
				if (rlAngle > -30.0)
					rlAngle -= 1.0 * 65 * currentTime;
			}
			else
				if (rlAngle < 0)
					rlAngle += 1.0 * 65 * currentTime;


		
			if (pressedKeys[GLFW_KEY_UP] && forwardCond() && driveMode && stop == false) {
				jeepCamera();
				if (tireAngle > 360)
					tireAngle = 2.0;
				else
					tireAngle += wheelSpeed;
				secondLightDir();

				newPosition += glm::vec3(jeepSpeed * sin(newAngle), 0, jeepSpeed * cos(newAngle));
				newPositionTireFL = newPosition + glm::vec3(1.43 * sin(newAngle - glm::radians(344.68)), 0.3, 1.43 * cos(newAngle - glm::radians(344.68)));
				newPositionTireFR = newPosition + glm::vec3(1.45 * sin(newAngle - glm::radians(23.32)), 0.3, 1.45 * cos(newAngle - glm::radians(23.32)));
				newPositionTireBL = newPosition + glm::vec3(0.67 * sin(newAngle - glm::radians(218.14)), 0.3, 0.67 * cos(newAngle - glm::radians(218.14)));
				newPositionTireBR = newPosition + glm::vec3(0.77 * sin(newAngle - glm::radians(133.82)), 0.3, 0.77 * cos(newAngle - glm::radians(132.82)));
			}
			if (pressedKeys[GLFW_KEY_DOWN] && backwardCond() && driveMode && stop == false) {
				jeepCamera();
				if (tireAngle < 0)
					tireAngle = 360;
				else
					tireAngle -= wheelSpeed;

				secondLightDir();

				newPosition -= glm::vec3(jeepSpeed * sin(newAngle), 0, jeepSpeed * cos(newAngle));
				newPositionTireFL = newPosition + glm::vec3(1.43 * sin(newAngle - glm::radians(344.68)), 0.31, 1.43 * cos(newAngle - glm::radians(344.68)));
				newPositionTireFR = newPosition + glm::vec3(1.45 * sin(newAngle - glm::radians(23.32)), 0.31, 1.45 * cos(newAngle - glm::radians(23.32)));
				newPositionTireBL = newPosition + glm::vec3(0.67 * sin(newAngle - glm::radians(218.0)), 0.31, 0.67 * cos(newAngle - glm::radians(218.0)));
				newPositionTireBR = newPosition + glm::vec3(0.77 * sin(newAngle - glm::radians(133.82)), 0.3, 0.77 * cos(newAngle - glm::radians(132.82)));
			}
		}

		if (pressedKeys[GLFW_KEY_Y]) {
			printf("Stanga fata - x: %f, z: %f \n", newPositionTireFL.x, newPositionTireFL.z);
			printf("Dreapta fata - x: %f, z: %f \n", newPositionTireFR.x, newPositionTireFR.z);
			printf("Stanga spate - x: %f, z: %f \n", newPositionTireBL.x, newPositionTireBL.z);
			printf("Dreapta spate - x: %f, z: %f \n \n \n", newPositionTireBR.x, newPositionTireBR.z);

		}

		if (pressedKeys[GLFW_KEY_LEFT_SHIFT]) {
			if (currentTime > 0.009)
				jeepSpeed = 0.18f;
			else
				jeepSpeed = 0.07f;
			printf("%f \n", jeepSpeed);

			wheelSpeed = 8.0 * 65 * currentTime;
		}
		else
		{
			if (currentTime > 0.009)
				jeepSpeed = 0.09f;
			else
				jeepSpeed = 0.02f;

			wheelSpeed = 4.0 * 65 * currentTime;
		}

		if (pressedKeys[GLFW_KEY_SPACE]) {
			jeepSpeed = 0.0;
			wheelSpeed = 0.0;
			stop = true;
		}
		else
			stop = false;
		if (pressedKeys[GLFW_KEY_H] && driveMode && mute == false) {
			SoundEngine->play2D("sounds/Airhorn.mp3", GL_FALSE);
			play = true;
		}
		else
			//SoundEngine->removeSoundSource("sounds/Airhorn-SoundBible.com-975027544.mp3");
			if (play == true)
			{
				SoundEngine->removeSoundSource("sounds/Airhorn.mp3");
				play = false;
			}
	}
}

bool initOpenGLWindow()
{
	if (!glfwInit()) {
		fprintf(stderr, "ERROR: could not start GLFW3\n");
		return false;
	}

	//for Mac OS X
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glWindow = glfwCreateWindow(glWindowWidth, glWindowHeight, "OpenGL Shader Example", NULL, NULL);
	if (!glWindow) {
		fprintf(stderr, "ERROR: could not open window with GLFW3\n");
		glfwTerminate();
		return false;
	}

	glfwSetWindowSizeCallback(glWindow, windowResizeCallback);
	glfwMakeContextCurrent(glWindow);

	glfwWindowHint(GLFW_SAMPLES, 4);

	// start GLEW extension handler
	glewExperimental = GL_TRUE;
	glewInit();

	// get version info
	const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
	const GLubyte* version = glGetString(GL_VERSION); // version as a string
	printf("Renderer: %s\n", renderer);
	printf("OpenGL version supported %s\n", version);

	//for RETINA display
	glfwGetFramebufferSize(glWindow, &retina_width, &retina_height);

	glfwSetKeyCallback(glWindow, keyboardCallback);
	glfwSetCursorPosCallback(glWindow, mouseCallback);
	glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetInputMode(glWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	return true;
}

void initOpenGLState()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glViewport(0, 0, retina_width, retina_height);

	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
						  //glEnable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
	glPolygonMode(GL_FRONT_AND_BACK, viewMode);
}

void initFBOs()
{
	//generate FBO ID
	glGenFramebuffers(1, &shadowMapFBO);

	//create depth texture for FBO
	glGenTextures(1, &depthMapTexture);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	//attach texture to FBO
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

glm::mat4 computeLightSpaceTrMatrix()
{
	const GLfloat near_plane = 1.0f, far_plane = 60.0f;
	glm::mat4 lightProjection = glm::ortho(-60.0f, 60.0f, -60.0f, 60.0f, near_plane, far_plane);
	glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
	glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

	return lightProjection * lightView;
}

void initModels()
{
	windmillPart1 = gps::Model3D("objects/windmill/windmillPart1.obj", "objects/windmill/");
	windmillPart2 = gps::Model3D("objects/windmill/windmillPart2.obj", "objects/windmill/");
	windmillPart3 = gps::Model3D("objects/windmill/windmillPart3.obj", "objects/windmill/");
	jeep = gps::Model3D("objects/jeep/jeep.obj", "objects/jeep/");
	tireFL = gps::Model3D("objects/jeep/tireL.obj", "objects/jeep/");
	tireFR = gps::Model3D("objects/jeep/tireR.obj", "objects/jeep/");
	tireBL = gps::Model3D("objects/jeep/tireL.obj", "objects/jeep/");
	tireBR = gps::Model3D("objects/jeep/tireR.obj", "objects/jeep/");
	ground = gps::Model3D("objects/ground/ground.obj", "objects/ground/");
	lightCube = gps::Model3D("objects/sun/sun.obj", "objects/sun/");
}

void initShaders()
{
	myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
	lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
	depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}


void initUniforms()
{
	myCustomShader.useShaderProgram();

	fogLoc = glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity");
	glUniform1f(fogLoc, fogDensity);

	modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

	viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

	normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

	lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");


	//set the light direction (direction towards the light)
	//lightDir = glm::vec3(0.0f, 0.0f, 0.0f);
	lightDir = glm::vec3(22.0, 10.0, -5.0);
	lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
	glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

	lightDir2 = glm::vec3(newPosition.x, -1.5, newPosition.z);
	lightDirLoc2 = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir2");
	glUniform3fv(lightDirLoc2, 1, glm::value_ptr(lightDir2));

	//set light color
	lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
	lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
	glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
	lightShader.useShaderProgram();
}

void initSky() {
	faces.push_back("textures/skybox/mp_sorbin/sorbin_ft.tga");
	faces.push_back("textures/skybox/mp_sorbin/sorbin_bk.tga");
	faces.push_back("textures/skybox/mp_sorbin/sorbin_up.tga");
	faces.push_back("textures/skybox/mp_sorbin/sorbin_dn.tga");
	faces.push_back("textures/skybox/mp_sorbin/sorbin_rt.tga");
	faces.push_back("textures/skybox/mp_sorbin/sorbin_lf.tga");
}

void renderSky() {
	mySkyBox.Load(faces);
	skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
	skyboxShader.useShaderProgram();
	view = myCamera.getViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void renderScene()
{
	startTime = glfwGetTime();
	currentTime = startTime - finishTime;
	finishTime = startTime;
	presentationAnimation();
	cameraSpeed = 0.15f * 55 * currentTime;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	processMovement();

	//render the scene to the depth buffer (first pass)
	depthMapShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create model matrix for windmill
	model = glm::translate(glm::mat4(1.0f), glm::vec3(15, -1, 17));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	windmillPart1.Draw(depthMapShader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(15, -1, 17));
	model = glm::rotate(model, glm::radians(weathervaneAngle), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));
	windmillPart2.Draw(depthMapShader);

	//create model matrix for Ventilator
	if (VentilatorAngle > 360)
		VentilatorAngle = 0;
	else
		VentilatorAngle += VentilatorSpeed;
	//model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//model = glm::translate(model, glm::vec3(1.415, 4.61, -22.55));
	//model = glm::rotate(model, glm::radians(VentilatorAngle), glm::vec3(0, 0, 1));

	model = glm::translate(glm::mat4(1.0f), glm::vec3(14.955, 4.617, 16.96));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(weathervaneAngle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(VentilatorAngle), glm::vec3(0, 0, 1));

	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	windmillPart3.Draw(depthMapShader);
	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//create model matrix for jeep
	model = glm::translate(glm::mat4(1.0f), newPosition);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	jeep.Draw(depthMapShader);


	model = glm::translate(glm::mat4(1.0f), newPositionTireBL);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	tireBL.Draw(depthMapShader);

	model = glm::translate(glm::mat4(1.0f), newPositionTireBR);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	tireBR.Draw(depthMapShader);

	model = glm::translate(glm::mat4(1.0f), newPositionTireFL);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rlAngle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	tireFL.Draw(depthMapShader);

	model = glm::translate(glm::mat4(1.0f), newPositionTireFR);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rlAngle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	tireFR.Draw(depthMapShader);

	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//send model matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "model"),
		1,
		GL_FALSE,
		glm::value_ptr(model));

	ground.Draw(depthMapShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//render the scene (second pass)

	myCustomShader.useShaderProgram();

	//send lightSpace matrix to shader
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"),
		1,
		GL_FALSE,
		glm::value_ptr(computeLightSpaceTrMatrix()));

	//send view matrix to shader
	view = myCamera.getViewMatrix();;
	glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"),
		1,
		GL_FALSE,
		glm::value_ptr(view));

	//compute light direction transformation matrix
	lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
	//send lightDir matrix data to shader
	glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

	glViewport(0, 0, retina_width, retina_height);
	myCustomShader.useShaderProgram();

	//bind the depth map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, depthMapTexture);
	glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);
	/// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//create model matrix for windmill
	model = glm::translate(glm::mat4(1.0f), glm::vec3(15, -1, 17));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	windmillPart1.Draw(myCustomShader);

	model = glm::translate(glm::mat4(1.0f), glm::vec3(15, -1, 17));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(weathervaneAngle), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
	windmillPart2.Draw(myCustomShader);

	//create model matrix for Ventilator

	//model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
	//model = glm::translate(model, VentilatorPos);
	//model = glm::rotate(model, glm::radians(weathervaneAngle), glm::vec3(0, 1, 0));
	//model = glm::rotate(model, glm::radians(VentilatorAngle), glm::vec3(0, 0, 1));

	model = glm::translate(glm::mat4(1.0f), glm::vec3(14.955, 4.617, 16.96));
	model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(weathervaneAngle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(VentilatorAngle), glm::vec3(0, 0, 1));

	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	windmillPart3.Draw(myCustomShader);

	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/// //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	//create model matrix for jeep
	model = glm::translate(glm::mat4(1.0f), newPosition);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));

	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	jeep.Draw(myCustomShader);

	//create model matrix for jeep
	model = glm::translate(glm::mat4(1.0f), newPositionTireFL);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rlAngle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	tireFL.Draw(myCustomShader);

	//create model matrix for jeep
	model = glm::translate(glm::mat4(1.0f), newPositionTireFR);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(rlAngle), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	tireFR.Draw(myCustomShader);


	//create model matrix for jeep
	model = glm::translate(glm::mat4(1.0f), newPositionTireBL);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	tireBL.Draw(myCustomShader);



	//create model matrix for jeep
	model = glm::translate(glm::mat4(1.0f), newPositionTireBR);
	model = glm::rotate(model, glm::radians(angleRotation), glm::vec3(0, 1, 0));
	model = glm::rotate(model, glm::radians(tireAngle), glm::vec3(1, 0, 0));
	//send model matrix data to shader	
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//compute normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	tireBR.Draw(myCustomShader);



	//create model matrix for ground
	model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	//send model matrix data to shader
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	//create normal matrix
	normalMatrix = glm::mat3(glm::inverseTranspose(view*model));
	//send normal matrix data to shader
	glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

	ground.Draw(myCustomShader);

	//draw a white cube around the light

	lightShader.useShaderProgram();

	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

	model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::translate(model, lightDir);
	model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
	glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

	lightCube.Draw(lightShader);

	mySkyBox.Draw(skyboxShader, view, projection);

}

int main(int argc, const char * argv[]) {

	initOpenGLWindow();
	initOpenGLState();
	initFBOs();
	initModels();
	initShaders();
	initUniforms();
	initSky();
	renderSky();
	glCheckError();
	while (!glfwWindowShouldClose(glWindow)) {
		renderScene();

		glfwPollEvents();
		glfwSwapBuffers(glWindow);
	}

	//close GL context and any other GLFW resources
	glfwTerminate();
}
