#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <bb/shader.h>

struct Vertex {
	float x, y, z;
	float u, v;
};

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned int* indicesData, int numIndices);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

Vertex vertices[4] = {
	//x    y    z    u    v
   { -1,  -1,   0,   0,   0 }, //Bottom left
   {  1,  -1,   0,   1,   0 }, //Bottom right
   {  1,   1,   0,   1,   1 },  //Top right
   { -1,   1,   0,   0,   1 }  //Top left
};

unsigned int indices[6] = {
	0, 1, 2,
	2, 3, 0
};

float skyBottom[3] = { 0.937f, 0.592f, 0.341f };
float skyTop[3] = { 0.294f, 0.286f, 0.431f };
float sunBottom[3] = { 0.996f, 0.270f, 0.0f };
float sunTop[3] = { 0.984f, 0.925f, 0.231f };
float hillColor[3] = { 0.290f, 0.352f, 0.188f };
float sunRadius = 0.2f;
float speed = 1.0f;

bool showImGUIDemoWindow = true;

int main() {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Hello Triangle", NULL, NULL);
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

	unsigned int vao = createVAO(vertices, 4, indices, 6);

	bbLib::Shader shader("assets/vertexShader.vert", "assets/fragmentShader.frag");

	shader.use();
	glBindVertexArray(vao);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Set uniforms
		shader.setVec3("_skyTop", skyTop[0], skyTop[1], skyTop[2]);
		shader.setVec3("_skyBottom", skyBottom[0], skyBottom[1], skyBottom[2]);
		shader.setVec3("_sunBottom", sunBottom[0], sunBottom[1], sunBottom[2]);
		shader.setVec3("_sunTop", sunTop[0], sunTop[1], sunTop[2]);
		shader.setVec3("_hillColor", hillColor[0], hillColor[1], hillColor[2]);
		shader.setFloat("_sunRadius", sunRadius);
		shader.setFloat("_speed", speed);

		float time = glfwGetTime();
		shader.setFloat("iTime", time);
		shader.setVec2("iResolution", SCREEN_WIDTH, SCREEN_HEIGHT);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			ImGui::Checkbox("Show Demo Window", &showImGUIDemoWindow);

			ImGui::ColorEdit3("Sky Top Color", skyTop);
			ImGui::ColorEdit3("Sky Bottom Color", skyBottom);
			ImGui::ColorEdit3("Sun Bottom Color", sunBottom);
			ImGui::ColorEdit3("Sun Top Color", sunTop);
			ImGui::ColorEdit3("Hill Color", hillColor);

			ImGui::SliderFloat("Sun Radius", &sunRadius, 0.01f, 2.0f);
			ImGui::SliderFloat("Sun Speed", &speed, 0.1f, 5.0f);

			ImGui::End();
			if (showImGUIDemoWindow) {
				ImGui::ShowDemoWindow(&showImGUIDemoWindow);
			}

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned int* indicesData, int numIndices)
{
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	//Define a new buffer id
	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//Allocate space for + send vertex data to GPU.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, vertexData, GL_STATIC_DRAW);

	//Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)offsetof(Vertex, x));
	glEnableVertexAttribArray(0);

	//UV
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void*)(offsetof(Vertex, u)));
	glEnableVertexAttribArray(1);

	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, indicesData, GL_STATIC_DRAW);

	return vao;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
}

