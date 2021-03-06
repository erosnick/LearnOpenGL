// Playground.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include <fstream>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define IMGUI_IMPL_OPENGL_LOADER_GLAD

#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/ImFileDialog.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>

#include "GeometryGenerator.h"

#include "Shader.h"
#include "Model.h"
#include "Camera.h"

struct Light {
	glm::vec4 color;
	glm::vec4 position;
    glm::vec3 direction;
    float exponent;
    float cutoff;
    float outerCutoff;
	float intensity;
    float Kc;
    float Kl;
    float Kq;
	int type;
};

struct Fog {
    float minDistance;
    float maxDistance;
    float density = 0.0f;
    glm::vec4 color;
};

uint32_t lightCubeVAO;
uint32_t screenQuadVAO;
uint32_t textureId;
std::shared_ptr<Shader> sceneShader;
std::shared_ptr<Shader> lightCubeShader;
std::shared_ptr<Shader> screenQuadShader;
std::shared_ptr<Shader> sobelOperatorShader;
std::shared_ptr<Shader> gaussianBlurShader;
std::shared_ptr<Shader> bloomShader;
std::shared_ptr<Shader> extractBrightnessShader;
std::shared_ptr<Shader> deferredShadingShader;
std::vector<std::shared_ptr<Model>> models;

std::map<std::string, std::shared_ptr<Texture>> textures;
std::map<std::string, std::shared_ptr<Material>> materials;

std::vector<Light> lights(5);

Fog fog = { 1.0f, 10.0f, 0.0f, {0.8f, 0.8f, 0.8f, 1.0f} };

float angle = 0.0f;

bool bShowDemoWindow = true;
bool bShowAnotherWindow = false;
bool bShowOpenMenuItem = true;
bool bShowProjector = false;
bool bDrawNormals = false;
bool bDrawBloom = false;
bool bMSAA = false;
ImVec4 clearColor = ImVec4(0.392f, 0.584f, 0.929f, 1.0f);
ImVec4 pointLightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 directionaLightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 lightDirection = ImVec4(0.0f, -1.0f, -1.0f, 0.0f);

float shininess = 32.0f;

GLFWwindow* window = nullptr;

std::vector<ImFont*> fonts;

int32_t WindowWidth = 1280;
int32_t WindowHeight = 720;

float aspect = static_cast<float>(WindowWidth) / WindowHeight;

float frameTime = 0.0f;

bool bRightMouseButtonDown = false;
bool bMiddleMouseButtonDown = false;

glm::vec2 lastMousePosition = { 0.0f, 0.0f };

float rotateSpeed = 3.0f;

glm::vec3 projectorPosition = glm::vec3(-2.5f, 2.5f, 0.0f);
glm::vec3 projectAt = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 projectorUp = glm::vec3(0.0f, 1.0f, 0.0f);

float fov = 60.0f;
float near = 0.2f;
float far = 200.0f;

//Camera camera(projectorPosition, projectAt);
Camera camera({ 0.0f, 2.5f, 4.5f }, { 0.0f, 2.5f, -1.0f });

glm::mat4 projectorView = glm::lookAt(projectorPosition, projectAt, projectorUp);
glm::mat4 projectorProjection = glm::perspective(glm::radians(60.0f), aspect, near, far);
glm::mat4 projectorScaleTranslate = glm::mat4(1.0f);
glm::mat4 projectorTransform = glm::mat4(1.0f);

uint32_t renderSceneFBO;
uint32_t gaussianBlurPassFBO;
uint32_t gaussianBlurredFBO;
uint32_t brightnessFBO;
uint32_t deferredShadingFBO;
std::shared_ptr<Texture> sceneTexture;
std::shared_ptr<Texture> objectToBloomTexture;
std::shared_ptr<Texture> gaussianBlurPassTexture;
std::shared_ptr<Texture> gaussianBlurredTexture;
std::shared_ptr<Texture> brightnessTexture;
std::shared_ptr<Texture> testTexture;
std::shared_ptr<Texture> defaultAlbedo;
std::shared_ptr<Texture> positionTexture;
std::shared_ptr<Texture> normalTexture;
std::shared_ptr<Texture> diffuseColorTexture;
uint32_t depthBuffer;

float edgeThreshold = 0.05f;
glm::vec3 edgeColor = { 1.0f, 1.0f, 1.0f };

char uniformName[20];
std::vector<float> weights(10);
float sum;
float sigmaSquared = 4.0f;

float luminanceThreshold = 0.95f;

const auto& getTexture(const std::string& name);
std::shared_ptr<Model> loadModel(const std::string& fileName, const std::string& inName = "", const std::string& materialPath = "./resources/models", const std::string& texturePath = "./resources/textures/");

void APIENTRY glDebugOutput(GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLsizei length,
    const GLchar* message,
    const void* userParam) {
// 忽略一些不重要的错误/警告代码
if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

std::cout << "---------------" << std::endl;
std::cout << "Debug message (" << id << "): " << message << std::endl;

switch (source)
{
case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
} std::cout << std::endl;

switch (type)
{
case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
} std::cout << std::endl;

switch (severity)
{
case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
} std::cout << std::endl;
std::cout << std::endl;
}

void onFrameBufferResize(GLFWwindow* window, int width, int height) {
    if (width > 0 && height > 0) {
        WindowWidth = width;
        WindowHeight = height;
        aspect = static_cast<float>(width) / height;
        camera.perspective(fov, aspect, near, far);
        glViewport(0, 0, width, height);
    }
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        bRightMouseButtonDown = true;
        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);
        lastMousePosition.x = static_cast<float>(x);
        lastMousePosition.y = static_cast<float>(y);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        bRightMouseButtonDown = false;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        bMiddleMouseButtonDown = true;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        bMiddleMouseButtonDown = false;
    }

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

void onScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
    
    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    camera.walk(static_cast<float>(yOffset / 8.0f));
}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
    double dx = (lastMousePosition.x - x) * frameTime;
    double dy = (lastMousePosition.y - y) * frameTime;

    if (bRightMouseButtonDown) {
        camera.yaw(static_cast<float>(dx) * rotateSpeed);
        camera.pitch(static_cast<float>(dy) * rotateSpeed);
    }

    if (bMiddleMouseButtonDown) {
        camera.strafe(static_cast<float>(-dx / 2.0f));
        camera.raise(static_cast<float>(dy / 2.0f));
    }

    lastMousePosition.x = static_cast<float>(x);
    lastMousePosition.y = static_cast<float>(y);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
        return;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.strafe(-frameTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.strafe(frameTime);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.walk(frameTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.walk(-frameTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        camera.raise(frameTime);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera.raise(-frameTime);
    }

    camera.updateViewMatrix();
}

void generateFrameBufferObject(uint32_t &fbo, const std::shared_ptr<Texture>& renderTexture, bool hasDepthBuffer = true) {
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture->getTextureId(), 0);

    if (hasDepthBuffer) {
        // Create the depth buffer
        glGenRenderbuffers(1, &depthBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTexture->getWidth(), renderTexture->getHeight());

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Bind the depth buffer to the FBO
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    }

    // Set the target for the fragment shader outputs
    uint32_t drawBuffers[] = {GL_NONE, GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(2, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Frame Buffer is not Complete." << std::endl;
    }

    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void generateGBuffer() {
    glGenFramebuffers(1, &deferredShadingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFBO);

    uint32_t depthBuffer;
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WindowWidth, WindowHeight);

    // The position buffer
    positionTexture = std::make_shared<Texture>("positionTexture", WindowWidth, WindowHeight, GL_NEAREST, false, GL_RGB32F, GL_RGB);

    // The normal buffer
    normalTexture = std::make_shared<Texture>("normalTexture", WindowWidth, WindowHeight, GL_NEAREST, false, GL_RGB32F, GL_RGB);

    // Diffuse color buffer
    diffuseColorTexture = std::make_shared<Texture>("diffuseColorTexture", WindowWidth, WindowHeight, GL_NEAREST, false, GL_RGB, GL_RGB);
    
    // Attach the images to the framebuffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTexture->getTextureId(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normalTexture->getTextureId(), 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, diffuseColorTexture->getTextureId(), 0);

    uint32_t drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(4, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Frame Buffer is not Complete." << std::endl;
    }

    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void prepareFrameBufferObject() {
    generateFrameBufferObject(renderSceneFBO, sceneTexture);
    generateFrameBufferObject(gaussianBlurPassFBO, gaussianBlurPassTexture, false);
    generateFrameBufferObject(gaussianBlurredFBO, gaussianBlurredTexture, false);
    generateFrameBufferObject(brightnessFBO, brightnessTexture, false);
    generateGBuffer();
}

float gaussian(int value, float sigmaSquared) {
    float exponent = glm::exp(-(glm::pow(static_cast<float>(value), 2.0f) / (2 * sigmaSquared)));
    float denominator = glm::sqrt(2.0f * glm::pi<float>() * sigmaSquared);
    return 1.0f / denominator * exponent;
}

void computeGaussianBlurWeights(std::vector<float>& weights, float sigmaSquared) {
    // Compute and sum the weights
    weights[0] = gaussian(0, sigmaSquared); // The 1-D Gaussian function
    sum = weights[0];

    // Sum of the raw Gaussian weights
    for (int i = 1; i < 10; i++) {
        weights[i] = gaussian(i, sigmaSquared);
        sum += 2 * weights[i];
    }

    gaussianBlurShader->use();

    // The Gaussian weights must sum to one in order to be a true weighted
    // average.Therefore, we need to normalize our Gaussian weights
    // Normalize the weights(weights[i] / sum part) and set the uniform
    for (int i = 0; i < 10; i++) {
        snprintf(uniformName, 20, "weights[%d]", i);
        gaussianBlurShader->setUniform(uniformName, weights[i] / sum);
        //std::cout << weights[i] / sum << std::endl;
    }
}

auto createShader(const std::string& name, const std::string& basePath) {
    auto shader = std::make_shared<Shader>(name);

    shader->create();
    shader->compileShaderFromFile(basePath + ".vert", ShaderType::VERTEX);
    shader->compileShaderFromFile(basePath + ".frag", ShaderType::FRAGMENT);

    shader->link();

    return shader;
}

void prepareShaderResources() {
    //auto vertexShader = loadShader("shaders/shader.vert", GL_VERTEX_SHADER);
    //auto fragmentShader = loadShader("shaders/shader.frag", GL_FRAGMENT_SHADER

    //sceneShader.compileShaderFromFile("shaders/texture.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("shaders/texture.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/lighting.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/lighting.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/twoside.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/twoside.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/subroutine.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/subroutine.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/spotlight.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/spotlight.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/celshading.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/celshading.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/fog.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/fog.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/texture.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/texture.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/alphatest.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/alphatest.frag", ShaderType::FRAGMENT);
    
    //sceneShader.compileShaderFromFile("./shaders/normalmap.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/normalmap.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/cubemapreflection.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/cubemapreflection.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/cubemaprefraction.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/cubemaprefraction.frag", ShaderType::FRAGMENT);
    
    //sceneShader.compileShaderFromFile("./shaders/projectivetexturemapping.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/projectivetexturemapping.frag", ShaderType::FRAGMENT);

    //sceneShader.compileShaderFromFile("./shaders/rendertotexture.vert", ShaderType::VERTEX);
    //sceneShader.compileShaderFromFile("./shaders/rendertotexture.frag", ShaderType::FRAGMENT);

    sceneShader = createShader("edgedetection", "./shaders/edgedetection");
    //sceneShader = createShader("gbuffer", "./shaders/gbuffer");

    lightCubeShader = createShader("color", "./shaders/color");

    screenQuadShader = createShader("screenquad", "./shaders/screenquad");
    
    sobelOperatorShader = createShader("sobeloperator", "./shaders/sobeloperator");

    gaussianBlurShader = createShader("gaussianblur", "./shaders/gaussianblur");
   
    bloomShader = createShader("bloom", "./shaders/bloom");

    extractBrightnessShader = createShader("extractBrightness", "./shaders/extractBrightness");

    deferredShadingShader = createShader("deferredshading", "./shaders/deferredshading");

    deferredShadingShader->printActiveUniforms();

    //// 获取uniform block索引
    //auto blockIndex = glGetUniformBlockIndex(sceneShader.get(), "BlobSettings");

    //int32_t blockSize = 0;

    //// 通过索引查询uniform block的数据大小
    //glGetActiveUniformBlockiv(sceneShader.get(), blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &blockSize);

    //uint8_t* blockBuffer = new uint8_t[blockSize];

    //// Query for the offsets of each block variable
    //const char* names[] = { "BlobSettings.InnerColor", "BlobSettings.OuterColor", "BlobSettings.RadiusInner", "BlobSettings.RadiusOuter" };
    //
    //// 通过名字查询uniform block每个变量的索引
    //uint32_t indices[4];
    //glGetUniformIndices(sceneShader.get(), 4, names, indices);

    //// 通过索引查询每个变量在uniform block中的位移
    //int32_t offsets[4];
    //glGetActiveUniformsiv(sceneShader.get(), 4, indices, GL_UNIFORM_OFFSET, offsets);

    //float innerColor[] = { 1.0f, 1.0f, 0.75f, 1.0f };
    //float outerColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
    //float innerRadius = 0.25f;
    //float outerRadius = 0.45f;

    //// 将数据拷贝到uniform block中
    //memcpy_s(blockBuffer + offsets[0], blockSize, innerColor, 4 * sizeof(float));
    //memcpy_s(blockBuffer + offsets[1], blockSize, outerColor, 4 * sizeof(float));
    //memcpy_s(blockBuffer + offsets[2], blockSize, &innerRadius, sizeof(float));
    //memcpy_s(blockBuffer + offsets[3], blockSize, &outerRadius, sizeof(float));

    //// 创建并绑定ubo
    //uint32_t ubo = 0;
    //glGenBuffers(1, &ubo);
    //glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    //glBufferData(GL_UNIFORM_BUFFER, blockSize, blockBuffer, GL_DYNAMIC_DRAW);

    //glBindBufferBase(GL_UNIFORM_BUFFER, blockIndex, ubo);

    //glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    //glBufferSubData(GL_UNIFORM_BUFFER, 0, blockSize, blockBuffer);

    //delete[] blockBuffer;

    lights[0].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[0].position = { 0.0f, 0.0f, -1.0f, 0.0f};
    lights[0].intensity = 0.5f;
    lights[0].Kc = 0.5f;
    lights[0].Kl = 0.09f;
    lights[0].Kq = 0.032f;
    lights[0].type = 1;

    lights[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[1].position = { -1.0f, 5.0f, 0.0f, 1.0f };
    lights[1].intensity = 0.25f;
    lights[1].Kc = 0.5f;
    lights[1].Kl = 0.09f;
    lights[1].Kq = 0.032f;
    lights[1].type = 0;

    lights[2].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[2].position = { -1.0f, 2.0f, -0.5f, 1.0f };
    lights[2].intensity = 0.25f;
    lights[2].Kc = 0.5f;
    lights[2].Kl = 0.09f;
    lights[2].Kq = 0.032f;
    lights[2].type = 0;

    lights[3].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[3].position = { 1.0f, -1.0f, 1.0f, 0.0};
    lights[3].intensity = 0.3f;
    lights[3].type = 1;
           
    //lights[2].color = { 1.0f, 0.418f, 1.0f, 1.0f };
    lights[4].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[4].position = {0.0f, 3.0f, 2.0f, 2.0f };
    lights[4].direction = { 0.0f, -1.0f, 0.0f };
    lights[4].exponent = 8.0f;
    lights[4].cutoff = 30.0f;
    lights[4].outerCutoff = 45.0f;
    lights[4].intensity = 1.0f;
    lights[4].type = 2;

    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.5f));
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f));

    glm::mat4 transform = translate * scale;

    projectorScaleTranslate = glm::translate(projectorScaleTranslate, glm::vec3(0.5f));
    projectorScaleTranslate = glm::scale(projectorScaleTranslate, glm::vec3(0.5f));

    glm::vec4 position = projectorScaleTranslate * glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

    //projectorTransform = projectorScaleTranslate * projectorProjection * projectorView;
    projectorTransform = projectorProjection * projectorView;

    prepareFrameBufferObject();
}

