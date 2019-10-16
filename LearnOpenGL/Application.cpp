#include "Application.h"
#include <iostream>
#include <algorithm>
#include <functional>
#include <vector>

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Application::Application(int inWidth, int inHeight)
{
	width = inWidth;
	height = inHeight;

	aspectRatio = (float)width / (float)height;

	alpha = 0.5f;

	rotateAngle = 0.0f;

	lastX = width / 2.0f;
	lastY = height / 2.0f;

	cameraControl = false;
	cameraPan = false;

	camera.setFOV(glm::radians(45.0f));
	camera.setAspectRatio(aspectRatio);
	camera.setPosition(glm::vec3(0.0f, 0.0f, 5.0f));
	camera.lookAt(glm::vec3(0.0f, 0.0f, 0.0f));

	initialize();

	appWindow = createWindow(width, height);

	if (appWindow == nullptr)
	{
		std::cout << "Failed to create GLFW window." << std::endl;
		glfwTerminate();
	}

	//glfwSetInputMode(appWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwMakeContextCurrent(appWindow);

	if (!loadGLLoader())
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	initImGui();

	CheckMaximumVertexAttributesSupport();

	bindCallbacks();

	initVBO();

	prepareResources();
}

void Application::run()
{
	while (!glfwWindowShouldClose(appWindow))
	{
		float realTime = glfwGetTime();

		while (simulationTime < realTime)
		{
			simulationTime += timeSlice;
			update(timeSlice);
		}

		render();

		renderImGui();


		glfwSwapBuffers(appWindow);

		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(appWindow);
	glfwTerminate();
}

void Application::renderImGui()
{
	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	buildImGuiWidgets();

	// Rendering
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::update(float delta)
{
	processInput(appWindow);
}

void Application::render()
{
	glEnable(GL_DEPTH_TEST);
	glClearColor(clearColor.x, clearColor.y, clearColor.z, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//shader.use();

	// 这里的glBindTexture可以用来切换不同的texture。
	//texture.use();
	//anotherTexture.use();

	double radius = 5.0f;

	double cameraX = sin(glfwGetTime()) * radius;
	double cameraY = cos(glfwGetTime()) * radius;

	glm::mat4 rotation = glm::rotate(glm::radians(rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

	basicShader.use();

	//basicShader.setMat4("model", rotation);
	basicShader.setMat4("model", cubeModel.modelMatrix());
	basicShader.setMat4("view", camera.viewMatrix());
	basicShader.setMat4("projection", camera.projectionMatrix());

	basicShader.setFloat("ambientStrength", ambientStrength);
	basicShader.setVec3("lightPosition", lightPosition.x, lightPosition.y, lightPosition.z);
	basicShader.setVec3("objectColor", 1.0f, 0.5f, 0.31f);
	basicShader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

	cubeModel.preDraw();
	//glDrawElements(GL_TRIANGLES, cubeModel.verticesCount(), GL_UNSIGNED_INT, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glDrawArrays(GL_POINTS, 0, 36);
	glBindVertexArray(0);

	lightShader.use();

	lightModel.setPosition(glm::vec3(lightPosition.x, lightPosition.y, lightPosition.z));

	lightShader.setMat4("model", lightModel.modelMatrix());
	lightShader.setMat4("view", camera.viewMatrix());
	lightShader.setMat4("projection", camera.projectionMatrix());

	lightShader.setVec3("lightColor", lightColor.x, lightColor.y, lightColor.z);

	lightModel.preDraw();
	glDrawElements(GL_TRIANGLES, lightModel.verticesCount(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Application::initImGui()
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
	const char* glsl_version = "#version 130";
	ImGui_ImplGlfw_InitForOpenGL(appWindow, false);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	fonts.push_back(io.Fonts->AddFontFromFileTTF("./imgui/misc/fonts/Roboto-Medium.ttf", 16.0f));
	fonts.push_back(io.Fonts->AddFontFromFileTTF("./imgui/misc/fonts/Cousine-Regular.ttf", 15.0f));
	fonts.push_back(io.Fonts->AddFontFromFileTTF("./imgui/misc/fonts/DroidSans.ttf", 16.0f));
	fonts.push_back(io.Fonts->AddFontFromFileTTF("./imgui/misc/fonts/ProggyTiny.ttf", 10.0f));
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}

void Application::buildImGuiWidgets()
{
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (showDemoWindow)
		ImGui::ShowDemoWindow(&showDemoWindow);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::PushFont(fonts[2]);
		ImGui::Text("This is some useful text use another font.");
		ImGui::PopFont();
		ImGui::Checkbox("Demo Window", &showDemoWindow);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &showAnotherWindow);

		ImGui::SliderFloat("Ambient Strength", &ambientStrength, 0.1f, 1.0f); // Edit 1 float using a slider from 0.1f to 1.0f
		ImGui::ColorEdit3("Clear color", (float*)&clearColor); // Edit 3 floats representing a color
		ImGui::ColorEdit3("Light Color", (float*)&lightColor);
		ImGui::DragFloat3("Light Position", (float*)&lightPosition, 0.1f, 0.0f, 10.f);

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
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

void Application::initialize()
{
	// Initialize GLFW. 
	// Use OpenGL3.3, Core-Profile.
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* Application::createWindow(int Width, int Height)
{
	GLFWwindow* window = glfwCreateWindow(Width, Height, "LearnOpenGL", nullptr, nullptr);

	return window;
}

void Application::bindCallbacks()
{
	glfwSetWindowUserPointer(appWindow, this);

	auto resizeBuffer = [](GLFWwindow* window, int inWidth, int inHeight)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalResizeBufferCallback(window, inWidth, inHeight);
	};

	glfwSetFramebufferSizeCallback(appWindow, resizeBuffer);

	auto keyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalKeyCallback(window, key, scancode, action, mods);
	};

	glfwSetKeyCallback(appWindow, keyCallback);

	auto mouseMoveCallback = [](GLFWwindow* window, double xpos, double ypos)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalMouseMoveCallback(window, xpos, ypos);
	};

	glfwSetCursorPosCallback(appWindow, mouseMoveCallback);

	auto mouseButtonCallback = [](GLFWwindow* window, int button, int action, int mods)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalMouseButtonCallback(window, button, action, mods);
	};

	glfwSetMouseButtonCallback(appWindow, mouseButtonCallback);

	auto scrollCallback = [](GLFWwindow* window, double xOffset, double yOffset)
	{
		static_cast<Application*>(glfwGetWindowUserPointer(window))->internalScrollCallback(window, xOffset, yOffset);
	};

	glfwSetScrollCallback(appWindow, scrollCallback);
}

bool Application::loadGLLoader()
{
	return gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

void Application::initVBO()
{
	// 三角形
	float vertices1[] = {
		 0.0f, -0.5f, 0.0f,
		 0.5f,  0.5f, 0.5f,
		-0.5f,  0.5f, 0.0f
	};

	unsigned int indices1[] = {
		0, 1, 2
	};

	// 三角形-顶点颜色
	float vertices2[] = {
		-0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f,
		 0.5f, -0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
		 0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
	};

	unsigned int indices2[] = {
		0, 1, 2
	};

	// 三角形-纹理
	std::vector<Vertex> vertices = {
		Vertex( 0.5f,  0.5f, 0.0f, 1.0f, 1.0f),  // 右上角
		Vertex( 0.5f, -0.5f, 0.0f, 1.0f, 0.0f),  // 右下角
	    Vertex(-0.5f, -0.5f, 0.0f, 0.0f, 0.0f),  // 左下角
	    Vertex(-0.5f,  0.5f, 0.0f, 0.0f, 1.0f)   // 左上角
	};

	unsigned int indices[] = { // 注意索引从0开始! 
		0, 1, 3, // 第一个三角形
		1, 2, 3  // 第二个三角形
	};

	std::vector<Vertex> cube = {
			   Vertex(-0.5f,  0.5f,  0.5f, 0.0f, 1.0f),   //正面左上0
			   Vertex(-0.5f, -0.5f,  0.5f, 0.0f, 0.0f),   //正面左下1
			   Vertex( 0.5f, -0.5f,  0.5f, 1.0f, 0.0f),   //正面右下2
			   Vertex( 0.5f,  0.5f,  0.5f, 1.0f, 1.0f),   //正面右上3
			   Vertex(-0.5f,  0.5f, -0.5f, 0.0f, 1.0f),   //反面左上4
			   Vertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f),   //反面左下5
			   Vertex( 0.5f, -0.5f, -0.5f, 1.0f, 0.0f),   //反面右下6
			   Vertex( 0.5f,  0.5f, -0.5f, 1.0f, 1.0f)    //反面右上7
	};

	std::vector<Vertex> lightCube = {
			   Vertex(-0.5f,  0.5f,  0.5f, 0.0f, 1.0f),   //正面左上0
			   Vertex(-0.5f, -0.5f,  0.5f, 0.0f, 0.0f),   //正面左下1
			   Vertex( 0.5f, -0.5f,  0.5f, 1.0f, 0.0f),   //正面右下2
			   Vertex( 0.5f,  0.5f,  0.5f, 1.0f, 1.0f),   //正面右上3
			   Vertex(-0.5f,  0.5f, -0.5f, 0.0f, 1.0f),   //反面左上4
			   Vertex(-0.5f, -0.5f, -0.5f, 0.0f, 0.0f),   //反面左下5
			   Vertex( 0.5f, -0.5f, -0.5f, 1.0f, 0.0f),   //反面右下6
			   Vertex( 0.5f,  0.5f, -0.5f, 1.0f, 1.0f)    //反面右上7
	};

	std::vector<unsigned int> cubeIndices = {
				0, 3, 2, 0, 2, 1,    //正面
				0, 1, 5, 0, 5, 4,    //左面
				0, 7, 3, 0, 4, 7,    //上面
				6, 7, 4, 6, 4, 5,    //后面
				6, 3, 7, 6, 2, 3,    //右面
				6, 5, 1, 6, 1, 2     //下面
	};

	std::vector<VertexNormal> normalVertices = {

	 VertexNormal(-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f),
	 VertexNormal( 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f),
	 VertexNormal( 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f),
	 VertexNormal( 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f),
	 VertexNormal(-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f),
	 VertexNormal(-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f),

	 VertexNormal(-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f),
	 VertexNormal( 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f),
	 VertexNormal( 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f),
	 VertexNormal( 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f),
	 VertexNormal(-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f),
	 VertexNormal(-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f),

	 VertexNormal(-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f),
	 VertexNormal(-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f),
	 VertexNormal(-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f),
	 VertexNormal(-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f),
	 VertexNormal(-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f),
	 VertexNormal(-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f),

	 VertexNormal( 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f),
	 VertexNormal( 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f),
	 VertexNormal( 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f),
	 VertexNormal( 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f),
	 VertexNormal( 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f),
	 VertexNormal( 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f),

	 VertexNormal(-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f),
	 VertexNormal( 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f),
	 VertexNormal( 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f),
	 VertexNormal( 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f),
	 VertexNormal(-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f),
	 VertexNormal(-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f),

	 VertexNormal(-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f),
	 VertexNormal( 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f),
	 VertexNormal( 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f),
	 VertexNormal( 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f),
	 VertexNormal(-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f),
	 VertexNormal(-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f)

	};

	cubeModel.initialize();
	//cubeModel.loadData(cube, cubeIndices);
	cubeModel.loadData(normalVertices);
	cubeModel.setPosition(glm::vec3(-1.0f, 0.0f, 0.0f));

	lightModel.initialize();
	lightModel.loadData(cube, cubeIndices);
	lightModel.setScale(glm::vec3(0.5f));

	unsigned int VBO[5];
	unsigned int IBO[5];

	glGenBuffers(5, VBO);
	glGenBuffers(5, IBO);

	glGenVertexArrays(5, VAO);

	// Initialize first VAO
	glBindVertexArray(VAO[0]);

	// Bind vertex data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);

	// Bind index data.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);

	// Location attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)0);
	glEnableVertexAttribArray(0);

	// Color attribute.
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	// Initialize second VAO
	glBindVertexArray(VAO[1]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
	glEnableVertexAttribArray(0);

	// Initialize third VAO
	glBindVertexArray(VAO[2]);

	glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO[2]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
	glEnableVertexAttribArray(0);

	// TexCoord attribute.
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);
}

void Application::prepareResources()
{
	basicShader.load("shaders/Basic.vs", "shaders/Basic.fs");

	basicShader.use();

	lightShader.load("shaders/Light.vs", "shaders/Light.fs");

	lightShader.use();

	textureShader.load("shaders/Texture.vs", "shaders/Texture.fs");

	// don't forget to activate/use the shader before setting uniforms!
	textureShader.use();

	texture.load("resources/rexie01.jpg", GL_TEXTURE0);

	textureShader.setInt("ourTexture", 0);

	anotherTexture.load("resources/rexie02.jpg", GL_TEXTURE1);

	textureShader.setInt("anotherTexture", 1);
}

void Application::processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera.forward(timeSlice);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera.forward(-timeSlice);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera.right(-timeSlice);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera.right(timeSlice);
	}

	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
	{
		camera.up(timeSlice);
	}

	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
	{
		camera.up(-timeSlice);
	}

	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		//camera.setPitch(-10.0f);
	}

	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
	{
		//camera.setPitch(10.0f);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
	{
		//camera.setYaw(10.0f);
	}

	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
	{
		//camera.setYaw(-10.0f);
	}
}

void Application::internalResizeBufferCallback(GLFWwindow* window, int inWidth, int inHeight)
{
	width = inWidth;
	height = inHeight;

	aspectRatio = (float)width / (float)height;

	camera.setAspectRatio(aspectRatio);

	glViewport(0, 0, width, height);
}

void Application::CheckMaximumVertexAttributesSupport()
{
	int numAttributes;

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);

	cout << "Maximum number of vertex attributes supported: " << numAttributes << endl;
}

void Application::internalKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		polygonMode = GL_FILL;
	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		polygonMode = GL_LINE;
	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		polygonMode = GL_POINT;
	}

	glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		alpha += 0.1f;
		alpha = std::min(1.0f, alpha);
		camera.setPitch(-10.0f);
	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		alpha -= 0.1f;
		alpha = std::max(0.0f, alpha);
		camera.setPitch(10.0f);
	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		//rotateAngle += 1.0f;
		//rotateAngle = std::max(0.0f, rotateAngle);
		camera.setYaw(-10.0f);
	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		//rotateAngle -= 1.0f;
		//rotateAngle = std::min(360.0f, rotateAngle);
		camera.setYaw(10.0f);
	}

	ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
}

