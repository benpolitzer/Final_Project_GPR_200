#version 450 
out vec4 FragColor; 
in vec2 UV;

uniform sampler2D _WallTexture; 
uniform sampler2D _NoiseTexture;

uniform float _NoiseStrength;
uniform float _Tiling;
uniform float iTime;

void main(){
    float normalizedTime = sin(iTime) * 0.1;
    float noise = texture(_NoiseTexture,UV).r * normalizedTime;

    vec2 uv = UV;
    uv += noise * _NoiseStrength;

    vec4 texA = texture(_WallTexture, (uv * 2.0) * _Tiling);
    vec4 texB = texture(_WallTexture, (uv) * _Tiling);

    vec3 color = mix(texA.rgb, texB.rgb, texB.a);

    FragColor = vec4(color, 1.0);
}