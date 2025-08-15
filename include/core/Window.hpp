#pragma once
struct GLFWwindow;

int InitializeOpenGL(GLFWwindow*& window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);