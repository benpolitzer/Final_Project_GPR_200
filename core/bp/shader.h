#pragma once
#include <sstream>
#include <fstream>
#include "../ew/external/glad.h"

using namespace std;

namespace bp {
	string loadShaderSourceFromFile(const string& filePath);
	unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
	unsigned int createShader(GLenum shaderType, const char* sourceCode);

	class Shader {
	public:
		Shader(const string& vertexShader, const string& fragmentShader);
		void use();
		void setInt(const string& name, int v) const;
		void setFloat(const string& name, float v) const;
		void setVec2(const string& name, float x, float y) const;
		void setVec3(const string& name, float x, float y, float z) const;
		void setVec4(const string& name, float x, float y, float z, float w) const;
	private:
		unsigned int m_id;
	};
}
/*
unsigned int createShader(GLenum shaderType, const char* sourceCode);
unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource);
unsigned int createVAO(float* vertexData, int numVertices);
*/