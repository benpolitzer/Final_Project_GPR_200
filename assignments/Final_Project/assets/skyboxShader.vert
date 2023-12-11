//Implemented by Benjamin Politzer

//Vertex Shader
#version 330 core
//Declares aPos of type vec3 representing the vertex position 
layout (location = 0) in vec3 aPos;
//Declares texCoords of type vec3 to pass data from the 
//vertex shader to the fragment shader.
out vec3 texCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    //Applies the projection and view matrices to 
    //the input vertex position and is stored in pos
    vec4 pos = projection * view * vec4(aPos, 1.0f);
    //Sets gl_Position to the transformed position 
    gl_Position = vec4(pos.x, pos.y, pos.w, pos.w);
    //Assigns a modified version of the input 
    //position to the texCoords variable (Flipped Z) 
    texCoords = vec3(aPos.x, aPos.y, -aPos.z);
} 









