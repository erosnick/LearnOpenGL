#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include <CL\cl.hpp>
#include "linear_algebra.h"
#include "camera.h"
#include "geometry.h"
//#include "user_interaction.h"

using namespace std;
using namespace cl;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

bool buffer_reset;
InteractiveCamera* interactiveCamera;

const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"void main()\n"
"{\n"
"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"}\0";
const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}\n\0";

const int sphere_count = 4;

// OpenGL vertex buffer object
GLuint vbo;

// OpenCL objects
Device device;
CommandQueue queue;
Kernel kernel;
Context context;
Program program;
Buffer cl_output;
Buffer cl_spheres;
Buffer cl_camera;
Buffer cl_accumbuffer;
BufferGL cl_vbo;
vector<Memory> cl_vbos;

// image buffer (not needed with real-time viewport)
// cl_float4* cpu_output;
cl_int err;
unsigned int framenumber = 0;

Camera* hostRendercam = NULL;
Sphere cpu_spheres[sphere_count];

double lastX = 0;
double lastY = 0;

int mouseButton = -1;

void drawTwoLineWithArray()
{
	GLint vertices[] = { 25,25,
					  100,100,
					  120,120,
					  200,200 };
	GLfloat colors[] = { 1.0, 0.0, 0.0,
					  1.0, 0.0, 0.0,
					  0.0, 1.0, 0.0,
					  0.0, 1.0, 0.0 };
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_INT, 0, vertices);
	glColorPointer(3, GL_FLOAT, 0, colors);
	glBegin(GL_LINES);
	glArrayElement(0);
	glArrayElement(1);
	glArrayElement(2);
	glArrayElement(3);
	glEnd();
}

