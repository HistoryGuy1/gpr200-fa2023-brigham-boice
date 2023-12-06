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
ew::Vec3 moveOnUnitCircle(float earthAngle, float distance);
float lerp(float a, float b, float f);
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
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Material material{
		material.ambientK = 0.05f,
		material.diffuseK = 1.0f,
		material.specular = 0.5f,
		material.shininess = 1.0f
	};

	Material moonMaterial{
		moonMaterial.ambientK = 0.05f,
		moonMaterial.diffuseK = 4.0f,
		moonMaterial.specular = 0.1f,
		moonMaterial.shininess = 0.05f
	};

	//----------------Earth---------------------

	ew::Shader earthShader("assets/defaultLit.vert", "assets/defaultLit.frag");
	unsigned int earthTexture = ew::loadTexture("assets/world5k.png", GL_REPEAT, GL_LINEAR);
	unsigned int nightTexture = ew::loadTexture("assets/worldN.jpg", GL_REPEAT, GL_LINEAR);

	ew::Mesh earthMesh(ew::createSphere(6357.0f * Constants::scaleRatio, 640));
	ew::Transform earthTransform;
	earthTransform.position = ew::Vec3(0.0f, 0.0f, 0.0f);
	earthTransform.rotation = ew::Vec3(0.0f, 0.0f, 0.0f);
	float earthAxialTilt = 180.0f + 23.4f;
	float earthRotY = 0.0f;
	float earthSpinSpeed = 10.0f;

	//Add height here
	earthMesh.load(ew::createEarth(40075.0f * Constants::scaleRatio, 20000.0f * Constants::scaleRatio, 6357.0f * Constants::scaleRatio, 640, 1.0f));

	//-------------------Clouds------------------------

	ew::Shader sphereShader("assets/cloud.vert", "assets/cloud.frag");
	unsigned int cloudTexture = ew::loadTexture("assets/cloud.png", GL_REPEAT, GL_LINEAR);

	ew::Mesh cloudMesh(ew::createSphere((6357.0f + 10.0f) * Constants::scaleRatio, 640));
	ew::Transform cloudTransform;
	cloudTransform.position = ew::Vec3(0.0f, 0.0f, 0.0f);
	cloudTransform.rotation = ew::Vec3(0.0f, 0.0f, 0.0f);

	//-------------------Moon----------------------

	ew::Shader moonShader("assets/moon.vert", "assets/moon.frag");
	unsigned int moonTexture = ew::loadTexture("assets/moon1k.jpg", GL_REPEAT, GL_LINEAR);

	float moonDistance = 384400.0f * Constants::scaleRatio;

	ew::Mesh moonMesh(ew::createSphere(1737.4f * Constants::scaleRatio, 64));
	ew::Transform moonTransform;
	moonTransform.position = ew::Vec3(moonDistance, 0.0f, 0.0f);
	moonTransform.rotation = ew::Vec3(0.0f, 0.0f, 0.0f);

	//----------------------Sun------------------------

	ew::Shader emissiveShader("assets/emissive.vert", "assets/emissive.frag");

	float sunDistance = 149600000.0f * Constants::scaleRatio;

	ew::Mesh sunMesh = ew::createSphere(1392000.0f * Constants::scaleRatio, 20);
	ew::Transform sunSphereTransform;
	Light sunLight;
	sunLight.position = ew::Vec3(sunDistance, 0.0f, 0.0f);
	sunLight.color = ew::Vec3(253.0f / 255.0f, 184.0f / 255.0f, 55.0f / 255.0f);

	float lightintensity = 1.0f;
	ew::Vec3 colorOnEarth = ew::Vec3(lightintensity, lightintensity, lightintensity);

	//---------------------Stars---------------------

	ew::Shader starShader("assets/stars.vert", "assets/stars.frag");
	unsigned int starTexture = ew::loadTexture("assets/starmap16k.jpg", GL_REPEAT, GL_LINEAR);

	ew::Mesh starMesh(ew::createSphere(18000.0f, 640));
	ew::Transform starTransform;
	cloudTransform.position = ew::Vec3(0.0f, 0.0f, 0.0f);
	cloudTransform.rotation = ew::Vec3(0.0f, 0.0f, 0.0f);

	camera.farPlane = 20000.0f;
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
		glEnable(GL_BLEND);

		//--------------------Earth------------------

		earthRotY += earthSpinSpeed * deltaTime;
		float scale = (cos(time) + 1.0f) / 2.0f;
		scale = 1;
		earthMesh.load(ew::createEarth(40075.0f * Constants::scaleRatio, 20000.0f * Constants::scaleRatio, 6357.0f * Constants::scaleRatio, 64, scale));

		earthTransform.rotation = ew::Vec3(
			lerp(180.0f, earthAxialTilt, scale),
			lerp(earthRotY / 365.25f, earthRotY, scale),
			lerp(180.0f, 0.0f, scale));

		earthShader.use();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, earthTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, nightTexture);
		earthShader.setInt("_Texture", 0);
		earthShader.setInt("_TextureNight", 1);

		earthShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		earthShader.setFloat("ambientK", material.ambientK);
		earthShader.setFloat("diffuseK", material.diffuseK);
		earthShader.setFloat("specularK", material.specular);
		earthShader.setFloat("shininess", material.shininess);
		earthShader.setVec3("_ViewPosition", camera.position);
		earthShader.setInt("numLights", 1);
		earthShader.setInt("useBlinnPhong", true);

		earthShader.setVec3("_Lights[0].position", sunLight.position);
		earthShader.setVec3("_Lights[0].color", colorOnEarth);

		earthShader.setMat4("_Model", earthTransform.getModelMatrix());
		earthMesh.draw();

		//-----------------Clouds----------------------

		cloudTransform.rotation = ew::Vec3(earthAxialTilt, earthRotY / 1.2f, 0.0f);

		sphereShader.use();
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, cloudTexture);
		sphereShader.setInt("_Texture", 2);

		sphereShader.setMat4("_Model", cloudTransform.getModelMatrix());
		sphereShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		sphereShader.setFloat("ambientK", material.ambientK);
		sphereShader.setFloat("diffuseK", material.diffuseK);
		sphereShader.setFloat("specularK", material.specular);
		sphereShader.setFloat("shininess", material.shininess);
		sphereShader.setVec3("_ViewPosition", camera.position);
		sphereShader.setInt("numLights", 1);
		sphereShader.setInt("useBlinnPhong", true);

		sphereShader.setVec3("_Lights[0].position", sunLight.position);
		sphereShader.setVec3("_Lights[0].color", colorOnEarth);

		cloudMesh.draw();

		//-----Math for sun, moon, and stars

		float spaceRotation = -earthRotY / 365.25f;

		//-----------------------Moon-------------------------

		moonTransform.position = moveOnUnitCircle(spaceRotation * 12.4f, moonDistance);
		moonTransform.rotation = ew::Vec3(0.0f, -spaceRotation * 12.4f, 0.0f);

		moonShader.use();
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, moonTexture);
		moonShader.setInt("_Texture", 4);
		moonShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		moonShader.setFloat("ambientK", moonMaterial.ambientK);
		moonShader.setFloat("diffuseK", moonMaterial.diffuseK);
		moonShader.setFloat("specularK", moonMaterial.specular);
		moonShader.setFloat("shininess", moonMaterial.shininess);
		moonShader.setVec3("_ViewPosition", camera.position);
		moonShader.setInt("numLights", 1);
		moonShader.setInt("useBlinnPhong", true);

		moonShader.setVec3("_Lights[0].position", sunLight.position);
		moonShader.setVec3("_Lights[0].color", colorOnEarth);

		moonShader.setMat4("_Model", moonTransform.getModelMatrix());
		moonMesh.draw();

		//------------------------Stars---------------------

		starTransform.rotation = ew::Vec3(0.0f, spaceRotation, 0.0f);

		starShader.use();
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, starTexture);
		starShader.setInt("_Texture", 3);

		starShader.setMat4("_Model", starTransform.getModelMatrix());
		starShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		starMesh.draw();

		//-------------------------Sun---------------------

		sunLight.position = moveOnUnitCircle(spaceRotation, sunDistance);

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

			ImGui::SliderFloat("Spin Speed", &earthSpinSpeed, 0.0f, 360.0f);


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

ew::Vec3 moveOnUnitCircle(float earthAngle, float distance)
{
	ew::Vec3 finalPos = ew::Vec3(0.0f);
	//angle / days in year * to radians
	float angleInRadians = earthAngle * (3.141592f / 180.0f);
	finalPos.x = cos(angleInRadians) * distance;
	finalPos.z = sin(angleInRadians) * distance;

	return finalPos;
}

float lerp(float a, float b, float f)
{
	return a * (1.0 - f) + (b * f);
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


