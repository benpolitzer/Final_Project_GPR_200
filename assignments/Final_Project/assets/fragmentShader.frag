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
    vec3 R = reflect(I, normalize(Normal)) * vec3(1.0, 1.0, -1.0);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);


    //alternate calculation
//    vec3 eye_forward = normalize(vec3((cos(cameraPos.y) * cos(cameraPos.x)),
//                                sin(cameraPos.y),
//                                (cos(cameraPos.y) * sin(cameraPos.x))));
//    vec3 eye_right = normalize(cross(eye_forward, vec3(0.0, 1.0, 0.0)));
//    vec3 eye_up = cross(eye_right, eye_forward);
//
//    mat3 invNormalM = mat3(eye_right, eye_up, eye_forward);
//    vec3 viewIncident  = normalize(Position);
//    vec3 viewNormal = normalize(Normal);
//    vec3 worldReflection = invNormalM * reflect(viewIncident, viewNormal);
//    FragColor = vec4(texture(skybox, worldReflection).rgb, 1.0);

}