void createVBO(GLuint* vbo)
{
	//create vertex buffer object
	glGenBuffers(1, vbo);
	glBindBuffer(GL_ARRAY_BUFFER, *vbo);

	//initialise VBO
	unsigned int size = WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(cl_float3);
	glBufferData(GL_ARRAY_BUFFER, size, 0, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void drawGL() {

	//clear all pixels, then render from the vbo
	glClear(GL_COLOR_BUFFER_BIT);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexPointer(2, GL_FLOAT, 16, 0); // size (2, 3 or 4), type, stride, pointer
	glColorPointer(4, GL_UNSIGNED_BYTE, 16, (GLvoid*)8); // size (3 or 4), type, stride, pointer

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glDrawArrays(GL_POINTS, 0, WINDOW_WIDTH * WINDOW_HEIGHT);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}

void pickPlatform(Platform& platform, const vector<Platform>& platforms) {

	if (platforms.size() == 1) platform = platforms[0];
	else {
		int input = 1;
		cout << "\nChoose an OpenCL platform: ";
		//cin >> input;

		// handle incorrect user input
		while (input < 1 || input > platforms.size()) {
			cin.clear(); //clear errors/bad flags on cin
			cin.ignore(cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			cout << "No such option. Choose an OpenCL platform: ";
			cin >> input;
		}
		platform = platforms[input - 1];
	}
}

void pickDevice(Device& device, const vector<Device>& devices) {

	if (devices.size() == 1) device = devices[0];
	else {
		int input = 0;
		cout << "\nChoose an OpenCL device: ";
		cin >> input;

		// handle incorrect user input
		while (input < 1 || input > devices.size()) {
			cin.clear(); //clear errors/bad flags on cin
			cin.ignore(cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			cout << "No such option. Choose an OpenCL device: ";
			cin >> input;
		}
		device = devices[input - 1];
	}
}

void printErrorLog(const Program& program, const Device& device) {

	// Get the error log and print to console
	string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
	cerr << "Build log:" << std::endl << buildlog << std::endl;

	// Print the error log to a file
	FILE* log;

	fopen_s(&log, "errorlog.txt", "w");
	fprintf(log, "%s\n", buildlog.c_str());
	cout << "Error log saved in 'errorlog.txt'" << endl;
	system("PAUSE");
	exit(1);
}

void initOpenCL()
{
	// Get all available OpenCL platforms (e.g. AMD OpenCL, Nvidia CUDA, Intel OpenCL)
	vector<Platform> platforms;
	Platform::get(&platforms);
	cout << "Available OpenCL platforms : " << endl << endl;
	for (int i = 0; i < platforms.size(); i++)
		cout << "\t" << i + 1 << ": " << platforms[i].getInfo<CL_PLATFORM_NAME>() << endl;

	// Pick one platform
	Platform platform;
	pickPlatform(platform, platforms);
	cout << "\nUsing OpenCL platform: \t" << platform.getInfo<CL_PLATFORM_NAME>() << endl;

	// Get available OpenCL devices on platform
	vector<Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	cout << "Available OpenCL devices on this platform: " << endl << endl;
	for (int i = 0; i < devices.size(); i++) {
		cout << "\t" << i + 1 << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << endl;
		cout << "\t\tMax compute units: " << devices[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
		cout << "\t\tMax work group size: " << devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl << endl;
	}

	// Pick one device
	//Device device;
	pickDevice(device, devices);
	cout << "\nUsing OpenCL device: \t" << device.getInfo<CL_DEVICE_NAME>() << endl;
	cout << "\t\t\tMax compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
	cout << "\t\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;

	// Create an OpenCL context on that device.
	// Windows specific OpenCL-OpenGL interop
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		0
	};

	context = Context(device, properties);

	// Create a command queue
	queue = CommandQueue(context, device);


	// Convert the OpenCL source code to a string// Convert the OpenCL source code to a string
	string source;
	ifstream file("opencl_kernel.cl");
	if (!file) {
		cout << "\nNo OpenCL file found!" << endl << "Exiting..." << endl;
		system("PAUSE");
		exit(1);
	}
	while (!file.eof()) {
		char line[256];
		file.getline(line, 255);
		source += line;
	}

	const char* kernel_source = source.c_str();

	// Create an OpenCL program with source
	program = Program(context, kernel_source);

	// Build the program for the selected device 
	cl_int result = program.build({ device }); // "-cl-fast-relaxed-math"
	if (result) cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);
}

// #define float3(x, y, z) {{x, y, z}}  // macro to replace ugly initializer braces

void initScene(Sphere* cpu_spheres) {

	// floor
	cpu_spheres[0].radius = 200.0f;
	cpu_spheres[0].position = Vector3Df(0.0f, -200.4f, 0.0f);
	cpu_spheres[0].color = Vector3Df(0.9f, 0.3f, 0.0f);
	cpu_spheres[0].emission = Vector3Df(0.0f, 0.0f, 0.0f);

	// left sphere
	cpu_spheres[1].radius = 0.16f;
	cpu_spheres[1].position = Vector3Df(-0.25f, -0.24f, -0.1f);
	cpu_spheres[1].color = Vector3Df(0.9f, 0.8f, 0.7f);
	cpu_spheres[1].emission = Vector3Df(0.0f, 0.0f, 0.0f);

	// right sphere
	cpu_spheres[2].radius = 0.16f;
	cpu_spheres[2].position = Vector3Df(0.25f, -0.24f, 0.1f);
	cpu_spheres[2].color = Vector3Df(0.9f, 0.8f, 0.7f);
	cpu_spheres[2].emission = Vector3Df(0.0f, 0.0f, 0.0f);

	float lightIntensity = 2.0f;

	// lightsource
	cpu_spheres[3].radius = 1.0f;
	cpu_spheres[3].position = Vector3Df(0.0f, 1.36f, 0.0f);
	cpu_spheres[3].color = Vector3Df(0.0f, 0.0f, 0.0f);
	cpu_spheres[3].emission = Vector3Df(0.9f, 0.8f, 0.6f) * lightIntensity;
}

// hash function to calculate new seed for each frame
// see http://www.reedbeta.com/blog/2013/01/12/quick-and-easy-gpu-random-numbers-in-d3d11/

unsigned int WangHash(unsigned int a) {
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return a;
}


void initCLKernel() {

	// pick a rendermode
	unsigned int rendermode = 1;

	// Create a kernel (entry point in the OpenCL source program)
	kernel = Kernel(program, "render_kernel");

	// specify OpenCL kernel arguments
	//kernel.setArg(0, cl_output);
	kernel.setArg(0, cl_spheres);
	kernel.setArg(1, WINDOW_WIDTH);
	kernel.setArg(2, WINDOW_HEIGHT);
	kernel.setArg(3, sphere_count);
	kernel.setArg(4, cl_vbo);
	kernel.setArg(5, framenumber);
	kernel.setArg(6, cl_camera);
	kernel.setArg(7, rand());
	kernel.setArg(8, rand());
	kernel.setArg(9, cl_accumbuffer);
	kernel.setArg(10, WangHash(framenumber));
}

void runKernel() {
	// every pixel in the image has its own thread or "work item",
	// so the total amount of work items equals the number of pixels
	std::size_t global_work_size = WINDOW_WIDTH * WINDOW_HEIGHT;
	std::size_t local_work_size = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);;

	// Ensure the global work size is a multiple of local work size
	if (global_work_size % local_work_size != 0)
		global_work_size = (global_work_size / local_work_size + 1) * local_work_size;

	//Make sure OpenGL is done using the VBOs
	glFinish();

	//this passes in the vector of VBO buffer objects 
	queue.enqueueAcquireGLObjects(&cl_vbos);
	queue.finish();

	// launch the kernel
	queue.enqueueNDRangeKernel(kernel, NULL, global_work_size, local_work_size); // local_work_size
	queue.finish();

	//Release the VBOs so OpenGL can play with them
	queue.enqueueReleaseGLObjects(&cl_vbos);
	queue.finish();
}


void render() {

	//cpu_spheres[1].position.y += 0.01f;
	queue.enqueueWriteBuffer(cl_spheres, CL_TRUE, 0, sphere_count * sizeof(Sphere), cpu_spheres);

	if (buffer_reset) {
		float arg = 0;
		queue.enqueueFillBuffer(cl_accumbuffer, arg, 0, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(cl_float3));
		framenumber = 0;
	}

	buffer_reset = false;
	framenumber++;

	// build a new camera for each frame on the CPU
	interactiveCamera->buildRenderCamera(hostRendercam);
	// copy the host camera to a OpenCL camera
	queue.enqueueWriteBuffer(cl_camera, CL_TRUE, 0, sizeof(Camera), hostRendercam);
	queue.finish();

	kernel.setArg(0, cl_spheres);  //  works even when commented out
	kernel.setArg(5, framenumber);
	kernel.setArg(6, cl_camera);
	kernel.setArg(7, rand());
	kernel.setArg(8, rand());
	kernel.setArg(10, WangHash(framenumber));

	runKernel();

	drawGL();
}

// initialise camera on the CPU
void initCamera()
{
	delete interactiveCamera;
	interactiveCamera = new InteractiveCamera();

	interactiveCamera->setResolution(WINDOW_WIDTH, WINDOW_HEIGHT);
	interactiveCamera->setFOVX(45);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key) {

		case(GLFW_KEY_ESCAPE): exit(0);
		case(GLFW_KEY_SPACE): initCamera(); buffer_reset = true; break;
		case(GLFW_KEY_A): interactiveCamera->strafe(-0.05f); buffer_reset = true; break;
		case(GLFW_KEY_D): interactiveCamera->strafe(0.05f); buffer_reset = true; break;
		case(GLFW_KEY_R): interactiveCamera->changeAltitude(0.05f); buffer_reset = true; break;
		case(GLFW_KEY_F): interactiveCamera->changeAltitude(-0.05f); buffer_reset = true; break;
		case(GLFW_KEY_W): interactiveCamera->goForward(0.05f); buffer_reset = true; break;
		case(GLFW_KEY_S): interactiveCamera->goForward(-0.05f); buffer_reset = true; break;
		case(GLFW_KEY_G): interactiveCamera->changeApertureDiameter(0.1); buffer_reset = true; break;
		case(GLFW_KEY_H): interactiveCamera->changeApertureDiameter(-0.1); buffer_reset = true; break;
		case(GLFW_KEY_T): interactiveCamera->changeFocalDistance(0.1); buffer_reset = true; break;
		case(GLFW_KEY_Y): interactiveCamera->changeFocalDistance(-0.1); buffer_reset = true; break;

		case GLFW_KEY_LEFT: interactiveCamera->changeYaw(0.02f); buffer_reset = true; break;
		case GLFW_KEY_RIGHT: interactiveCamera->changeYaw(-0.02f); buffer_reset = true; break;
		case GLFW_KEY_UP: interactiveCamera->changePitch(0.02f); buffer_reset = true; break;
		case GLFW_KEY_DOWN: interactiveCamera->changePitch(-0.02f); buffer_reset = true; break;
	}
}

void mouseMoveCallback(GLFWwindow* window, double xPos, double yPos)
{
	int deltaX = lastX - xPos;
	int deltaY = lastY - yPos;

	if (deltaX != 0 || deltaY != 0) {

		if (mouseButton == GLFW_MOUSE_BUTTON_1)  // Rotate
		{
			interactiveCamera->changeYaw(deltaX * 0.01);
			interactiveCamera->changePitch(-deltaY * 0.01);
		}
		else if (mouseButton == GLFW_MOUSE_BUTTON_2) // Zoom
		{
			interactiveCamera->changeAltitude(-deltaY * 0.01);
		}

		if (mouseButton == GLFW_MOUSE_BUTTON_3) // camera move
		{
			interactiveCamera->changeRadius(-deltaY * 0.01);
		}

		lastX = xPos;
		lastY = yPos;
		
		if (mouseButton != -1)
		{
			buffer_reset = true;
		}
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_1)
	{
		mouseButton = GLFW_MOUSE_BUTTON_1;
	}

	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_1)
	{
		mouseButton = -1;
	}

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_2)
	{
		mouseButton = GLFW_MOUSE_BUTTON_2;
	}

	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_2)
	{
		mouseButton = -1;
	}

	if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_3)
	{
		mouseButton = GLFW_MOUSE_BUTTON_3;
	}

	if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_3)
	{
		mouseButton = -1;
	}

	glfwGetCursorPos(window, &lastX, &lastY);

	mouseMoveCallback(window, lastX, lastY);
}

