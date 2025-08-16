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
#include "gfx/Model.hpp"
#include "io/ImageLoader.hpp"
#include "Shader.hpp"

const glm::mat4 identityMatrix = glm::mat4(1.0f);
extern bool flashlightOn;

int main(void)
{    GLFWwindow* window;
    int initSuccess = 1;

    if (InitializeOpenGL(window) != initSuccess)
    {
        return -1;
    }

    stbi_set_flip_vertically_on_load(true);

    unsigned int boxVAO, lightVAO;
    
    const char* backpackVertexShaderPath = "../shaders/backpack_vert.shad";
    const char* backpackFragmentShaderPath = "../shaders/backpack_frag.shad";

    const char* lightVertexShaderPath = "../shaders/3.3.light_vert.shad";
    const char* lightFragmentShaderPath = "../shaders/3.3.light_frag.shad";

    Shader backpackShader(backpackVertexShaderPath, backpackFragmentShaderPath);
    Shader lightShader(lightVertexShaderPath, lightFragmentShaderPath);
        
    const char* backpackModelPath = "../assets/models/backpack/backpack.obj";
    Model ourModel(backpackModelPath);

    int numOfVerticesInBox = 36;
    int amountOfLightPoints = 2;

    glm::vec3 dirLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 spotLightColor = glm::vec3(1.0f, 1.0f, 1.0f);

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
        
        backpackShader.UseProgram();

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); 
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        backpackShader.SetUniformMat4("model", model);
        backpackShader.SetUniformMat4("view", mainCamera->GetViewMatrix());
        backpackShader.SetUniformMat4("projection", projectionMatrix);
        backpackShader.SetUniformVec3("cameraPos", mainCamera->Position);

        backpackShader.SetUniformVec3("dirLight[0].position", dirLightPosition);
        backpackShader.SetUniformVec3("dirLight[0].ambient", dirLightColor * 0.1f);
        backpackShader.SetUniformVec3("dirLight[0].diffuse", dirLightColor);
        backpackShader.SetUniformVec3("dirLight[0].specular", dirLightColor);

        backpackShader.SetUniformVec3("pointLight[0].position", lightPointPositions[0]);
        backpackShader.SetUniformVec3("pointLight[0].ambient", lightPointColors[0] * 0.1f);
        backpackShader.SetUniformVec3("pointLight[0].diffuse", lightPointColors[0]);
        backpackShader.SetUniformVec3("pointLight[0].specular", lightPointColors[0]);
        backpackShader.SetUniformFloat("pointLight[0].constant", 1.0f);
        backpackShader.SetUniformFloat("pointLight[0].linear", 0.05f);
        backpackShader.SetUniformFloat("pointLight[0].quadratic",0.02f);

        backpackShader.SetUniformVec3("pointLight[1].position", lightPointPositions[1]);
        backpackShader.SetUniformVec3("pointLight[1].ambient", lightPointColors[1] * 0.1f);
        backpackShader.SetUniformVec3("pointLight[1].diffuse", lightPointColors[1]);
        backpackShader.SetUniformVec3("pointLight[1].specular", lightPointColors[1]);
        backpackShader.SetUniformFloat("pointLight[1].constant", 1.0f);
        backpackShader.SetUniformFloat("pointLight[1].linear", 0.05f);
        backpackShader.SetUniformFloat("pointLight[1].quadratic",0.02f);
        
        backpackShader.SetUniformVec3("spotLight[0].position", mainCamera->Position);
        backpackShader.SetUniformVec3("spotLight[0].direction", mainCamera->Front);
        backpackShader.SetUniformFloat("spotLight[0].cutOff", glm::cos(glm::radians(10.0f)));
        backpackShader.SetUniformFloat("spotLight[0].outerCutOff", glm::cos(glm::radians(12.0f)));
        backpackShader.SetUniformBool("spotLight[0].on", flashlightOn);
        backpackShader.SetUniformVec3("spotLight[0].ambient", spotLightColor * 0.1f);
        backpackShader.SetUniformVec3("spotLight[0].diffuse", spotLightColor);
        backpackShader.SetUniformVec3("spotLight[0].specular", spotLightColor);
        backpackShader.SetUniformFloat("spotLight[0].constant", 1.0f);
        backpackShader.SetUniformFloat("spotLight[0].linear", 0.09f);
        backpackShader.SetUniformFloat("spotLight[0].quadratic",0.032f);


        ourModel.Draw(backpackShader);

        glfwSwapBuffers(window);
        ProcessInput(window, mainCamera.get(), deltaTime);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}