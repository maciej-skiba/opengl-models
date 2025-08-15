#pragma once
struct GLFWwindow;
#include "camera.hpp"

void ProcessInput(GLFWwindow* window, Camera* camera, float deltaTime);

// była u Ciebie globalna, main.cpp używa extern:
extern bool flashlightOn;