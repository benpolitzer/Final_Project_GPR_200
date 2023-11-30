#version 450
	out vec4 FragColor;
	in vec2 UV;
	//uniform vec3 _Color;
	//uniform float _Brightness;
	void main(){
		FragColor = vec4(UV,0.0,1.0);
	}