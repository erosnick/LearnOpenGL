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
    float density;
    glm::vec4 color;
};

uint32_t vao;
uint32_t textureId;
Shader lightCubeShader;
Shader sceneShader;

std::vector<std::shared_ptr<Model>> models;

std::map<std::string, std::shared_ptr<Texture>> textures;
std::map<std::string, std::shared_ptr<Material>> materials;

Light lights[3];

Fog fog = { 1.0f, 10.0f, 0.02f, {0.8f, 0.8f, 0.8f, 1.0f} };

float angle = 0.0f;

bool showDemoWindow = true;
bool showAnotherWindow = false;
bool showOpenMenuItem = true;
bool showProjector = false;
ImVec4 clearColor = ImVec4(0.392f, 0.584f, 0.929f, 1.0f);
ImVec4 pointLightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 directionaLightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
ImVec4 lightDirection = ImVec4(0.0f, -1.0f, -1.0f, 0.0f);

float shininess = 32.0f;

GLFWwindow* window = nullptr;

std::vector<ImFont*> fonts;

const uint32_t WindowWidth = 1280;
const uint32_t WindowHeight = 720;

float frameTime = 0.0f;

bool rightMouseButtonDown = false;
bool middleMouseButtonDown = false;

glm::vec2 lastMousePosition = { 0.0f, 0.0f };

float rotateSpeed = 3.0f;

glm::vec3 projectorPosition = glm::vec3(-2.5f, 2.5f, 0.0f);
glm::vec3 projectAt = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 projectorUp = glm::vec3(0.0f, 1.0f, 0.0f);

float fov = 60.0f;
float near = 0.2f;
float far = 200.0f;

//Camera camera(projectorPosition, projectAt);
Camera camera({ 0.0f, 1.0f, 4.5f }, { 0.0f, 1.0f, -1.0f });

glm::mat4 projectorView = glm::lookAt(projectorPosition, projectAt, projectorUp);
glm::mat4 projectorProjection = glm::perspective(glm::radians(60.0f), static_cast<float>(WindowWidth) / WindowHeight, near, far);
glm::mat4 projectorScaleTranslate = glm::mat4(1.0f);
glm::mat4 projectorTransform = glm::mat4(1.0f);

uint32_t fbo;
std::shared_ptr<Texture> renderTexture;
std::shared_ptr<Texture> defaultAlbedo;
uint32_t depthBuffer;

const std::shared_ptr<Texture>& getTexture(const std::string& name);
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
    camera.perspective(fov, static_cast<float>(width) / height, near, far);
	glViewport(0, 0, width, height);
}

void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {    
	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void onMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        rightMouseButtonDown = true;
        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);
        lastMousePosition.x = static_cast<float>(x);
        lastMousePosition.y = static_cast<float>(y);
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        rightMouseButtonDown = false;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
        middleMouseButtonDown = true;
    }

    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
        middleMouseButtonDown = false;
    }

    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

void onScrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.walk(yOffset / 8.0f);
    ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
}

void onMouseMoveCallback(GLFWwindow* window, double x, double y) {
    double dx = (lastMousePosition.x - x) * frameTime;
    double dy = (lastMousePosition.y - y) * frameTime;

    if (rightMouseButtonDown) {
        camera.yaw(static_cast<float>(dx) * rotateSpeed);
        camera.pitch(static_cast<float>(dy) * rotateSpeed);
    }

    if (middleMouseButtonDown) {
        camera.strafe(-dx / 2.0f);
        camera.raise(dy / 2.0f);
    }

    lastMousePosition.x = static_cast<float>(x);
    lastMousePosition.y = static_cast<float>(y);
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
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

void prepareFrameBufferObject() {
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Bind the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTexture->getTextureId(), 0);

    // Create the depth buffer
    glGenRenderbuffers(1, &depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTexture->getWidth(), renderTexture->getHeight());

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Bind the depth buffer to the FBO
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);

    // Set the target for the fragment shader outputs
    uint32_t drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Frame Buffer is not Complete." << std::endl;
    }

    // Unbind the framebuffer, and revert to default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void prepareShaderResources() {
    //auto vertexShader = loadShader("shaders/shader.vert", GL_VERTEX_SHADER);
    //auto fragmentShader = loadShader("shaders/shader.frag", GL_FRAGMENT_SHADER
    sceneShader.create();

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

    sceneShader.compileShaderFromFile("./shaders/rendertotexture.vert", ShaderType::VERTEX);
    sceneShader.compileShaderFromFile("./shaders/rendertotexture.frag", ShaderType::FRAGMENT);

    sceneShader.link();

    lightCubeShader.create();
    lightCubeShader.compileShaderFromFile("./shaders/color.vert", ShaderType::VERTEX);
    lightCubeShader.compileShaderFromFile("./shaders/color.frag", ShaderType::FRAGMENT);

    lightCubeShader.link();

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
    lights[0].position = { 0.0f, 1.0f, 0.5f, 1.0f};
    lights[0].intensity = 1.0f;
    lights[0].Kc = 1.0f;
    lights[0].Kl = 0.09f;
    lights[0].Kq = 0.032f;
    lights[0].type = 0;

    lights[1].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[1].position = { 0.0f, 0.0f, -1.0f, 0.0};
    lights[1].intensity = 1.0f;
    lights[1].type = 1;

    //lights[2].color = { 1.0f, 0.418f, 1.0f, 1.0f };
    lights[2].color = { 1.0f, 1.0f, 1.0f, 1.0f };
    lights[2].position = {0.0f, 3.0f, 2.0f, 2.0f };
    lights[2].direction = { 0.0f, -1.0f, 0.0f };
    lights[2].exponent = 8.0f;
    lights[2].cutoff = 30.0f;
    lights[2].outerCutoff = 45.0f;
    lights[2].intensity = 1.0f;
    lights[2].type = 2;

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
    uint32_t vbo;
    glGenBuffers(1, &vbo);

    uint32_t ibo;
    glGenBuffers(1, &ibo);

    // Create and setup the vertex array object
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
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

