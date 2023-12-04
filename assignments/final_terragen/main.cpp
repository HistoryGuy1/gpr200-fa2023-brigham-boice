#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/texture.h>
#include <ew/procGen.h>
#include <ew/transform.h>
#include <ew/camera.h>
#include <ew/cameraController.h>
#include <../assignments/final_terragen/constants.h>


void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);

struct Light {
	ew::Vec3 position; //World space
	ew::Vec3 color; //RGB
};

struct Material {
	float ambientK; //Ambient coefficient (0-1)
	float diffuseK; //Diffuse coefficient (0-1)
	float specular; //Specular coefficient (0-1)
	float shininess; //Shininess
};

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

const int scaleRatio = 1.0 / 100.0;

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.0f);

ew::Camera camera;
ew::CameraController cameraController;

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

	//Global settings
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	ew::Shader shader("assets/defaultLit.vert", "assets/defaultLit.frag");
	unsigned int brickTexture = ew::loadTexture("assets/brick_color.jpg",GL_REPEAT,GL_LINEAR);

	ew::Shader emissiveShader("assets/emissive.vert", "assets/emissive.frag");

	camera.farPlane = 20000.0f;

	Material material{
		material.ambientK = 0.1f,
		material.diffuseK = 1.0f,
		material.specular = 1.0f,
		material.shininess = 50.0f
	};

	ew::Mesh earthMesh(ew::createSphere(6357.0f * Constants::scaleRatio, 64));
	ew::Transform earthTransform;
	earthTransform.position = ew::Vec3(0.0f, 0.0f, 0.0f);

	//ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10));
	//ew::Transform planeTransform;
	//planeTransform.position = ew::Vec3(0, -1.0, 0);

	ew::Mesh sunMesh = ew::createSphere(1392000.0f * Constants::scaleRatio, 20);
	ew::Transform sunSphereTransform;
	Light sunLight;
	sunLight.position = ew::Vec3(149600000.0f * Constants::scaleRatio, 0.0f, 0.0f);
	sunLight.color = ew::Vec3(253.0f / 255.0f, 184.0f / 255.0f, 19.0f / 255.0f);

	float lightintensity = 0.7f;
	ew::Vec3 colorOnEarth = ew::Vec3(lightintensity, lightintensity, lightintensity);

	resetCamera(camera, cameraController);
	
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		float deltaTime = time - prevTime;
		prevTime = time;

		//Update camera
		camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
		cameraController.Move(window, &camera, deltaTime);

		//RENDER
		glClearColor(bgColor.x, bgColor.y,bgColor.z,1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		glBindTexture(GL_TEXTURE_2D, brickTexture);
		shader.setInt("_Texture", 0);
		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		shader.setFloat("ambientK", material.ambientK);
		shader.setFloat("diffuseK", material.diffuseK);
		shader.setFloat("specularK", material.specular);
		shader.setFloat("shininess", material.shininess);
		shader.setVec3("_ViewPosition", camera.position);
		shader.setInt("numLights", 1);
		shader.setInt("useBlinnPhong", true);

		shader.setVec3("_Lights[0].position", sunLight.position);
		shader.setVec3("_Lights[0].color", colorOnEarth);

		//Draw shapes
		/*shader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();*/

		shader.setMat4("_Model", earthTransform.getModelMatrix());
		earthMesh.draw();

		//TODO: Render sun light
		emissiveShader.use();
		emissiveShader.setVec3("_Color", sunLight.color);
		emissiveShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		sunSphereTransform.position = sunLight.position;
		emissiveShader.setMat4("_Model", sunSphereTransform.getModelMatrix());
		sunMesh.draw();

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			if (ImGui::CollapsingHeader("Camera")) {
				ImGui::DragFloat3("Position", &camera.position.x, 0.1f);
				ImGui::DragFloat3("Target", &camera.target.x, 0.1f);
				ImGui::Checkbox("Orthographic", &camera.orthographic);
				if (camera.orthographic) {
					ImGui::DragFloat("Ortho Height", &camera.orthoHeight, 0.1f);
				}
				else {
					ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 180.0f);
				}
				ImGui::DragFloat("Near Plane", &camera.nearPlane, 0.1f, 0.0f);
				ImGui::DragFloat("Far Plane", &camera.farPlane, 0.1f, 0.0f);
				ImGui::DragFloat("Move Speed", &cameraController.moveSpeed, 0.1f);
				ImGui::DragFloat("Sprint Speed", &cameraController.sprintMoveSpeed, 0.1f);
				if (ImGui::Button("Reset")) {
					resetCamera(camera, cameraController);
				}
			}
			ImGui::ColorEdit3("BG color", &bgColor.x);

			ImGui::SliderFloat("AmbientK", &material.ambientK, 0.0f, 1.0f);
			ImGui::SliderFloat("DiffuseK", &material.diffuseK, 0.0f, 1.0f);
			ImGui::SliderFloat("SpecularK", &material.specular, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &material.shininess, 2.0f, 256.0f);

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
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void resetCamera(ew::Camera& camera, ew::CameraController& cameraController) {
	camera.position = ew::Vec3(0, 0, 5);
	camera.target = ew::Vec3(0);
	camera.fov = 60.0f;
	camera.orthoHeight = 6.0f;
	camera.nearPlane = 0.1f;
	camera.farPlane = 20000.0f;
	camera.orthographic = false;

	cameraController.yaw = 0.0f;
	cameraController.pitch = 0.0f;
}


