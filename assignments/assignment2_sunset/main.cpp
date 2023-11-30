#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "bp/shader.h"

struct Vertex {
	float x, y, z;
	float u, v;
};

unsigned int createShader(GLenum shaderType, const char* sourceCode);
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned int* indicesData, int numIndices);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);


const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

Vertex vertices[4] = {
	//x    y    z    u    v
	{-1.0, -1.0, 0.0, 0.0, 0.0},
	{1.0, -1.0, 0.0, 1.0, 0.0},
	{1.0,  1.0, 0.0, 1.0, 1.0},
	{-1.0,  1.0, 0.0, 0.0, 1.0}
};

unsigned int indices[6] = {
	0, 1, 2,
	2, 3, 0
};

bool showImGUIDemoWindow = false;
float skyTop[3] = { 0.263, 0.612, 0.855 };
float skyBot[3] = { 0.741, 0.325, 0.0 };
float sunColor[3] = { 0.918, 0.839, 0.039 };
float hillColor[3] = { 0.184,0.251,0.157 };
float backHillColor[3] = { 0.129,0.173,0.106 };
float sunRadius = 0.2;
float sunSpeed = 1.0;


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

	bp::Shader shader("assets/vertexShader.vert", "assets/alienSunsetShader.frag");
	shader.use();

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	unsigned int vao = createVAO(vertices, 4, indices, 6);

	glBindVertexArray(vao);
	

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//Set uniforms
		shader.setFloat("iTime", (float)glfwGetTime());
		shader.setVec3("iResolution", SCREEN_WIDTH, SCREEN_HEIGHT, 1);
		shader.setVec3("skyTop", skyTop[0], skyTop[1], skyTop[2]);
		shader.setVec3("skyBot", skyBot[0], skyBot[1], skyBot[2]);
		shader.setVec3("sunColor", sunColor[0], sunColor[1], sunColor[2]);
		shader.setVec3("hillColor", hillColor[0], hillColor[1], hillColor[2]);
		shader.setVec3("backHillColor", backHillColor[0], backHillColor[1], backHillColor[2]);
		shader.setFloat("sunRadius", sunRadius);
		shader.setFloat("sunSpeed", sunSpeed);


		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);


		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			ImGui::Checkbox("Show Demo Window", &showImGUIDemoWindow);
			ImGui::ColorEdit3("Sky Color (top)", skyTop);
			ImGui::ColorEdit3("Sky Color (bottom)", skyBot);
			ImGui::ColorEdit3("Sun Color", sunColor);
			ImGui::ColorEdit3("Hill Color", hillColor);
			ImGui::ColorEdit3("Back Hill Color", backHillColor);
			ImGui::SliderFloat("Sun Radius", &sunRadius, 0.0f, 2.0f);
			ImGui::SliderFloat("Sun Speed", &sunSpeed, 0.0f, 10.0f);

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

unsigned int createVAO(Vertex* vertexData, int numVertices, unsigned int* indicesData, int numIndices) {
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices, indicesData, GL_STATIC_DRAW);

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

	return vao;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

