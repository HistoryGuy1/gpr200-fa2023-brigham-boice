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

float prevTime;
ew::Vec3 bgColor = ew::Vec3(0.1f);

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

	Material material{
		material.ambientK = 1.0f,
		material.diffuseK = 1.0f,
		material.specular = 1.0f,
		material.shininess = 50.0f
	};

	//Create cube
	ew::Mesh cubeMesh(ew::createCube(1.0f));
	ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10));
	ew::Mesh sphereMesh(ew::createSphere(0.5f, 64));
	ew::Mesh cylinderMesh(ew::createCylinder(0.5f, 1.0f, 32));

	//Initialize transforms
	ew::Transform cubeTransform;
	ew::Transform planeTransform;
	ew::Transform sphereTransform;
	ew::Transform cylinderTransform;
	planeTransform.position = ew::Vec3(0, -1.0, 0);
	sphereTransform.position = ew::Vec3(-1.5f, 0.0f, 0.0f);
	cylinderTransform.position = ew::Vec3(1.5f, 0.0f, 0.0f);

	resetCamera(camera,cameraController);

	const int MAX_LIGHTS = 4;
	int numLights = MAX_LIGHTS;
	bool useBlinnPhong = true;

	ew::Mesh lightSphereMeshes[MAX_LIGHTS]{
		lightSphereMeshes[0] = ew::createSphere(0.5f, 20),
		lightSphereMeshes[1] = ew::createSphere(0.5f, 20),
		lightSphereMeshes[2] = ew::createSphere(0.5f, 20),
		lightSphereMeshes[3] = ew::createSphere(0.5f, 20)
	};

	ew::Transform lightSphereTransforms[MAX_LIGHTS];

	Light lights[MAX_LIGHTS]{
		lights[0].position = ew::Vec3(5.0f,0.0f,0.0f),
		lights[0].color = ew::Vec3(0.5f,0.0f,0.0f),
		lights[1].position = ew::Vec3(0.0f,5.0f,0.0f),
		lights[1].color = ew::Vec3(0.0f,0.5f,0.0f),
		lights[2].position = ew::Vec3(5.0f,0.0f,5.0f),
		lights[2].color = ew::Vec3(0.0f,0.0f,0.5f),
		lights[3].position = ew::Vec3(0.0f,5.0f,0.0f),
		lights[3].color = ew::Vec3(0.5f,0.5f,0.5f),
	};

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
		shader.setInt("numLights", numLights);
		shader.setInt("useBlinnPhong", useBlinnPhong ? 0 : 1);

		for (int i = 0; i < numLights; i++)
		{
			shader.setVec3("_Lights[" + std::to_string(i) + "].position", lights[i].position);
			shader.setVec3("_Lights[" + std::to_string(i) + "].color", lights[i].color);
		}

		//Draw shapes
		shader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		shader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		shader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//TODO: Render point lights
		for (int i = 0; i < numLights; i++) {
			emissiveShader.use();
			emissiveShader.setVec3("_Color", lights[i].color);
			emissiveShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

			lightSphereTransforms[i].position = lights[i].position;
			emissiveShader.setMat4("_Model", lightSphereTransforms[i].getModelMatrix());
			lightSphereMeshes[i].draw();
		}

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

			ImGui::End();

			ImGui::Begin("Material Settings");
			ImGui::SliderFloat("AmbientK", &material.ambientK, 0.0f, 1.0f);
			ImGui::SliderFloat("DiffuseK", &material.diffuseK, 0.0f, 1.0f);
			ImGui::SliderFloat("SpecularK", &material.specular, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &material.shininess, 2.0f, 256.0f);

			ImGui::Checkbox("Use Blinn-Phong", &useBlinnPhong);

			ImGui::SliderInt("Number of Lights", &numLights, 0, MAX_LIGHTS);

			for (int i = 0; i < numLights; i++) {
				ImGui::DragFloat3(("Light " + std::to_string(i) + " Position").c_str(), &lights[i].position.x);
				ImGui::ColorEdit3(("Light " + std::to_string(i) + " Color").c_str(), &lights[i].color.x);
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
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

void resetCamera(ew::Camera& camera, ew::CameraController& cameraController) {
	camera.position = ew::Vec3(0, 0, 5);
	camera.target = ew::Vec3(0);
	camera.fov = 60.0f;
	camera.orthoHeight = 6.0f;
	camera.nearPlane = 0.1f;
	camera.farPlane = 100.0f;
	camera.orthographic = false;

	cameraController.yaw = 0.0f;
	cameraController.pitch = 0.0f;
}


