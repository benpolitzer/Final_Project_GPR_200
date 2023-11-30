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

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

struct Light {
	ew::Vec3 position;
	ew::Vec3 color;
};

struct Material {
	float ambientK; //Ambient coefficient (0-1)
	float diffuseK; //Diffuse coefficient (0-1)
	float specular; //Specular coefficient (0-1)
	float shininess; //Shininess
};

const int MAX_LIGHTS = 4;

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

	ew::Shader lightShader("assets/unlit.vert", "assets/unlit.frag");

	ew::Shader shader("assets/defaultLit.vert", "assets/defaultLit.frag");
	unsigned int brickTexture = ew::loadTexture("assets/brick_color.jpg", GL_REPEAT, GL_LINEAR);

	//Create cube
	ew::Mesh cubeMesh(ew::createCube(1.0f));
	ew::Mesh planeMesh(ew::createPlane(5.0f, 5.0f, 10));
	ew::Mesh sphereMesh(ew::createSphere(0.5f, 64));
	ew::Mesh cylinderMesh(ew::createCylinder(0.5f, 1.0f, 32));

	ew::Mesh lightMesh[MAX_LIGHTS];

	for (int i = 0; i < 4; i++)
	{
		lightMesh[i] = ew::createSphere(0.3f, 15);
	}

	//Initialize transforms
	ew::Transform cubeTransform;
	ew::Transform planeTransform;
	ew::Transform sphereTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform[4];
	planeTransform.position = ew::Vec3(0, -1.0, 0);
	sphereTransform.position = ew::Vec3(-1.5f, 0.0f, 0.0f);
	cylinderTransform.position = ew::Vec3(1.5f, 0.0f, 0.0f);

	Light lights[MAX_LIGHTS];

	for (int i = 0; i < MAX_LIGHTS; i++)
	{
		lightTransform[i].position.z = (i % (MAX_LIGHTS / 2) - 0.5) * 4;
		lightTransform[i].position.y = 2.0;
		lightTransform[i].position.x = (i / (MAX_LIGHTS / 2) - 0.5) * 4;
		lights[i].position = lightTransform[i].position;
	}

	lights[0].color = ew::Vec3(1.0, 1.0, 1.0);
	lights[1].color = ew::Vec3(0.6, 0.0, 1.0);
	lights[2].color = ew::Vec3(1.0, 0.0, 0.0);
	lights[3].color = ew::Vec3(0.0, 0.0, 1.0);

	Material material;

	material.shininess = 50.0f;
	material.ambientK = 0.1;
	material.diffuseK = 0.5;
	material.specular = 1.0;

	int numLights = 1;
	float lightIntensity = 0.5;
	bool orbit = false;

	resetCamera(camera, cameraController);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		float deltaTime = time - prevTime;
		prevTime = time;

		if (orbit)
		{
			for (int i = 0; i < MAX_LIGHTS; i++)
			{
				lightTransform[i].position.x = cos(time + (2 * ew::PI / (numLights)*i)) * 3;
				lightTransform[i].position.z = (sin(time + (2 * ew::PI / (numLights)*i))) * 3;
				lights[i].position = lightTransform[i].position;
			}
		}

		//Update camera
		camera.aspectRatio = (float)SCREEN_WIDTH / SCREEN_HEIGHT;
		cameraController.Move(window, &camera, deltaTime);

		//RENDER
		glClearColor(bgColor.x, bgColor.y, bgColor.z, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();

		glBindTexture(GL_TEXTURE_2D, brickTexture);
		shader.setInt("_Texture", 0);
		shader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());

		for (int i = 0; i < numLights; i++)
		{
			shader.setVec3("_Lights[" + std::to_string(i) + "].position", lights[i].position);
			shader.setVec3("_Lights[" + std::to_string(i) + "].color", lights[i].color);
		}

		shader.setVec3("camPos", camera.position);

		//Draw shapes
		shader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		shader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		shader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		shader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		shader.setFloat("shininess", material.shininess);
		shader.setFloat("ambient", material.ambientK);
		shader.setFloat("specular", material.specular);
		shader.setFloat("diffuse", material.diffuseK);
		shader.setInt("numLights", numLights);
		shader.setFloat("lightIntensity", lightIntensity);

		//TODO: Render point lights

		lightShader.use();

		for (int i = 0; i < numLights; i++)
		{
			lightShader.setMat4("_Model", lightTransform[i].getModelMatrix());
			lightShader.setMat4("_ViewProjection", camera.ProjectionMatrix() * camera.ViewMatrix());
			lightShader.setVec3("_Color", lights[i].color);
			lightMesh[i].draw();
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

			if (ImGui::CollapsingHeader("Light Settings"))
			{
				ImGui::DragFloat("Ambient", &material.ambientK, 0.01, 0.0, 1.0);
				ImGui::DragFloat("Diffuse", &material.diffuseK, 0.01, 0.0, 1.0);
				ImGui::DragFloat("Specular", &material.specular, 0.01, 0.0, 1.0);
				ImGui::DragFloat("Shininess", &material.shininess, 16.0, 0.0, 1024);
				ImGui::DragFloat("Light Intensity", &lightIntensity, 0.01, 0.0, 1.0f);
				ImGui::SliderInt("Number Of Lights", &numLights, 0, 4);
				ImGui::Checkbox("Orbitting", &orbit);
				for (int i = 0; i < numLights; i++)
				{
					ImGui::PushID(i);
					std::string currLight = "Light " + std::to_string(i + 1);

					if (ImGui::CollapsingHeader(currLight.c_str())) {
						ImGui::ColorEdit3("Color", &lights[i].color.x, 1.0f);
						ImGui::DragFloat3("Position", &lightTransform[i].position.x, 0.1f);
						lights[i].position = lightTransform[i].position;
					}
					ImGui::PopID();
				}
			}

			ImGui::ColorEdit3("BG color", &bgColor.x);

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