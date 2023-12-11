//Reflections implemented by Austin Butt and Benjamin Politzer
//Skybox implemented by Benjamin Politzer with debugging from William Rutherford
//Vornoi implemented by William Rutherford
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
int sphereSegments = 50;

float skyboxVertices[] ={
	//   Coordinates
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
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

	//Shader creation
	ew::Shader shader("assets/vertexShader.vert", "assets/fragmentShader.frag");
	ew::Shader skyboxShader("assets/skyboxShader.vert", "assets/skyboxShader.frag");

	//Sphere Mesh
	ew::MeshData sphereMeshData = bp::createSphere(sphereRadius, sphereSegments);
	ew::Mesh sphereMesh(sphereMeshData);

	//Initialize transforms for sphere
	ew::Transform sphereTransform;
	sphereTransform.position = ew::Vec3(0.0f, 0.0f, 0.0f);

	resetCamera(camera, cameraController);

	/////////////////////SKYBOX/////////////////////

	//Creation of VAO, VBO, and EBO for the skybox
	glEnable(GL_DEPTH_TEST);
	unsigned int skyboxVAO, skyboxVBO, skyboxEBO;
	//Generates a VAO and stores its ID in skyboxVAO which is then bound
	glGenVertexArrays(1, &skyboxVAO);
	glBindVertexArray(skyboxVAO);
	//Generates a VBO and stores its ID in skyboxVBO which is then bound
	glGenBuffers(1, &skyboxVBO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	//Generates a EBO and stores its ID in skyboxVBO which is then bound
	glGenBuffers(1, &skyboxEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
	//Loads the vertex data into the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	//Loads the indices data into the EBO
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices, GL_STATIC_DRAW);
	//Sets up the vertex attribute pointer for position data 
	//where 3 indicates there are that many components per vertex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//Unbinds the array buffer to prevent modification
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//Unbinds the VAO
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	//Textures for cube map stored in an array of strings
	std::string facesCubemap[6] ={
			"assets/Cube Maps/Chapel/posx.jpg",
			"assets/Cube Maps/Chapel/negx.jpg",
			"assets/Cube Maps/Chapel/posy.jpg",
			"assets/Cube Maps/Chapel/negy.jpg",
			"assets/Cube Maps/Chapel/posz.jpg",
			"assets/Cube Maps/Chapel/negz.jpg"
	};
	
	//Creates the cubemap texture object
	unsigned int cubemapTexture;
	//Generates a single texture object and stores its ID in cubemapTexture
	glGenTextures(1, &cubemapTexture);
	//Binds the texture object to the target GL_TEXTURE_CUBE_MAP for operations down the line
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	//Sets the magnification and minification filter to linear interpolation
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//Prevent seams that may appear at edges during rendering
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	//Cycles through all the textures and attaches them to the cubemap object
	for (unsigned int i = 0; i < 6; i++)
	{
		int width, height, nrChannels;
		//Load image data from the file and returns a pointer to the image data, 
		// width, height, and the # of color channels.
		unsigned char* data = stbi_load(facesCubemap[i].c_str(), &width, &height, &nrChannels, 0);
		//If data for image was loaded correctly, enter the loop
		if (data)
		{
			//Do not flip image on load
			stbi_set_flip_vertically_on_load(false);
			//Used to specify a two-dimensional texture image for the cubemap face at current index
			glTexImage2D
			(
				//Specifies the target face of the cubemap
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				//Indicates the format for the texture(RGB)
				GL_RGB,
				//Image dimensions
				width,
				height,
				0,
				//Specify the format and type of the pixel data
				GL_RGB,
				GL_UNSIGNED_BYTE,
				//Contains image data
				data
			);
			//Free memory allocated by stb_image for loading the current image
			stbi_image_free(data);
		}
		//If the current image fails to load, print message below
		else
		{
			std::cout << "Failed to load texture: " << facesCubemap[i] << std::endl;
			stbi_image_free(data);
		}
	}

	/////////////////////END/////////////////////

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

		shader.use();
		glBindTexture(GL_TEXTURE_2D, cubemapTexture);
		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		/////////////////////SKYBOX/////////////////////
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		skyboxShader.setInt("skybox", 0);
		skyboxShader.setMat4("view", removeTranslation(camera.ViewMatrix()));
		skyboxShader.setMat4("projection", camera.ProjectionMatrix());

		//Binds skyboxVAO
		glBindVertexArray(skyboxVAO);
		//Activates texture unit 0
		glActiveTexture(GL_TEXTURE0);
		//Binds cube map texture to the active texture unit
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		//Draws the skybox using indexed rendered triangles and 36 indices
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		//Unbinds the VAO
		glBindVertexArray(0);
		// Switch back to the normal depth function
		glDepthMask(GL_LESS);
		/////////////////////END/////////////////////

		shader.use();
		//Draw Sphere
		shader.setMat4("_Model", sphereTransform.getModelMatrix());
		shader.setVec3("cameraPos", camera.position);
		sphereMesh.draw((ew::DrawMode)appSettings.drawAsPoints);

		//For switching between vornoi and no vornoi
		if (voronoi)
			shader.setInt("_Voronoi", 1);
		else
			shader.setInt("_Voronoi", 0);

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
	//Free up memory taken up by skyboxVAO and VBO
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