std::shared_ptr<Texture> addTexture(const std::string& name, const std::string& path, int32_t wrapMode = GL_REPEAT) {
    auto texture = getTexture(name);

    if (texture) {
        return texture;
    }

    if (std::filesystem::exists(path)) {
        auto texture = std::make_shared<Texture>(path, wrapMode);
        textures[name] = texture;
        return texture;
    }

    return nullptr;
}

std::shared_ptr<Texture> addCubemapTexture(const std::string& name, const std::string& path, int32_t wrapMode = GL_REPEAT, bool cubeMap = false, bool hdr = false) {
    auto texture = getTexture(name);

    if (texture) {
        return texture;
    }

    texture = std::make_shared<Texture>(path, wrapMode, cubeMap, hdr);
    if (texture) {
        textures[name] = texture;
        return texture;
    }

    return nullptr;
}

const std::shared_ptr<Texture>& getTexture(const std::string& name) {
    return textures[name];
}

void addMaterial(const std::string& name, const std::shared_ptr<Material>& material) {
    materials[name] = material;
}

const std::shared_ptr<Material>& getMaterial(const std::string& name) {
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
    if (showDemoWindow)
        ImGui::ShowDemoWindow(&showDemoWindow);

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
        ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &showAnotherWindow);

        auto commonMaterial = getMaterial("Common");

        ImGui::ColorEdit3("Ambient", (float*)&commonMaterial->Ka); // Edit 1 float using a slider from 0.1f to 1.0f
        ImGui::SliderFloat("Reflection", &commonMaterial->reflectionFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Refraction", &commonMaterial->refractionFactor, 0.0f, 1.0f);
        ImGui::SliderFloat("Shininess", &commonMaterial->shininess, 32.0f, 128.0f);
        ImGui::SliderFloat("Fog Density", &fog.density, 0.0f, 1.0f);
        ImGui::ColorEdit3("Clear color", (float*)&clearColor); // Edit 3 floats representing a color
        ImGui::ColorEdit3("Point Light Color", (float*)&lights[0].color);
        ImGui::ColorEdit3("Directional Light Color", (float*)&lights[1].color);
        ImGui::DragFloat3("Light Direction", (float*)&lights[1].position, 0.1f, -1.0f, 1.f);
        ImGui::DragFloat3("Light Position", (float*)&lights[0].position, 0.1f, 0.0f, 10.f);
        ImGui::Checkbox("Projective Texture Mapping", &showProjector);

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
    if (showAnotherWindow)
    {
        ImGui::Begin("Another Window", &showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

		ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            showAnotherWindow = false;
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
    sceneShader.use();
    sceneShader.setUniform("lights[0].color", lights[0].color);
    sceneShader.setUniform("lights[0].position", lights[0].position);
    sceneShader.setUniform("lights[0].intensity", lights[0].intensity);
    sceneShader.setUniform("lights[0].Kc", lights[0].Kc);
    sceneShader.setUniform("lights[0].Kl", lights[0].Kl);
    sceneShader.setUniform("lights[0].Kq", lights[0].Kq);
    sceneShader.setUniform("lights[0].type", lights[0].type);

    sceneShader.setUniform("lights[1].color", lights[1].color);
    sceneShader.setUniform("lights[1].position", lights[1].position);
    sceneShader.setUniform("lights[1].intensity", lights[1].intensity);
    sceneShader.setUniform("lights[1].type", lights[1].type);

    sceneShader.setUniform("lights[2].color", lights[2].color);
    sceneShader.setUniform("lights[2].position", lights[2].position);
    sceneShader.setUniform("lights[2].direction", lights[2].direction);
    sceneShader.setUniform("lights[2].exponent", lights[2].exponent);
    sceneShader.setUniform("lights[2].cutoff", glm::cos(glm::radians(lights[2].cutoff)));
    sceneShader.setUniform("lights[2].outerCutoff", glm::cos(glm::radians(lights[2].outerCutoff)));
    sceneShader.setUniform("lights[2].intensity", lights[2].intensity);
    sceneShader.setUniform("lights[2].type", lights[2].type);

    sceneShader.setUniform("fog.minDistance", fog.minDistance);
    sceneShader.setUniform("fog.maxDistance", fog.maxDistance);
    sceneShader.setUniform("fog.density", fog.density);
    sceneShader.setUniform("fog.color", fog.color);

    sceneShader.setUniform("cubeMap", getTexture("CubeMap")->getTextureIndex());

    sceneShader.setUniform("eye", camera.getEye());

    sceneShader.setUniform("projectorTransform", projectorTransform);

    sceneShader.setUniform("projection", getTexture("Projection")->getTextureIndex());
    
    sceneShader.setUniform("showProjector", showProjector);
}

void drawSkyBox(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    sceneShader.setUniform("drawSkyBox", true);

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

    sceneShader.setUniform("worldMatrix", worldMatrix);
    sceneShader.setUniform("mvpMatrix", mvpMatrix);

    glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
}

void drawLight(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    lightCubeShader.use();

    glm::mat4 worldMatrix = glm::mat4(1.0f);
    worldMatrix = glm::translate(worldMatrix, glm::vec3(lights[0].position.x, lights[0].position.y, lights[0].position.z));
    worldMatrix = glm::scale(worldMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

    glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

    lightCubeShader.setUniform("mvpMatrix", mvpMatrix);
    lightCubeShader.setUniform("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

    glBindVertexArray(vao);
    //glDrawArrays(GL_TRIANGLES, 0, 36);
    glDrawElements(GL_TRIANGLES, 64, GL_UNSIGNED_INT, 0);
}

void updateMaterialUniform(const std::shared_ptr<Material>& material) {
    if (material) {
        sceneShader.setUniform("material.Ka", material->Ka);
        sceneShader.setUniform("material.Kd", material->Kd);
        sceneShader.setUniform("material.Ks", material->Ks);
        sceneShader.setUniform("material.Ke", material->Ke);
        sceneShader.setUniform("material.shininess", material->shininess);
        sceneShader.setUniform("material.reflectionFactor", material->reflectionFactor);
        sceneShader.setUniform("material.refractionFactor", material->refractionFactor);
        sceneShader.setUniform("material.ior", material->ior);
        sceneShader.setUniform("material.eta", material->eta);
        sceneShader.setUniform("material.hasNormalMap", material->hasNormalMap);
    }
}

void drawModel(const std::shared_ptr<Model>& model, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    sceneShader.setUniform("drawSkyBox", false);

    for (auto& mesh : model->getMeshes()) {
        mesh->use();

        auto material = mesh->getMaterial();

        updateMaterialUniform(material);

        sceneShader.setUniform("textures[0]", mesh->getTextureIndex(0));
        sceneShader.setUniform("textures[1]", mesh->getTextureIndex(1));

        glm::mat4 worldMatrix = model->getTransform();
        glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

        sceneShader.setUniform("worldMatrix", worldMatrix);
        sceneShader.setUniform("mvpMatrix", mvpMatrix);

        glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
    }
}

void drawModels(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
    for (size_t i = 1; i < models.size() - 1; i++) {
        drawModel(models[i], inViewMaterix, inProjectionMatrix);
    }
}

void clear(ImVec4 color, int32_t clearFlag) {
    glClearColor(color.x, color.y, color.z, 1.0f);
    glClear(clearFlag);
}

void renderToTexture() {
    // Bind to texture's FBO
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, 512, 512); // Viewport for the texture

    clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewMatrix = camera.getViewMatrix();
    camera.perspective(fov, 1.0f, near, far);
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    drawSkyBox(viewMatrix, projectionMatrix);

    drawModels(viewMatrix, projectionMatrix);

    // Unbind texture's FBO (back to default FB)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene()
{
    updateGlobalUniform();

    renderToTexture();

    glViewport(0, 0, WindowWidth, WindowHeight);
    glm::mat4 viewMatrix = camera.getViewMatrix();
    camera.perspective(fov, static_cast<float>(WindowWidth) / WindowHeight, near, far);
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSkyBox(viewMatrix, projectionMatrix);

    drawModels(viewMatrix, projectionMatrix);

    drawModel(models[models.size() - 1], viewMatrix, projectionMatrix);

    drawLight(viewMatrix, projectionMatrix);
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
    model->addTexture(getTexture("CubeMap"));
    models.push_back(model); 
    
    model = loadModel("./resources/models/torus.obj");
    model->setPosition(glm::vec3(-2.5f, 0.0f, 1.0f));
    model->addTexture(getTexture("Fieldstone"));
    model->addTexture(getTexture("FieldstoneBumpDOT3"));
    model->setMaterial(material);

    //models.push_back(model);

    model = std::make_shared<Model>();

    GeometryGenerator geometryGenerator;
    auto sphere = geometryGenerator.CreateSphere(1, 20, 20);

    auto mesh = std::make_shared<Mesh>();

    for (size_t i = 0; i < sphere.Vertices.size(); i++) {
        Vertex vertex;
        vertex.position = sphere.Vertices[i].Position;
        vertex.normal = sphere.Vertices[i].Normal;
        vertex.texCoord = sphere.Vertices[i].TexC;
        mesh->addVertex(vertex);
    }

    for (size_t i = 0; i < sphere.Indices32.size(); i++) {
        mesh->addIndex(sphere.Indices32[i]);
    }

    model->setMaterial(material);

    model->addTexture(getTexture("BrickDiffuse"));
    model->addTexture(getTexture("BrickNormal"));
    model->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
    model->setMaterial(material);

    //models.push_back(model);

    model = loadModel("./resources/models/plane.obj");
    model->addTexture(getTexture("Fieldstone"));
    model->addTexture(getTexture("FieldstoneBumpDOT3"));
    model->scale(glm::vec3(10.0f, 1.0f, 10.0f));

    auto planeMaterial = std::make_shared<Material>();

    planeMaterial->Ka = material->Ka;
    planeMaterial->Kd = material->Ka;
    planeMaterial->Ks = material->Ks;
    planeMaterial->shininess = material->shininess;
    planeMaterial->reflectionFactor = material->reflectionFactor;
    planeMaterial->refractionFactor = material->refractionFactor;

    model->setMaterial(planeMaterial);

    models.push_back(model);

    model = loadModel("./resources/models/teapot.obj");
    model->addTexture(getTexture("Fieldstone"));
    model->addTexture(getTexture("FieldstoneBumpDOT3"));
    model->setPosition(glm::vec3(2.5f, 0.0f, 1.0f));
    model->setMaterial(material);

    //models.push_back(model);

    model = loadModel("./resources/models/crate.obj");
    //model->addTexture(getTexture("CrateDiffuse"));
    //model->addTexture(renderTexture);
    //model->addTexture(getTexture("CrateNormal"));
    model->setPosition(glm::vec3(0.0f, 1.0f, 2.0f));
    model->setMaterial(material);

    //models.push_back(model);

    model = loadModel("./resources/models/plane.obj");
    //model->addTexture(getTexture("CrateDiffuse"));
    model->addTexture(renderTexture);
    model->addTexture(getTexture("CrateNormal"));
    model->setPosition(glm::vec3(0.0f, 5.0f, -5.0f));
    model->rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    model->setMaterial(material);
    model->scale(glm::vec3(10.0f, 1.0f, 10.0f));
    models.push_back(model);

    model = loadModel("./resources/models/Marry/Marry.obj", "Marry", "./resources/models/Marry/", "./resources/models/Marry/");
    models.push_back(model);

    for (size_t i = 0; i < models.size(); i++) {
        models[i]->computeTangentSpace();
        models[i]->prepareDraw();
    }
}

void prepareTextures() {
    // Create the render texture
    addTexture("Error", "./resources/textures/Error.png");

    renderTexture = std::make_shared<Texture>("renderTexture", 512, 512);

    defaultAlbedo = std::make_shared<Texture>("defaultAlbedo", 1, 1);

    addCubemapTexture("CubeMap", "./resources/textures/grace", GL_CLAMP_TO_EDGE, true, true);
    //addTexture("Fieldstone", "./resources/textures/Fieldstone.tga");
    //addTexture("FieldstoneBumpDOT3", "./resources/textures/FieldstoneBumpDOT3.tga");
    //addTexture("BrickDiffuse", "./resources/textures/Brick_Diffuse.jpg");
    //addTexture("BrickNormal", "./resources/textures/Brick_Normal.jpg");
    addTexture("Coin", "./resources/textures/Coin.png");
    addTexture("CoinDot3", "./resources/textures/CoinDot.png");
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

    prepareTextures();

	prepareShaderResources();
	sceneShader.printActiveAttributes();
	sceneShader.printActiveUniforms();

    camera.perspective(60.0f, static_cast<float>(WindowWidth) / WindowHeight, near, far);

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
