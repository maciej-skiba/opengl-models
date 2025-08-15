#include "common/gl_includes.hpp"
#include <iostream>
#include <memory>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "vertices.hpp" 
#include "camera.hpp"

#include "app/Config.hpp"
#include "core/Window.hpp"
#include "core/InputCallbacks.hpp"
#include "gfx/Input.hpp"
#include "gfx/MeshUtils.hpp"
#include "io/ImageLoader.hpp"
#include "shader.hpp"

const glm::mat4 identityMatrix = glm::mat4(1.0f);
extern bool flashlightOn;

int main(void)
{
    GLFWwindow* window;
    int initSuccess = 1;

    if (InitializeOpenGL(window) != initSuccess)
    {
        return -1;
    }

    unsigned int boxVAO, lightVAO;
    
    const char* boxVertexShaderPath = "../shaders/3.3.box_vert.shad";
    const char* boxFragmentShaderPath = "../shaders/3.3.box_frag.shad";

    const char* lightVertexShaderPath = "../shaders/3.3.light_vert.shad";
    const char* lightFragmentShaderPath = "../shaders/3.3.light_frag.shad";

    Shader boxShader(boxVertexShaderPath, boxFragmentShaderPath);
    Shader lightShader(lightVertexShaderPath, lightFragmentShaderPath);

    int numOfVerticesInBox = 36;
    int amountOfBoxes = 4;
    int amountOfLightPoints = 2;

    glm::vec3 dirLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 spotLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 boxPositions[] = {
        glm::vec3(1.5f, 0.0f, 1.5f),
        glm::vec3(-0.5f, 0.0f, -0.5f),
        glm::vec3(-1.0f, 0.0f, 2.0f),
        glm::vec3(2.0f, 0.0f, -2.0f),
    };
    glm::vec3 lightPointPositions[] = {
        glm::vec3(5.0f, 0.0f, 0.0f),
        glm::vec3(-4.0f, 0.0f, 0.0f),
    };

    glm::vec3 lightPointColors[] = {
        glm::vec3(1, 0.275, 0.855), //pink
        glm::vec3(0.271, 0.808, 1)  //cyan
    };
    
    glm::vec3 dirLightPosition = glm::vec3(20.0f, 20.0f, 0.0f);

    int lightBufferSize = numOfVerticesInBox * 6;
    CreateLightVao(lightVAO, lightBoxVertices, lightBufferSize);

    int boxBufferSize = numOfVerticesInBox * 8;
    CreateBoxVao(boxVAO, boxVertices, boxBufferSize);

    glEnable(GL_DEPTH_TEST);

    std::unique_ptr<Camera> mainCamera = std::make_unique<Camera>(
        glm::vec3(0.0f, 0.0f,  10.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSetWindowUserPointer(window, mainCamera.get());

    float deltaTime = 0.0f;	// time between current frame and last frame
    float lastFrame = 0.0f;
    float aspectRatio = static_cast<float>(WINDOW_WIDTH) / WINDOW_HEIGHT;
    float nearClippingPlane = 0.1f;
    float farClippingPlane = 100.0f;

    glm::mat4 boxModelMatrix = identityMatrix;
    glm::mat4 lightModelMatrix = identityMatrix;
    
    unsigned int diffuseMap = LoadTexture("../assets/textures/crate.png");
    unsigned int specularMap = LoadTexture("../assets/textures/specularMap.png");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    boxShader.UseProgram();
    boxShader.SetUniformInt("material.diffuse", 0);
    boxShader.SetUniformInt("material.specular", 1);
    boxShader.SetUniformFloat("material.shininess", 32.0f);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(lightVAO);

        glm::mat4 projectionMatrix = 
            glm::perspective(
                glm::radians(mainCamera->Zoom),
                aspectRatio, 
                nearClippingPlane, 
                farClippingPlane);
        
        lightShader.UseProgram();
        
        for (int lightPoint = 0; lightPoint < amountOfLightPoints; lightPoint++)
        {
            lightModelMatrix = glm::translate(identityMatrix, lightPointPositions[lightPoint]);

            lightShader.SetUniformMat4("model", lightModelMatrix);
            lightShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
            lightShader.SetUniformMat4("projection", projectionMatrix);
            lightShader.SetUniformVec3("lightColor", lightPointColors[lightPoint]);

            glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        }

        glBindVertexArray(boxVAO);
        
        boxShader.UseProgram();

        for (int boxIndex = 0; boxIndex < amountOfBoxes; boxIndex++)
        {
            boxModelMatrix = glm::translate(identityMatrix, boxPositions[boxIndex]);
            boxModelMatrix = 
                glm::rotate(
                    boxModelMatrix, 
                    glm::radians(20.0f * boxIndex), 
                    glm::normalize(glm::vec3(0.3f, 0.3f, 0.3f)));
            boxShader.SetUniformMat4("model", boxModelMatrix);
            boxShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
            boxShader.SetUniformMat4("projection", projectionMatrix);
            boxShader.SetUniformVec3("cameraPos", mainCamera->Position);

            boxShader.SetUniformVec3("dirLight[0].position", dirLightPosition);
            boxShader.SetUniformVec3("dirLight[0].ambient", dirLightColor * 0.1f);
            boxShader.SetUniformVec3("dirLight[0].diffuse", dirLightColor);
            boxShader.SetUniformVec3("dirLight[0].specular", dirLightColor);

            boxShader.SetUniformVec3("pointLight[0].position", lightPointPositions[0]);
            boxShader.SetUniformVec3("pointLight[0].ambient", lightPointColors[0] * 0.1f);
            boxShader.SetUniformVec3("pointLight[0].diffuse", lightPointColors[0]);
            boxShader.SetUniformVec3("pointLight[0].specular", lightPointColors[0]);
            boxShader.SetUniformFloat("pointLight[0].constant", 1.0f);
            boxShader.SetUniformFloat("pointLight[0].linear", 0.05f);
            boxShader.SetUniformFloat("pointLight[0].quadratic",0.02f);

            boxShader.SetUniformVec3("pointLight[1].position", lightPointPositions[1]);
            boxShader.SetUniformVec3("pointLight[1].ambient", lightPointColors[1] * 0.1f);
            boxShader.SetUniformVec3("pointLight[1].diffuse", lightPointColors[1]);
            boxShader.SetUniformVec3("pointLight[1].specular", lightPointColors[1]);
            boxShader.SetUniformFloat("pointLight[1].constant", 1.0f);
            boxShader.SetUniformFloat("pointLight[1].linear", 0.05f);
            boxShader.SetUniformFloat("pointLight[1].quadratic",0.02f);

            boxShader.SetUniformVec3("spotLight[0].position", mainCamera->Position);
            boxShader.SetUniformVec3("spotLight[0].direction", mainCamera->Front);
            boxShader.SetUniformFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
            boxShader.SetUniformFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(12.0f)));
            boxShader.SetUniformBool("spotLight[0].on", flashlightOn);
            boxShader.SetUniformVec3("spotLight[0].ambient", spotLightColor * 0.1f);
            boxShader.SetUniformVec3("spotLight[0].diffuse", spotLightColor);
            boxShader.SetUniformVec3("spotLight[0].specular", spotLightColor);
            boxShader.SetUniformFloat("spotLight[0].constant", 1.0f);
            boxShader.SetUniformFloat("spotLight[0].linear", 0.09f);
            boxShader.SetUniformFloat("spotLight[0].quadratic",0.032f);

            glDrawArrays(GL_TRIANGLES, 0, numOfVerticesInBox);
        }

        glfwSwapBuffers(window);
        ProcessInput(window, mainCamera.get(), deltaTime);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}