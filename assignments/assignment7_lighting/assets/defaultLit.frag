#version 450
out vec4 FragColor;

in Surface{
	vec2 UV;
	vec3 WorldPosition;
	vec3 WorldNormal;
}fs_in;

uniform sampler2D _Texture;

struct Light
{
	vec3 position;
	vec3 color;
};
#define MAX_LIGHTS 4
uniform Light _Lights[MAX_LIGHTS];

uniform vec3 camPos;
uniform float shininess;
uniform float ambient;
uniform float diffuse;
uniform float specular;
uniform int mode;
uniform int numLights;
uniform float lightIntensity;

void main(){
	vec3 ambient = vec3(1.0) * ambient;
	vec3 finalLight = vec3(0);
	for(int i = 0; i < numLights; i++)
	{
		vec3 normal = normalize(fs_in.WorldNormal);
		vec3 lightDirection = _Lights[i].position - fs_in.WorldPosition;
		lightDirection = normalize(lightDirection);
		float dist = length(lightDirection);
		dist = dist * dist;
		vec3 _diffuse = _Lights[i].color * diffuse;
		vec3 specularColor = _Lights[i].color * specular;

		float iDiffuse = max(dot(normal, lightDirection), 0);
		float _specular = 0.0;
		if(iDiffuse > 0)
		{
			vec3 viewDirection = normalize(camPos - fs_in.WorldPosition);
			if(mode == 0)
			{
				vec3 halfDirection = normalize(lightDirection + viewDirection);
				float specularAngle = max(dot(halfDirection, normal), 0.0);
				_specular = pow(specularAngle, shininess);
			}
			else
			{
				vec3 r = reflect(-lightDirection, normal);
				float specularAngle = max(dot(r, viewDirection), 0.0);
				_specular = pow(specularAngle, shininess/4.0);
			}
		}
		finalLight += _diffuse * iDiffuse * _Lights[i].color * lightIntensity + 
					 specularColor * _specular * _Lights[i].color * lightIntensity;
	}
	vec3 color = ambient + finalLight;

	vec4 textColor = texture(_Texture,fs_in.UV);
	FragColor = vec4(textColor.rgb * color, textColor.a);
}