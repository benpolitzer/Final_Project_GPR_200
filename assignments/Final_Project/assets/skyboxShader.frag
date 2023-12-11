//Implemented by Benjamin Politzer
//Frag Shader
#version 330 core
//Final color of fragment
out vec4 FragColor;
//Texture coordinates recieved from the vertex shader
in vec3 texCoords;
//Uniform for sampling cubemap texture
uniform samplerCube skybox;

void main()
{
	//Samples the cubemap texture using the texture
	//coords and assigns the outputted color to Frag Color
	FragColor = texture(skybox, texCoords);
}







