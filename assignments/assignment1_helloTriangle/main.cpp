#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/ewMath/ewMath.h>
#include <GLFW/glfw3.h>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;


unsigned int createVAO(float* vertexData, int numVertices)
{
	//Define a new buffer id
	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	//Allocates space and sends data to GPU
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * numVertices, vertexData, GL_STATIC_DRAW);

	//Makes a VAO that sends inputs to the vertex shader
	unsigned int vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	//Tell vao to pull vertex data from vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	//Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)0);
	glEnableVertexAttribArray(0);

	//Color attribute
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (const void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	return vao;
}

unsigned int createShader(GLenum shaderType, const char* sourceCode)
{
	unsigned int vertexShader = glCreateShader(shaderType);
	glShaderSource(vertexShader, 1, &sourceCode, NULL);
	glCompileShader(vertexShader);
	int success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		//512 is an arbitrary length, but should be plenty of characters for our error message.
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		printf("Failed to compile shader: %s", infoLog);
		return 0;
	}
	return vertexShader;
}

unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
	unsigned int vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
	unsigned int fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

	unsigned int shaderProgram = glCreateProgram();
	//Attach each stage
	glAttachShader(shaderProgram, vertexShader);
	//glAttachShader(shaderProgram, geometryShader);
	glAttachShader(shaderProgram, fragmentShader);
	//Link all the stages together
	glLinkProgram(shaderProgram);

	int success;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("Failed to link shader program: %s", infoLog);
		return 0;
	}
	//The linked program now contains our compiled code, so we can delete these intermediate objects
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

int main() {
	//global variable in main.cpp, or in main()
	float vertices[21] = {
		//x   //y  //z   //r  //g  //b  //a
		-0.5, -0.5, 0.0, 1.0, 0.0, 0.0, 1.0, //Bottom left
		 0.5, -0.5, 0.0, 0.0, 1.0, 0.0, 1.0, //Bottom right
		 0.0,  0.5, 0.0, 0.0, 0.0, 1.0, 1.0  //Top center
	};

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

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return 1;
	}

	unsigned int vao = createVAO(vertices, sizeof(vertices));

	const char* vertexShaderSource = R"(
		#version 450
		layout(location = 0) in vec3 vPos;
		layout(location = 1) in vec4 vColor;
		out vec4 Color;
		uniform float _Time;
		void main(){
			Color = vColor;
			vec3 offset = vec3(0,sin(vPos.x + _Time),0)*0.75;
			gl_Position = vec4(vPos + offset,0.75);
		}
	)";

	const char* fragmentShaderSource = R"(
		#version 450
		out vec4 FragColor;
		in vec4 Color;
		uniform float _Time;
		void main(){
			FragColor = Color * abs(sin(_Time));
		}
	)";

	//Opengl setup
	//VAO
	//Shader Program
	unsigned int shader = createShaderProgram(vertexShaderSource, fragmentShaderSource);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		glClearColor(0.3f, 0.4f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(shader);
		glBindVertexArray(vao);
		//Get time in seconds
		float time = (float)glfwGetTime();
		//Get uniform
		int timeLocation = glGetUniformLocation(shader, "_Time");
		//Set unitform
		glUniform1f(timeLocation, time);
		glDrawArrays(GL_TRIANGLES, 0, 3);

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}
