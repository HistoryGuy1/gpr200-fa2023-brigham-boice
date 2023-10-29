#include <stdio.h>
#include <math.h>
#include <cmath>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/procGen.h>
#include <ew/transform.h>
#include <bb/camera.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void moveCamera(GLFWwindow* window, myLib::Camera* camera, myLib::CameraControls* controls, float deltaTime);
float DegreesToRads(float degrees);

//Projection will account for aspect ratio!
const int SCREEN_WIDTH = 1080;
const int SCREEN_HEIGHT = 720;

const int NUM_CUBES = 4;
ew::Transform cubeTransforms[NUM_CUBES];

ew::Vec3 position = ew::Vec3(0, 0, 5);
ew::Vec3 target = ew::Vec3(0, 0, 0);
bool orthographic = false;
float orthoSize = 6.0f;
float fov = 60.0f;
float nearPlane = 0.1f;
float farPlane = 100.0f;

int main() {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Camera", NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return 1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return 1;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Depth testing - required for depth sorting!
	glEnable(GL_DEPTH_TEST);

	ew::Shader shader("assets/vertexShader.vert", "assets/fragmentShader.frag");
	
	//Cube mesh
	ew::Mesh cubeMesh(ew::createCube(0.5f));
	myLib::Camera cam{
		cam.position = position,
		cam.target = target,
		cam.orthographic = orthographic,
		cam.orthoSize = orthoSize,
		cam.fov = fov,
		cam.nearPlane = nearPlane,
		cam.farPlane = farPlane
	};

	myLib::CameraControls cameraControls;

	//Cube positions
	for (size_t i = 0; i < NUM_CUBES; i++)
	{
		cubeTransforms[i].position.x = i % (NUM_CUBES / 2) - 0.5;
		cubeTransforms[i].position.y = i / (NUM_CUBES / 2) - 0.5;
	}

	float prevTime = 0;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
		//Clear both color buffer AND depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Calculate deltaTime
		float time = (float)glfwGetTime(); //Timestamp of current frame
		float deltaTime = time - prevTime;
		prevTime = time;

		moveCamera(window, &cam, &cameraControls, deltaTime);

		//Set uniforms
		shader.use();

		position = cam.position;
		target = cam.target;
		cam.orthographic = orthographic;
		cam.fov = fov;
		cam.orthoSize = orthoSize;
		cam.nearPlane = nearPlane;
		cam.farPlane = farPlane;
		cam.aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

		//TODO: Set model matrix uniform
		for (size_t i = 0; i < NUM_CUBES; i++)
		{
			//Construct model matrix
			shader.setMat4("_Model", cubeTransforms[i].getModelMatrix());

			//Do Projection Work
			shader.setMat4("_View", cam.ViewMatrix());
			shader.setMat4("_Projection", cam.ProjectionMatrix());

			cubeMesh.draw();
		}

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			ImGui::Text("Cubes");
			for (size_t i = 0; i < NUM_CUBES; i++)
			{
				ImGui::PushID(i);
				if (ImGui::CollapsingHeader("Transform")) {
					ImGui::DragFloat3("Position", &cubeTransforms[i].position.x, 0.05f);
					ImGui::DragFloat3("Rotation", &cubeTransforms[i].rotation.x, 1.0f);
					ImGui::DragFloat3("Scale", &cubeTransforms[i].scale.x, 0.05f);
				}
				ImGui::PopID();
			}

			ImGui::Text("Camera");
			ImGui::DragFloat3("Position", &position.x, 0.5f);
			ImGui::DragFloat3("Target", &target.x, 0.5f);
			ImGui::Checkbox("Orthographic", &orthographic);

			if (!cam.orthographic) 
			{
				ImGui::DragFloat("FOV", &fov, 0.5f);
			}
			else 
			{
				ImGui::DragFloat("Ortho Size", &orthoSize, 0.5f);
			}

			ImGui::DragFloat("Near Plane", &nearPlane, 0.5f);
			ImGui::DragFloat("Far Plane", &farPlane, 0.5f);

			ImGui::Text("Camera Controller");
			
			std::string yaw_text = "Yaw: " + std::to_string((int)cameraControls.yaw);
			ImGui::Text(yaw_text.c_str());
			std::string pitch_text = "Pitch: " + std::to_string((int)cameraControls.pitch);
			ImGui::Text(pitch_text.c_str());

			if (ImGui::Button("Reset")) 
			{
				//Reset
				position = ew::Vec3(0, 0, 5);
				target = ew::Vec3(0, 0, 0);
				orthographic = false;
				orthoSize = 6.0f;
				fov = 60.0f;
				nearPlane = 0.1f;
				farPlane = 100.0f;

				//Set
				cam.position = position;
				cam.target = target;
				cam.orthographic = orthographic;
				cam.fov = fov;
				cam.orthoSize = orthoSize;
				cam.nearPlane = nearPlane;
				cam.farPlane = farPlane;
				cam.aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;
				
				cameraControls.yaw = 0;
				cameraControls.pitch = 0;
			}

			ImGui::End();
			
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void moveCamera(GLFWwindow* window, myLib::Camera* camera, myLib::CameraControls* controls, float deltaTime) {
	if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
		//Release cursor
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		controls->firstMouse = true;
		return;
	}

	//GLFW_CURSOR_DISABLED hides the cursor, but the position will still be changed as we move our mouse.
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Get screen mouse position this frame
	double mouseX, mouseY;
	glfwGetCursorPos(window, &mouseX, &mouseY);

	//If we just started right clicking, set prevMouse values to current position.
	//This prevents a bug where the camera moves as soon as we click.
	if (controls->firstMouse) {
		controls->firstMouse = false;
		controls->prevMouseX = mouseX;
		controls->prevMouseY = mouseY;
	}

	//TODO: Get mouse position delta for this frame
	float xOffset = mouseX - controls->prevMouseX;
	float yOffset = controls->prevMouseY - mouseY;  // Reversed since y-coordinates go from bottom to top

	xOffset *= controls->mouseSensitivity;
	yOffset *= controls->mouseSensitivity;

	//TODO: Add to yaw and pitch
	controls->yaw += xOffset;
	controls->pitch += yOffset;

	//TODO: Clamp pitch between -89 and 89 degrees
	controls->pitch = std::max(-89.0f, std::min(controls->pitch, 89.0f));

	float yawRad = DegreesToRads(controls->yaw);
	float pitchRad = DegreesToRads(controls->pitch);

	//Construct forward vector using yaw and pitch. Don't forget to convert to radians!
	ew::Vec3 forward;
	forward.x = sin(yawRad) * cos(pitchRad);
	forward.y = sin(pitchRad);
	forward.z = -cos(yawRad) * cos(pitchRad);

	//Remember previous mouse position
	controls->prevMouseX = mouseX;
	controls->prevMouseY = mouseY;

	//TODO: Using camera forward and world up (0,1,0), construct camera right and up vectors. Graham-schmidt process!
	ew::Vec3 right;
	right = ew::Cross(ew::Vec3(0, 1, 0), forward);
	right = ew::Normalize(right);

	ew::Vec3 up;
	up = ew::Cross(forward, right);
	up = ew::Normalize(up);

	//TODO: Keyboard controls for moving along forward, back, right, left, up, and down. See Requirements for key mappings.
	//EXAMPLE: Moving along forward axis if W is held.
	//Note that this is framerate dependent, and will be very fast until you scale by deltaTime. See the next section.
	//Z
	if (glfwGetKey(window, GLFW_KEY_W)) 
	{
		camera->position += forward * controls->moveSpeed * deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_S)) 
	{
		camera->position -= forward * controls->moveSpeed * deltaTime;
	}

	//X
	if (glfwGetKey(window, GLFW_KEY_D)) 
	{
		camera->position -= right * controls->moveSpeed * deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_A)) 
	{
		camera->position += right * controls->moveSpeed * deltaTime;
	}

	//Y
	if (glfwGetKey(window, GLFW_KEY_Q)) 
	{
		camera->position -= up * controls->moveSpeed * deltaTime;
	}
	else if (glfwGetKey(window, GLFW_KEY_E)) 
	{
		camera->position += up * controls->moveSpeed * deltaTime;
	}

	//Add value to target
	camera->target = camera->position + ew::Normalize(forward);
}

float DegreesToRads(float degrees) {
	return degrees * (3.1415 / 180.0);
}
