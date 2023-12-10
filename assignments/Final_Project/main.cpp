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
#include <bp/procGen.h>;
#include <ew/external/stb_image.h>
#include <iostream>
#include <ew/ewMath/mat4.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void resetCamera(ew::Camera& camera, ew::CameraController& cameraController);
ew::Mat4 removeTranslation(ew::Mat4);

int SCREEN_WIDTH = 1920;
int SCREEN_HEIGHT = 1080;

float prevTime;

struct AppSettings {
	bool wireframe = false;
	bool drawAsPoints = false;
}appSettings;

ew::Camera camera;
ew::CameraController cameraController;

float sphereRadius = 1.0f;
int sphereSegments = 20;

float skyboxVertices[] ={
	//   Coordinates
	-1.0f, -1.0f,  1.0f,//        7--------6
	 1.0f, -1.0f,  1.0f,//       /|       /|
	 1.0f, -1.0f, -1.0f,//      4--------5 |
	-1.0f, -1.0f, -1.0f,//      | |      | |
	-1.0f,  1.0f,  1.0f,//      | 3------|-2
	 1.0f,  1.0f,  1.0f,//      |/       |/
	 1.0f,  1.0f, -1.0f,//      0--------1
	-1.0f,  1.0f, -1.0f
};
unsigned int skyboxIndices[] ={
	// Right
	1, 2, 6,
	6, 5, 1,
	// Left
	0, 4, 7,
	7, 3, 0,
	// Top
	4, 5, 6,
	6, 7, 4,
	// Bottom
	0, 3, 2,
	2, 1, 0,
	// Back
	0, 1, 5,
	5, 4, 0,
	// Front
	3, 7, 6,
	6, 2, 3
};