void cleanUp() {
	//	delete cpu_output;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, mouseMoveCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glOrtho(0.0f, WINDOW_WIDTH, 0.0f, WINDOW_HEIGHT, 0.0f, 1.0f);

	// initialise OpenCL
	initOpenCL();

	// create vertex buffer object
	createVBO(&vbo);

	//make sure OpenGL is finished before we proceed
	glFinish();

	// initialise scene
	initScene(cpu_spheres);

	cl_spheres = Buffer(context, CL_MEM_READ_ONLY, sphere_count * sizeof(Sphere));
	queue.enqueueWriteBuffer(cl_spheres, CL_TRUE, 0, sphere_count * sizeof(Sphere), cpu_spheres);

	// initialise an interactive camera on the CPU side
	initCamera();
	// create a CPU camera
	hostRendercam = new Camera();
	interactiveCamera->buildRenderCamera(hostRendercam);

	cl_camera = Buffer(context, CL_MEM_READ_ONLY, sizeof(Camera));
	queue.enqueueWriteBuffer(cl_camera, CL_TRUE, 0, sizeof(Camera), hostRendercam);

	// create OpenCL buffer from OpenGL vertex buffer object
	cl_vbo = BufferGL(context, CL_MEM_WRITE_ONLY, vbo);
	cl_vbos.push_back(cl_vbo);

	// reserve memory buffer on OpenCL device to hold image buffer for accumulated samples
	cl_accumbuffer = Buffer(context, CL_MEM_WRITE_ONLY, WINDOW_WIDTH * WINDOW_HEIGHT * sizeof(cl_float3));

	// intitialise the kernel
	initCLKernel();

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //// draw our first triangle
        //glUseProgram(shaderProgram);
        //glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        //// glBindVertexArray(0); // no need to unbind it every time 

        //drawTwoLineWithArray();
		render();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}