void prepareGeometryData() {
    Vertex vertices[] = {
        // Back
        {{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 0
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 1
        {{ 0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 2
        {{-0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 3

        // Front
        {{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 4
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 5
        {{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 6
        {{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 7

        // Left
        {{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f }}, // 8
        {{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f }}, // 9
        {{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f }}, // 10
        {{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f }}, // 11

        // Right
        {{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f }}, // 12
        {{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f }}, // 13
        {{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f }}, // 14
        {{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f }}, // 15

        {{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f }}, // 16
        {{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f }}, // 17
        {{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f }}, // 18
        {{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f }}, // 19

        {{-0.5f,  0.5f, -0.5f},  { 0.0f,  1.0f,  0.0f}}, // 20
        {{ 0.5f,  0.5f, -0.5f},  { 0.0f,  1.0f,  0.0f}}, // 21
        {{ 0.5f,  0.5f,  0.5f},  { 0.0f,  1.0f,  0.0f}}, // 22
        {{-0.5f,  0.5f,  0.5f},  { 0.0f,  1.0f,  0.0f}}, // 23
    };

    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6,
        6, 7, 4,
        8, 9, 10,
        10, 11, 8,
        12, 13, 14,
        14, 15, 12,
        16, 17, 18,
        18, 19, 16,
        20, 21, 22,
        22, 23, 20
    };

    // Create the buffer objects
    uint32_t lightCubeVBO;
    glGenBuffers(1, &lightCubeVBO);

    uint32_t lightCubeIBO;
    glGenBuffers(1, &lightCubeIBO);

    // Create and setup the vertex array object
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    int32_t stride = sizeof(Vertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)nullptr);
    // Map index 0 to the position buffer
    glEnableVertexAttribArray(0);	// Vertex Position

        // Enable the vertex attribute arrays
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    // Map index 1 to the tangent buffer
    glEnableVertexAttribArray(1);	// tangent

    // Enable the vertex attribute arrays
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    // Map index 1 to the binormal buffer
    glEnableVertexAttribArray(2);	// binormal

    // Enable the vertex attribute arrays
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
    // Map index 1 to the normal buffer
    glEnableVertexAttribArray(3);	// normal

    glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 12));
    // Map index 1 to the texture coordinate buffer
    glEnableVertexAttribArray(4);	//

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightCubeIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    SimpleVertex screenQuadVertices[] = {
        {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{WindowWidth, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{WindowWidth, WindowHeight, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{0.0f, WindowHeight, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };

    uint32_t screenQuadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &screenQuadVAO);
    glBindVertexArray(screenQuadVAO);

    uint32_t screenQuadVbo;
    glGenBuffers(1, &screenQuadVbo);
    glBindBuffer(GL_ARRAY_BUFFER, screenQuadVbo);

    glBufferData(GL_ARRAY_BUFFER, sizeof(screenQuadVertices), screenQuadVertices, GL_STATIC_DRAW);

    stride = sizeof(SimpleVertex);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)nullptr);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    glEnableVertexAttribArray(2);
    
    uint32_t screenQuadIbo;

    glGenBuffers(1, &screenQuadIbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, screenQuadIbo);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(screenQuadIndices), screenQuadIndices, GL_STATIC_DRAW);
}

void initImGui()
{
    // Setup Dear ImGui context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style.
    ImGui::StyleColorsDark();
    ImGui::GetStyle().ScaleAllSizes(1.0f);
    // ImGui::StyleColorsClassic();

    // Setup platform/Renderer bindings.
    const char* glsl_version = "#version 400";
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'misc/fonts/README.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/Roboto-Medium.ttf", 10.0f));
    //fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/Cousine-Regular.ttf", 10.0f));
    //fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/DroidSans.ttf", 10.0f));
    //fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/ProggyTiny.ttf", 10.0f));
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    ifd::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt) -> void* {
        GLuint tex;

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, (fmt == 0) ? GL_BGRA : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        return (void*)tex;
    };

    ifd::FileDialog::Instance().DeleteTexture = [](void* tex) {
        GLuint texID = (GLuint)tex;
        glDeleteTextures(1, &texID);
    };
}

void showMenuBar()
{
	// Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
                ifd::FileDialog::Instance().Open("ModelOpenDialog", "Open a model", "Model file (*.obj;){.obj},.*");
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}

    if (ifd::FileDialog::Instance().IsDone("ModelOpenDialog")) {
        if (ifd::FileDialog::Instance().HasResult()) {
            std::string fileName = ifd::FileDialog::Instance().GetResult().u8string();
            printf("OPEN[%s]\n", fileName.c_str());
			models.push_back(loadModel(fileName));
			prepareGeometryData();
        }
        ifd::FileDialog::Instance().Close();
    }
}

const auto& getTexture(const std::string& name) {
    return textures[name];
}

auto createTexture(const std::string& name, int32_t width, int32_t height, int32_t filter = GL_LINEAR, int32_t internalFormat = GL_RGBA, int32_t format = GL_RGBA) {
    return std::make_shared<Texture>(name, width, height, filter, internalFormat, format);
}

auto addTexture(const std::string& name, const std::string& path, int32_t wrapMode = GL_REPEAT) {
    auto texture = getTexture(name);

    if (texture) {
        return texture;
    }

    if (std::filesystem::exists(path)) {
        auto texture = std::make_shared<Texture>(path, wrapMode);
        textures[name] = texture;
        return texture;
    }

    return std::make_shared<Texture>();
}

auto addCubemapTexture(const std::string& name, const std::string& path, int32_t wrapMode = GL_REPEAT, bool cubeMap = false, bool hdr = false) {
    auto texture = getTexture(name);

    if (texture) {
        return texture;
    }

    texture = std::make_shared<Texture>(path, wrapMode, cubeMap, hdr);
    if (texture) {
        textures[name] = texture;
        return texture;
    }

    return std::make_shared<Texture>();
}

void addMaterial(const std::string& name, const std::shared_ptr<Material>& material) {
    materials[name] = material;
}

const auto& getMaterial(const std::string& name) {
    return materials[name];
}

std::shared_ptr<Model> loadModel(const std::string& fileName, const std::string& inName, const std::string& materialPath, const std::string& texturePath) {
    tinyobj::ObjReaderConfig readConfig;
    readConfig.mtl_search_path = materialPath;

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(fileName, readConfig)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjRead: " << reader.Error();
        }
        return nullptr;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto slash = fileName.find_last_of('/');
    auto dot = fileName.find_last_of('.');

    auto model = std::make_shared<Model>();

    if (!inName.empty()) {
        model->setName(inName);
    }
    else {
        model->setName(fileName.substr(slash + 1, dot - (slash + 1)));
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    // 三层循环分别遍历mesh(s), face(s), vertex(s)
    // 不进行任何过滤的情况下，获得的顶点数组会包含
    // 荣誉数据(比如支持贴图和光照的立方体最多只需24个顶点)
    // 而这里会产生36个顶点，它们的排布是线性的，绘制的时候
    // 直接调用glDrawArrays即可，但是更优化的做法是过滤掉
    // 冗余的顶点，并使用glDrawElements来进行绘制
    // Loop over shapes
    //for (size_t s = 0; s < shapes.size(); s++) {
    //    // Loop over faces(polygon)
    //    size_t indexOffset = 0;
    //    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
    //        size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

    //        // Loop over vertices in the face
    //        for (size_t v = 0; v < fv; v++) {
    //            // Access to vertex
    //            tinyobj::index_t index = shapes[s].mesh.indices[indexOffset + v];
    //            tinyobj::real_t vx = attrib.vertices[3 * size_t(index.vertex_index) + 0];
    //            tinyobj::real_t vy = attrib.vertices[3 * size_t(index.vertex_index) + 1];
    //            tinyobj::real_t vz = attrib.vertices[3 * size_t(index.vertex_index) + 2];

    //            Vertex vertex;

    //            vertex.position = { vx, vy, vz };

    //            // Check if 'normal_index' is zero of positive. negative = no normal data
    //            if (index.normal_index >= 0) {
    //                tinyobj::real_t nx = attrib.normals[3 * size_t(index.normal_index) + 0];
    //                tinyobj::real_t ny = attrib.normals[3 * size_t(index.normal_index) + 1];
//                tinyobj::real_t nz = attrib.normals[3 * size_t(index.normal_index) + 2];
//                vertex.normal = { nx, ny, nz };
//            }

//            // Check if 'texcoord_index' is zero or positive. negative = no texcoord data
//            if (index.texcoord_index >= 0) {
//                tinyobj::real_t tx = attrib.texcoords[2 * size_t(index.texcoord_index) + 0];
//                tinyobj::real_t ty = 1.0f - attrib.texcoords[2 * size_t(index.texcoord_index) + 1];
//                vertex.texCoord = { tx, ty };
//            }

//            // 只有顶点的每个分量都相同时(这里包含position, normal和texcoord)，才考虑它是唯一
//            // 举例来说，要让一个立方体能支持正确的纹理贴图和光照计算，总计需要24个顶点
//            // 因为立方体有6个面，每个面4个顶点，总计24个顶点，都需要有独立的normal和texcoord
//            if (uniqueVertices.count(vertex) == 0) {
//                uniqueVertices[vertex] = static_cast<uint32_t>(model.getVertices().size());
//                model.addVertex(vertex);
//            }

//            // 如函数开头所说，tinyobjloader读取出的顶点数据是线性的
//            // 还是以立方体为例，36个顶点数组对应的索引数组就是0~35
//            // 上面经过顶点去重之后，索引也要跟着更新，既然顶点数组
//            // 是线性的，我们可以直接用去重过后的顶点数组尺寸做索引
//            // 所有的索引值肯定是位于0 ~ model.vertices.size() - 1
//            // 之间。model.indices数组的大小最后一定是
//            // shapes[s].mesh.num_face_vertices.size() * fv，
//            // 也就是三角形面数 * 3
//            model.addIndex(uniqueVertices[vertex]);
//        }
//        indexOffset += fv;

//        // per-face material
//        shapes[s].mesh.material_ids[f];
//    }
//} 
    size_t materialIndex = 0;

    for (const auto& shape : shapes) {
        auto mesh = std::make_shared<Mesh>();
        std::unordered_map<Vertex, uint32_t> uniqueVertices;
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex = {};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            // Check if 'normal_index' is zero of positive. negative = no normal data
            if (index.normal_index >= 0) {
                tinyobj::real_t nx = attrib.normals[3 * size_t(index.normal_index) + 0];
                tinyobj::real_t ny = attrib.normals[3 * size_t(index.normal_index) + 1];
                tinyobj::real_t nz = attrib.normals[3 * size_t(index.normal_index) + 2];
                vertex.normal = { nx, ny, nz };
            }

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(mesh->getVertices().size());
                mesh->addVertex(vertex);
            }

            mesh->addIndex(uniqueVertices[vertex]);
        }

        mesh->setName(shape.name);

        const auto& material = materials[materialIndex];

        auto meshMaterial = std::make_shared<Material>();

        meshMaterial->Ka = { material.ambient[0], material.ambient[1], material.ambient[2] };
        meshMaterial->Kd = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
        meshMaterial->Ke = { material.emission[0], material.emission[2], material.emission[2] };
        meshMaterial->Ks = { material.specular[0], material.specular[1], material.specular[2] };

        meshMaterial->shininess = material.shininess;
        meshMaterial->ior = material.ior;
        meshMaterial->eta = 1.0f / meshMaterial->ior;

        if (!material.diffuse_texname.empty()) {
            auto texture = addTexture(material.name + "Diffuse", texturePath + material.diffuse_texname);

            if (texture) {
                mesh->addTexture(texture);
            }
 
        }
        else {
            mesh->addTexture(defaultAlbedo);
        }

        if (!material.bump_texname.empty()) {
            auto texture = addTexture(material.name + "Normal", texturePath + material.bump_texname);

            if (texture) {
                mesh->addTexture(texture);
                meshMaterial->hasNormalMap = true;
            }
        }

        mesh->setMaterial(std::move(meshMaterial));

        materialIndex++;

        model->addMesh(std::move(mesh));
    }

    return model;
}

void buildImGuiWidgets()
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (bShowDemoWindow)
        ImGui::ShowDemoWindow(&bShowDemoWindow);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.

		showMenuBar();

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        //ImGui::PushFont(fonts[2]);
        ImGui::Text("This is some useful text use another font.");
        //ImGui::PopFont();
        ImGui::Checkbox("Demo Window", &bShowDemoWindow);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &bShowAnotherWindow);

        auto commonMaterial = getMaterial("Common");

        ImGui::ColorEdit3("Ambient", (float*)&commonMaterial->Ka); // Edit 1 float using a slider from 0.1f to 1.0f
        ImGui::SliderFloat("Reflection", &commonMaterial->reflectionFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Refraction", &commonMaterial->refractionFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &commonMaterial->shininess, 32.0f, 128.0f);
        ImGui::SliderFloat("Fog Density", &fog.density, 0.0f, 1.0f);
        ImGui::SliderFloat("Edge Threshold", &edgeThreshold, 0.05f, 1.0f);
        ImGui::SliderFloat("Luminance Threshold", &luminanceThreshold, 0.0f, 1.0f);
        ImGui::SliderFloat("SigmaSquared", &sigmaSquared, 0.001f, 10.0f);
        ImGui::ColorEdit3("Edge color", (float*)&edgeColor);
        ImGui::ColorEdit3("Clear color", (float*)&clearColor); // Edit 3 floats representing a color
        ImGui::ColorEdit3("Point Light Color", (float*)&lights[0].color);
        ImGui::ColorEdit3("Directional Light Color", (float*)&lights[1].color);
        ImGui::DragFloat3("Light Direction", (float*)&lights[1].position, 0.1f, -1.0f, 1.f);
        ImGui::DragFloat3("Light Position", (float*)&lights[0].position, 0.1f, -10.0f, 10.f);
        ImGui::Checkbox("Projective Texture Mapping", &bShowProjector);
        ImGui::Checkbox("Draw Normals", &bDrawNormals);
        ImGui::Checkbox("Draw Bloom", &bDrawBloom);
        ImGui::Checkbox("MSAA", &bMSAA);

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        frameTime = 1.0f / ImGui::GetIO().Framerate;

        ImGui::Text("Camera Position %f, %f, %f", camera.getEye().x, camera.getEye().y, camera.getEye().z);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (bShowAnotherWindow)
    {
        ImGui::Begin("Another Window", &bShowAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

		ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            bShowAnotherWindow = false;
        ImGui::End();
    }
}

void bindCallbacks() {
	glfwSetFramebufferSizeCallback(window, onFrameBufferResize);
	glfwSetKeyCallback(window, onKeyCallback);
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);
    glfwSetScrollCallback(window, onScrollCallback);
    glfwSetCursorPosCallback(window, onMouseMoveCallback);
}

void update() {
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

	buildImGuiWidgets();
}

void renderImGui() {
    // Rendering
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void updateGlobalUniform() {
    sceneShader->use();
    sceneShader->setUniform("lights[0].color", lights[0].color);
    sceneShader->setUniform("lights[0].position", lights[0].position);
    sceneShader->setUniform("lights[0].intensity", lights[0].intensity);
    sceneShader->setUniform("lights[0].Kc", lights[0].Kc);
    sceneShader->setUniform("lights[0].Kl", lights[0].Kl);
    sceneShader->setUniform("lights[0].Kq", lights[0].Kq);
    sceneShader->setUniform("lights[0].type", lights[0].type);

    sceneShader->setUniform("lights[1].color", lights[1].color);
    sceneShader->setUniform("lights[1].position", lights[1].position);
    sceneShader->setUniform("lights[1].intensity", lights[1].intensity);
    sceneShader->setUniform("lights[1].Kc", lights[1].Kc);
    sceneShader->setUniform("lights[1].Kl", lights[1].Kl);
    sceneShader->setUniform("lights[1].Kq", lights[1].Kq);
    sceneShader->setUniform("lights[1].type", lights[1].type);

    sceneShader->setUniform("lights[2].color", lights[2].color);
    sceneShader->setUniform("lights[2].position", lights[2].position);
    sceneShader->setUniform("lights[2].intensity", lights[2].intensity);
    sceneShader->setUniform("lights[2].Kc", lights[2].Kc);
    sceneShader->setUniform("lights[2].Kl", lights[2].Kl);
    sceneShader->setUniform("lights[2].Kq", lights[2].Kq);
    sceneShader->setUniform("lights[2].type", lights[2].type);
    
    sceneShader->setUniform("lights[3].color", lights[3].color);
    sceneShader->setUniform("lights[3].position", lights[3].position);
    sceneShader->setUniform("lights[3].intensity", lights[3].intensity);
    sceneShader->setUniform("lights[3].type", lights[3].type);

    sceneShader->setUniform("lights[4].color", lights[4].color);
    sceneShader->setUniform("lights[4].position", lights[4].position);
    sceneShader->setUniform("lights[4].direction", lights[4].direction);
    sceneShader->setUniform("lights[4].exponent", lights[4].exponent);
    sceneShader->setUniform("lights[4].cutoff", glm::cos(glm::radians(lights[4].cutoff)));
    sceneShader->setUniform("lights[4].outerCutoff", glm::cos(glm::radians(lights[4].outerCutoff)));
    sceneShader->setUniform("lights[4].intensity", lights[4].intensity);
    sceneShader->setUniform("lights[4].type", lights[4].type);

    sceneShader->setUniform("fog.minDistance", fog.minDistance);
    sceneShader->setUniform("fog.maxDistance", fog.maxDistance);
    sceneShader->setUniform("fog.density", fog.density);
    sceneShader->setUniform("fog.color", fog.color);

    sceneShader->setUniform("cubeMap", getTexture("CubeMap")->getTextureIndex());

    sceneShader->setUniform("eye", camera.getEye());

    sceneShader->setUniform("projectorTransform", projectorTransform);

    sceneShader->setUniform("projection", getTexture("Projection")->getTextureIndex());
    
    sceneShader->setUniform("showProjector", bShowProjector);
}

void drawSkyBox(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    sceneShader->setUniform("drawSkyBox", true);

    auto& model = models[0];

    auto& mesh = model->getMeshes()[0];
    
    mesh->use();

    glm::mat4 viewMatrix = inViewMaterix;

    // 消除平移部分，使天空盒和玩家一起移动(无限远的效果)
    viewMatrix[3][0] = 0.0f;
    viewMatrix[3][1] = 0.0f;
    viewMatrix[3][2] = 0.0f;

    glm::mat4 worldMatrix = model->getTransform();
    glm::mat4 mvpMatrix = inProjectionMatrix * viewMatrix * worldMatrix;

    sceneShader->setUniform("worldMatrix", worldMatrix);
    sceneShader->setUniform("mvpMatrix", mvpMatrix);
    
    glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
}

void drawLights(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    lightCubeShader->use();

    for (size_t i = 0; i < lights.size(); i++) {
        if (lights[i].type == 0) {
            glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(lights[i].position.x, lights[i].position.y, lights[i].position.z));
            worldMatrix = glm::scale(worldMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

            glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

            lightCubeShader->setUniform("mvpMatrix", mvpMatrix);
            lightCubeShader->setUniform("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
                           
            glBindVertexArray(lightCubeVAO);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
            glDrawElements(GL_TRIANGLES, 64, GL_UNSIGNED_INT, 0);
        }
    }
}

void updateMaterialUniform(const std::shared_ptr<Material>& material) {
    if (material) {
        sceneShader->setUniform("material.Ka", material->Ka);
        sceneShader->setUniform("material.Kd", material->Kd);
        sceneShader->setUniform("material.Ks", material->Ks);
        sceneShader->setUniform("material.Ke", material->Ke);
        sceneShader->setUniform("material.shininess", material->shininess);
        sceneShader->setUniform("material.reflectionFactor", material->reflectionFactor);
        sceneShader->setUniform("material.refractionFactor", material->refractionFactor);
        sceneShader->setUniform("material.ior", material->ior);
        sceneShader->setUniform("material.eta", material->eta);
        sceneShader->setUniform("material.hasNormalMap", material->hasNormalMap);
    }              
}

void drawModel(const std::shared_ptr<Model>& model, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
  
    sceneShader->setUniform("drawSkyBox", false);

    for (auto& mesh : model->getMeshes()) {
        mesh->use();

        auto material = mesh->getMaterial();

        updateMaterialUniform(material);

        sceneShader->setUniform("textures[0]", mesh->getTextureIndex(0));
        sceneShader->setUniform("textures[1]", mesh->getTextureIndex(1));
        sceneShader->setUniform("material.Kd", glm::vec3(1.0f, 1.0f, 1.0f));

        glm::mat4 worldMatrix = model->getTransform();
        glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

        sceneShader->setUniform("worldMatrix", worldMatrix);
        sceneShader->setUniform("mvpMatrix", mvpMatrix);
        sceneShader->setUniform("projectionMatrix", inProjectionMatrix);
        //sceneShader->setUniform("normalMatrix", glm::transpose(glm::inverse(worldMatrix)));
        sceneShader->setUniform("normalMatrix", worldMatrix);

        glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
    }
}

void drawModels(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    for (size_t i = 1; i < models.size() - 1; i++) {
        drawModel(models[i], inViewMaterix, inProjectionMatrix);
    }
}

void drawNormals(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    lightCubeShader->use();

    lightCubeShader->setUniform("color", glm::vec4(0.270588f, 0.552941f, 0.874510f, 1.0f));

    for (size_t i = 1; i < models.size(); i++) {
        glm::mat4 worldMatrix = models[i]->getTransform();
        for (auto& mesh : models[i]->getMeshes()) {
            mesh->useNormal();

            glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

            lightCubeShader->setUniform("mvpMatrix", mvpMatrix);
            glDrawArrays(GL_LINES, 0, mesh->getNormalIndexCount());
        }
    }
}

void clear(ImVec4 color, int32_t clearFlag) {
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(clearFlag);
}

void drawScene(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {
    clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    sceneShader->use();

    //drawSkyBox(viewMatrix, projectionMatrix);

    drawModels(viewMatrix, projectionMatrix);
}

void renderToTexture(uint32_t width = 512, uint32_t height = 512) {
    // Bind to texture's FBO
    glBindFramebuffer(GL_FRAMEBUFFER, renderSceneFBO);
    glViewport(0, 0, width, height); // Viewport for the texture

    clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewMatrix = camera.getViewMatrix();
    camera.perspective(fov, aspect, near, far);
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    drawScene(viewMatrix, projectionMatrix);

    drawLights(viewMatrix, projectionMatrix);

    // Unbind texture's FBO (back to default FB)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawEdgeDetection(const std::shared_ptr<Texture>& renderTexture) {

    glBindVertexArray(screenQuadVAO);

    camera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    sobelOperatorShader->use();
    sobelOperatorShader->setUniform("renderTexture", renderTexture->getTextureIndex());
    sobelOperatorShader->setUniform("projectionMatrix", projectionMatrix);
    sobelOperatorShader->setUniform("width", WindowWidth);
    sobelOperatorShader->setUniform("height", WindowHeight);
    sobelOperatorShader->setUniform("edgeThreshold", edgeThreshold);
    sobelOperatorShader->setUniform("edgeColor", edgeColor);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawGaussianBlur(const std::shared_ptr<Texture>& renderTexture, float verticalPass = true) {
    glBindVertexArray(screenQuadVAO);

    camera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    gaussianBlurShader->use();
    gaussianBlurShader->setUniform("renderTexture", renderTexture->getTextureIndex());
    gaussianBlurShader->setUniform("projectionMatrix", projectionMatrix);
    gaussianBlurShader->setUniform("width", WindowWidth);
    gaussianBlurShader->setUniform("height", WindowHeight);
    gaussianBlurShader->setUniform("verticalPass", verticalPass);

    computeGaussianBlurWeights(weights, sigmaSquared);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawBloom(const std::shared_ptr<Texture>& sceneTexture, const std::shared_ptr<Texture>& bloomTexture) {
    glBindVertexArray(screenQuadVAO);

    camera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    bloomShader->use();
    bloomShader->setUniform("sceneTexture", sceneTexture->getTextureIndex());
    bloomShader->setUniform("bloomTexture", bloomTexture->getTextureIndex());
    bloomShader->setUniform("projectionMatrix", projectionMatrix);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void extractBrightness(const std::shared_ptr<Texture>& sceneTexture, float luminanceThreshold) {
    glBindVertexArray(screenQuadVAO);

    camera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    extractBrightnessShader->use();
    extractBrightnessShader->setUniform("sceneTexture", sceneTexture->getTextureIndex());
    extractBrightnessShader->setUniform("projectionMatrix", projectionMatrix);
    extractBrightnessShader->setUniform("luminanceThreshold", luminanceThreshold);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawScreenQuad(const std::shared_ptr<Texture>& renderTexture) {
    glBindVertexArray(screenQuadVAO);

    camera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    screenQuadShader->use();
    screenQuadShader->setUniform("projectionMatrix", projectionMatrix);
    screenQuadShader->setUniform("renderTexture", renderTexture->getTextureIndex());

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void deferredShading() {
    glBindVertexArray(screenQuadVAO);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    camera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    //projectionMatrix = glm::mat4(1.0f);

    //glm::mat4 scale = glm::mat4(1.0f);

    ////  Orthogonal projection near = -1.0f, far 1.0f
    //scale[0][0] = 2.0f / WindowWidth;
    //scale[1][1] = 2.0f / WindowHeight;
    //scale[2][2] = -1.0f; // 2.0f / (near - far) = 2.0f / (-1.0f - 1.0f) = -1.0f
    //                     // or -2.0f / (far - near) = -2.0f / (1.0f - (-1.0f)) = -1.0f
    //scale[2][3] = 1.0f;

    //glm::mat4 translate = glm::mat4(1.0f);

    //translate[3][0] = -WindowWidth / 2.0f;
    //translate[3][1] = -WindowHeight / 2.0f;
    //translate[3][2] = 0.0f;

    //projectionMatrix = scale * translate;

    deferredShadingShader->use();
    deferredShadingShader->setUniform("projectionMatrix", projectionMatrix);
    deferredShadingShader->setUniform("positionTexture", positionTexture->getTextureIndex());
    deferredShadingShader->setUniform("normalTexture", normalTexture->getTextureIndex());
    deferredShadingShader->setUniform("diffuseColorTexture", diffuseColorTexture->getTextureIndex());
    deferredShadingShader->setUniform("light.color", lights[0].color);
    deferredShadingShader->setUniform("light.position", lights[0].position);
    deferredShadingShader->setUniform("light.intensity", lights[0].intensity);
    deferredShadingShader->setUniform("light.Kc", lights[0].Kc);
    deferredShadingShader->setUniform("light.Kl", lights[0].Kl);
    deferredShadingShader->setUniform("light.Kq", lights[0].Kq);
    deferredShadingShader->setUniform("light.type", lights[0].type);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void renderGBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, deferredShadingFBO);
    clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, WindowWidth, WindowHeight);
    glm::mat4 viewMatrix = camera.getViewMatrix();

    viewMatrix = glm::mat4(1.0f);

    glm::vec3 right = camera.getRight();
    glm::vec3 up = camera.getUp();
    glm::vec3 forward = camera.getForward();

    viewMatrix[0][0] = right.x;
    viewMatrix[0][1] = up.x;
    viewMatrix[0][2] = -forward.x;

    viewMatrix[1][0] = right.y;
    viewMatrix[1][1] = up.y;
    viewMatrix[1][2] = -forward.y;

    viewMatrix[2][0] = right.z;
    viewMatrix[2][1] = up.z;
    viewMatrix[2][2] = -forward.z;

    viewMatrix[3][0] = -glm::dot(right, camera.getEye());
    viewMatrix[3][1] = -glm::dot(up, camera.getEye());
    viewMatrix[3][2] = glm::dot(forward, camera.getEye());

    camera.perspective(fov, aspect, near, far);
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    //projectionMatrix = glm::mat4(1.0f);

    //glm::mat4 scale = glm::mat4(1.0f);

    ////  Orthogonal projection near = -1.0f, far 1.0f
    //scale[0][0] = 2.0f / WindowWidth;
    //scale[1][1] = 2.0f / WindowHeight;
    //scale[2][2] = 2.0f / (near - far); // 2.0f / (near - far) = 2.0f / (-1.0f - 1.0f) = -1.0f
    //                                   // or -2.0f / (far - near) = -2.0f / (1.0f - (-1.0f)) = -1.0f
    ////scale[2][3] = 1.0f;

    //glm::mat4 translate = glm::mat4(1.0f);

    //translate[3][0] = -WindowWidth / 2.0f;
    //translate[3][1] = -WindowHeight / 2.0f;
    //translate[3][2] = -(near + far) / 2.0f;

    //glm::mat4 orthogonalMatrix = scale * translate;

    //projectionMatrix[0][0] = near;
    //projectionMatrix[1][1] = near;
    //projectionMatrix[2][2] = near + far; // A
    //projectionMatrix[2][3] = 1.0f;
    //projectionMatrix[3][2] = -near * far; // B
    //projectionMatrix[3][3] = 0.0f;

    //glm::vec4 np = { 10.0f, 10.0f, near, 1.0f};
    //glm::vec4 fp = { 0.0f, 0.0f, far, 1.0f};

    //np = projectionMatrix * np;
    //fp = projectionMatrix * fp;

    //projectionMatrix = orthogonalMatrix * projectionMatrix;

    drawScene(viewMatrix, projectionMatrix);
    drawModel(models[models.size() - 1], viewMatrix, projectionMatrix);
}

void renderScene()
{
    updateGlobalUniform();

    if (bMSAA) {
        glEnable(GL_MULTISAMPLE);
    }
    else {
        glDisable(GL_MULTISAMPLE);
    }

    //renderToTexture(WindowWidth, WindowHeight);

    //renderGBuffer();

    glViewport(0, 0, WindowWidth, WindowHeight);
    glm::mat4 viewMatrix = camera.getViewMatrix();
    camera.perspective(fov, aspect, near, far);
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    //glViewport(WindowWidth - WindowWidth / 4, WindowHeight - WindowHeight / 4, WindowWidth / 4, WindowHeight / 4);
    //viewMatrix = camera.getViewMatrix();
    //camera.perspective(fov, static_cast<float>(WindowWidth) / WindowHeight, near, far);
    //projectionMatrix = camera.getProjectionMatrix();

    projectionMatrix = glm::mat4(1.0f);

    glm::mat4 scale = glm::mat4(1.0f);

    float nearZ = -near;
    float farZ = -far;

    // 这里陷入了之前做全屏四边形渲染时的思维定势，以为l r b t要符合窗口大小
    // 而实际上这里的l r b t是和投影平面相关的。它们之间存在着以下关系：
    // l = -r, l < r; b = -t, b < t，(r - l) = w, (t - b) = h
    // 为了简单起见可以直接将使h = 2，则w = h * aspect，垂直视角α
    // 则有b = -tan(α / 2) * near = -h / 2, 
    // t = tan(α / 2) * near = h / 2
    // 水平视角为β，同理有l = -tan(β / 2) * near = -w / 2
    // 因为w = h * aspect, -w / 2 = -(h * aspect) / 2 
    // = -tan(α / 2) * near * aspect，这里还可以求出水平视角β
    float tanHalfFovy = glm::tan(glm::radians(fov / 2.0f));
    float r = tanHalfFovy * nearZ * aspect;
    float l = -r; 
    float t = tanHalfFovy * nearZ;
    float b = -t;
    scale[0][0] = 2.0f / (r - l);
    scale[1][1] = 2.0f / (t - b);
    scale[2][2] = 2.0f / (nearZ - farZ);
    //scale[2][3] = 1.0f;

    glm::mat4 translate = glm::mat4(1.0f);

    translate[3][0] = -(r + l) / 2.0f;
    translate[3][1] = -(b + t) / 2.0f;
    translate[3][2] = -(farZ + nearZ) / 2.0f;

    glm::mat4 orthogonalMatrix = scale * translate;

    orthogonalMatrix = glm::ortho(l, r, b, t, nearZ, farZ);

    projectionMatrix[0][0] = nearZ;
    projectionMatrix[1][1] = nearZ;
    projectionMatrix[2][2] = nearZ + farZ; // A
    projectionMatrix[2][3] = -1.0f;
    projectionMatrix[3][2] = -nearZ * farZ; // B
    projectionMatrix[3][3] = 0.0f;

    ////glm::vec4 np = { 10.0f, 10.0f, near, 1.0f };
    ////glm::vec4 fp = { 0.0f, 0.0f, far, 1.0f };

    ////np = projectionMatrix * np;
    ////fp = projectionMatrix * fp;

    projectionMatrix = orthogonalMatrix * projectionMatrix;

    //projectionMatrix[0][0] = static_cast<float>(1) / (aspect * tanHalfFovy);
    //projectionMatrix[1][1] = static_cast<float>(1) / (tanHalfFovy);
    //projectionMatrix[2][2] = (nearZ + farZ) / (nearZ - farZ);
    //projectionMatrix[2][3] = -static_cast<float>(1);
    //projectionMatrix[3][2] = -(static_cast<float>(2) * nearZ * farZ) / (nearZ - farZ);

    drawScene(viewMatrix, projectionMatrix);

    drawModel(models[models.size() - 1], viewMatrix, projectionMatrix);

    drawLights(viewMatrix, projectionMatrix);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //drawScreenQuad(diffuseColorTexture);

    //deferredShading();

    if (bDrawNormals) {
        drawNormals(viewMatrix, projectionMatrix);
    }

    //glBindFramebuffer(GL_FRAMEBUFFER, gaussianBlurPassFBO);
    //clear(clearColor, GL_COLOR_BUFFER_BIT);
    //drawGaussianBlur(renderTexture);

    //glBindFramebuffer(GL_FRAMEBUFFER, gaussianBlurredFBO);
    //clear(clearColor, GL_COLOR_BUFFER_BIT);
    //drawGaussianBlur(gaussianBlurPassTexture, false);

    //glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //drawGaussianBlur(gaussianBlurPassTexture, false);

    if (bDrawBloom) {
        glBindFramebuffer(GL_FRAMEBUFFER, brightnessFBO);
        clear(clearColor, GL_COLOR_BUFFER_BIT);
        extractBrightness(sceneTexture, luminanceThreshold);

        // Gaussian blur first pass
        glBindFramebuffer(GL_FRAMEBUFFER, gaussianBlurPassFBO);
        clear(clearColor, GL_COLOR_BUFFER_BIT);
        drawGaussianBlur(brightnessTexture);

        // Gaussian blur second pass
        glBindFramebuffer(GL_FRAMEBUFFER, gaussianBlurredFBO);
        clear(clearColor, GL_COLOR_BUFFER_BIT);
        drawGaussianBlur(gaussianBlurPassTexture, false);

        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //drawScreenQuad(positionTexture);

        //// Sobel operator edge detection pass
        //glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //drawEdgeDetection(gaussianBlurredTexture);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawBloom(sceneTexture, gaussianBlurredTexture);
    }
}

void render() {
	renderScene();
	renderImGui();
}

void loadModels() {
    auto material = std::make_shared<Material>();

    material->Ka = { 0.37f, 0.37f, 0.37f };
    material->Kd = { 0.87f, 0.87f, 0.87f };
    material->Ks = { 0.49f, 0.49f, 0.49f };
    material->shininess = 32.0f;
    material->reflectionFactor = 0.0f;
    material->refractionFactor = 0.0f;
    // air / glass
    material->eta = 1.0f / 1.5f;

    addMaterial("Common", material);

    auto model = loadModel("./resources/models/cube.obj", "SkyBox");
    model->scale(glm::vec3(100.0f));
    models.push_back(model); 
    
    model = loadModel("./resources/models/torus.obj");
    model->setPosition(glm::vec3(-2.5f, 0.0f, 1.0f));

    //models.push_back(model);

    //model = std::make_shared<Model>();

    //GeometryGenerator geometryGenerator;
    //auto sphere = geometryGenerator.CreateSphere(1, 20, 20);

    //auto mesh = std::make_shared<Mesh>();

    //for (size_t i = 0; i < sphere.Vertices.size(); i++) {
    //    Vertex vertex;
    //    vertex.position = sphere.Vertices[i].Position;
    //    vertex.normal = sphere.Vertices[i].Normal;
    //    vertex.texCoord = sphere.Vertices[i].TexC;
    //    mesh->addVertex(vertex);
    //}

    //for (size_t i = 0; i < sphere.Indices32.size(); i++) {
    //    mesh->addIndex(sphere.Indices32[i]);
    //}

    //model->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));

    //models.push_back(model);

    model = loadModel("./resources/models/plane.obj");
    model->scale(glm::vec3(10.0f, 1.0f, 10.0f));

    //model->getMeshes()[0]->setMaterial(material);

    models.push_back(model);

    model = loadModel("./resources/models/teapot.obj");
    model->setPosition(glm::vec3(2.5f, 0.0f, 1.0f));

    models.push_back(model);

    model = loadModel("./resources/models/crate.obj");
    model->setPosition(glm::vec3(0.0f, 0.5f, 0.0f));

    models.push_back(model);

    model = loadModel("./resources/models/Marry/Marry.obj", "Marry", "./resources/models/Marry/", "./resources/models/Marry/");
    model->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    //model->getMeshes()[0]->getMaterial()->refractionFactor = 1.0f;
    //model->getMeshes()[1]->getMaterial()->refractionFactor = 1.0f;
    models.push_back(model);

    model = loadModel("./resources/models/window.obj");
    model->setPosition(glm::vec3(0.0f, 5.0f, -5.0f));
    model->rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    //model->scale(glm::vec3(10.0f, 1.0f, 10.0f));
    //model->getMeshes()[0]->getMaterial()->refractionFactor = 1.0f;
    //models.push_back(model);

    model = loadModel("./resources/models/plane.obj");
    model->setPosition(glm::vec3(0.0f, 5.0f, -5.0f));
    model->rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    model->scale(glm::vec3(10.0f, 1.0f, 10.0f));
    //model->getMeshes()[0]->getMaterial()->refractionFactor = 1.0f;
    models.push_back(model);

    for (size_t i = 0; i < models.size(); i++) {
        models[i]->computeTangentSpace();
        models[i]->prepareDraw();
    }
}

void prepareTextures() {
    // Create the render texture
    addTexture("Error", "./resources/textures/Error.png");

    sceneTexture = std::make_shared<Texture>("sceneTexture", WindowWidth, WindowHeight, GL_LINEAR);

    gaussianBlurPassTexture = std::make_shared<Texture>("gaussianBlurPassTexture", WindowWidth, WindowHeight, GL_LINEAR);

    gaussianBlurredTexture = std::make_shared<Texture>("gaussianBlurredTexture", WindowWidth, WindowHeight, GL_LINEAR);

    brightnessTexture = std::make_shared<Texture>("brightnessTexture", WindowWidth, WindowHeight, GL_LINEAR);

    //testTexture = addTexture("Valve", "./resources/textures/Valve_original.png");

    testTexture = addTexture("Valve", "./resources/textures/Bikesgray.jpg");

    defaultAlbedo = std::make_shared<Texture>("defaultAlbedo", 1, 1);

    addCubemapTexture("CubeMap", "./resources/textures/grace", GL_CLAMP_TO_EDGE, true, true);
    //addTexture("Fieldstone", "./resources/textures/Fieldstone.tga");
    //addTexture("FieldstoneBumpDOT3", "./resources/textures/FieldstoneBumpDOT3.tga");
    //addTexture("BrickDiffuse", "./resources/textures/Brick_Diffuse.jpg");
    //addTexture("BrickNormal", "./resources/textures/Brick_Normal.jpg");
    addTexture("Coin", "./resources/textures/Coin.png");
    addTexture("CoinDot3", "./resources/textures/CoinDot3.png");
    //addTexture("CrateDiffuse", "./resources/textures/CrateDiffuse.png");
    //addTexture("CrateNormal", "./resources/textures/CrateNormal.png");
    addTexture("Projection", "./resources/textures/Kanna.jpg", GL_CLAMP_TO_BORDER);
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    glfwWindowHint(GLFW_SAMPLES, 4);

	window = glfwCreateWindow(WindowWidth, WindowHeight, "OpenGL Shading Language", nullptr, nullptr);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

    glDebugMessageControl(GL_DEBUG_SOURCE_API,
                          GL_DEBUG_TYPE_ERROR,
                          GL_DEBUG_SEVERITY_HIGH,
                          0, nullptr, GL_TRUE);

    glDebugMessageCallback(glDebugOutput, nullptr);

	bindCallbacks();

	glEnable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);

    prepareTextures();

	prepareShaderResources();
	sceneShader->printActiveAttributes();
	sceneShader->printActiveUniforms();

    camera.perspective(fov, aspect, near, far);

    loadModels();

	prepareGeometryData();

	initImGui();

	while (!glfwWindowShouldClose(window)) {
		processInput(window);

		update();

		render();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