void Application::internalMouseMoveCallback(GLFWwindow* window, double xPos, double yPos)
{
	double xOffset = xPos - lastX;
	double yOffset = lastY - yPos; // reversed since y - coordinates go from bottom to top

	lastX = xPos;
	lastY = yPos;

	if (cameraControl)
	{
		camera.setYaw(-xOffset);
		camera.setPitch(yOffset);
	}

	if (cameraPan)
	{
		camera.right(xOffset);
		camera.up(yOffset);
	}
}

void Application::internalMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// 0 - 鼠标左键
	// 1 - 鼠标右键
	// 2 - 鼠标中键
	if (action == GLFW_PRESS && button == GLFW_KEY_RIGHT)
	{
		cameraControl = true;
	}

	if (action == GLFW_RELEASE &&        button == 1)
	{
		cameraControl = false;
	}

	if (action == GLFW_PRESS && button == 2)
	{                           
		cameraPan = true;            
	}        

	if (action == GLFW_RELEASE && button == 2)
	{
		cameraPan = false;
	}

	glfwGetCursorPos(window, &lastX, &lastY);

	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

void Application::internalScrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
	if (fov >= 1.0f && fov <= 90.0f)
	{
		fov -= yOffset;
	}

	if (fov <= 1.0f)
	{
		fov = 1.0f;
	}

	if (fov >= 90.0f)
	{
		fov = 90.0f;
	}

	camera.setFOV(glm::radians(fov));

	ImGui_ImplGlfw_ScrollCallback(window, xOffset, yOffset);
}
