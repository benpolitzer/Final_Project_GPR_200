#version 330 core
out vec4 FragColor;

in vec3 Position;
in vec3 Normal;
in vec2 UV;
uniform sampler2D _Texture;

//0 for uniform color 
//1 for normals
//2 for UVs
//3 for texture
//4 for shaded
uniform int _Mode;
uniform vec3 _Color;
uniform vec3 cameraPos;
uniform samplerCube skybox;

void main(){
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(skybox, R).rgb, 1.0);

}