//Implemented by Austin Butt and William Rutherford
#version 330 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vUV;

out vec3 Normal;
out vec3 Position;
out vec2 UV;

uniform mat4 _Model;
uniform mat4 _ViewProjection;


void main(){
	Normal = mat3(transpose(inverse(_Model))) * vNormal;
	UV = vUV;
    Position = vec3(_Model * vec4(vPos, 1.0));
	gl_Position = _ViewProjection * _Model * vec4(vPos,1.0);
}