int main() {
	bool voronoi = false;
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

	//Depth testing - required for depth sorting!
	glEnable(GL_DEPTH_TEST);
	glPointSize(3.0f);
	glPolygonMode(GL_FRONT_AND_BACK, appSettings.wireframe ? GL_LINE : GL_FILL);

	//Shader creation
	ew::Shader shader("assets/vertexShader.vert", "assets/fragmentShader.frag");
	ew::Shader skyboxShader("assets/skyboxShader.vert", "assets/skyboxShader.frag");

	//Texture creation
	unsigned int discoTexture = ew::loadTexture("assets/discoBall.jpg", GL_REPEAT, GL_LINEAR);

	//Create Mesh Data
	ew::MeshData sphereMeshData = bp::createSphere(sphereRadius, sphereSegments);
	ew::Mesh cubeMesh(ew::createCube(1.0f));


	//Create Mesh Renderer
	ew::Mesh sphereMesh(sphereMeshData);

	//Initialize transforms
	ew::Transform sphereTransform;
	ew::Transform cubeTransform;
	sphereTransform.position = ew::Vec3(2.0f, 0.0f, 0.0f);
	cubeTransform.position = ew::Vec3(-2.0f, 0.0f, 0.0f);

	resetCamera(camera, cameraController);

	/////////////////////SKYBOX/////////////////////

	// Creation of VAO, VBO, and EBO for the skybox
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glGenBuffers(1, &skyboxEBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//Textures for cube map stored in an array of strings
	std::string facesCubemap[6] ={
			"assets/Cube Maps/Church/posx.jpg",
			"assets/Cube Maps/Church/negx.jpg",
			"assets/Cube Maps/Church/posy.jpg",
			"assets/Cube Maps/Church/negy.jpg",
			"assets/Cube Maps/Church/posz.jpg",
			"assets/Cube Maps/Church/negz.jpg"

	};
	
	//Creates the cubemap texture object
	unsigned int cubemapTexture;
	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Prevent seams
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			stbi_set_flip_vertically_on_load(false);
			glTexImage2D
			(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}
	/////////////////////END/////////////////////

	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;

		float time = (float)glfwGetTime();
		float deltaTime = time - prevTime;
		prevTime = time;

		cameraController.Move(window, &camera, deltaTime);

		//Render
		glClearColor(0.07f, 0.13f, 0.17f, 1.0f);

		//Clear both color buffer AND depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		unsigned int textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

		shader.use();
		glBindTexture(GL_TEXTURE_2D, cubemapTexture);
		shader.setInt("_Texture", 0);
		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		/////////////////////SKYBOX/////////////////////
		glDepthFunc(GL_LEQUAL);
		//glDepthMask(GL_FALSE);
		skyboxShader.use();
		skyboxShader.setInt("skybox", 0);
		skyboxShader.setMat4("view", removeTranslation(camera.ViewMatrix()));
		skyboxShader.setMat4("projection", camera.ProjectionMatrix());

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Switch back to the normal depth function
		glDepthMask(GL_LESS);
		/////////////////////END/////////////////////

		shader.use();
		//Draw Sphere
		shader.setMat4("_Model", sphereTransform.getModelMatrix());
		shader.setVec3("cameraPos", camera.position);

		if (voronoi)
			shader.setInt("_Voronoi", 1);
		else
			shader.setInt("_Voronoi", 0);

		sphereMesh.draw((ew::DrawMode)appSettings.drawAsPoints);

		shader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Render UI
		{
			ImGui_ImplGlfw_NewFrame();
			ImGui_ImplOpenGL3_NewFrame();
			ImGui::NewFrame();

			ImGui::Begin("Settings");
			if (ImGui::CollapsingHeader("Camera")) {
				ImGui::DragFloat3("Position", &camera.position.x, 0.1f);
				ImGui::DragFloat3("Target", &camera.target.x, 0.1f);
				ImGui::SliderFloat("FOV", &camera.fov, 0.0f, 180.0f);
				ImGui::DragFloat("Move Speed", &cameraController.moveSpeed, 0.1f);
				ImGui::DragFloat("Sprint Speed", &cameraController.sprintMoveSpeed, 0.1f);
				if (ImGui::Button("Reset")) {
					resetCamera(camera, cameraController);
				}
			}

			//ImGui::ColorEdit3("BG color", &appSettings.bgColor.x);
			ImGui::Checkbox("Draw as points", &appSettings.drawAsPoints);
			if (ImGui::Checkbox("Wireframe", &appSettings.wireframe)) {
				glPolygonMode(GL_FRONT_AND_BACK, appSettings.wireframe ? GL_LINE : GL_FILL);
			}

			ImGui::Text("Cube Controls");
			ImGui::DragFloat3("Cube Size", &cubeTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Cube Transform", &cubeTransform.position.x, 0.1f);

			ImGui::Text("Sphere Controls");
			ImGui::DragFloat3("Sphere Scale", &sphereTransform.scale.x, 0.1f);
			ImGui::DragFloat3("Sphere Transform", &sphereTransform.position.x, 0.1f);
			ImGui::DragInt("Sphere Segments", &sphereSegments, 0.1, 3.0, 1000000000.0);
			ImGui::Checkbox("Use Voronoi (WIP)", &voronoi);
			sphereMeshData = bp::createSphere(sphereRadius, sphereSegments);
			sphereMesh.load(sphereMeshData);

			ImGui::End();

			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		}

		glfwSwapBuffers(window);
	}
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
	printf("Shutting down...");
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
}

void resetCamera(ew::Camera& camera, ew::CameraController& cameraController) {
	camera.position = ew::Vec3(0, 0, 3);
	camera.target = ew::Vec3(0);
	camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
	camera.fov = 60.0f;
	camera.orthoHeight = 6.0f;
	camera.nearPlane = 0.1f;
	camera.farPlane = 100.0f;
	camera.orthographic = false;

	cameraController.yaw = 0.0f;
	cameraController.pitch = 0.0f;
}

ew::Mat4 removeTranslation(ew::Mat4 in)
{
	for (int i = 0; i <= 3; i++)
	{
		in[3][i] = 0.0f;
	}
	for (int i = 0; i <= 3; i++)
	{
		in[i][3] = 0.0f;
	}
	return in;
}
