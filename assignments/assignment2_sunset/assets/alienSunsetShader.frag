#version 450

uniform float iTime;
uniform vec3 iResolution;    

uniform vec3 skyTop; //vec3(0.263, 0.612, 0.855)
uniform vec3 skyBot; //vec3(0.741, 0.325, 0.0)
uniform vec3 sunColor; //vec3(0.918, 0.839, 0.039)
uniform vec3 hillColor; //vec3(0.184,0.251,0.157)
uniform vec3 backHillColor; //vec3(0.129,0.173,0.106)
uniform float sunRadius; //2
uniform float sunSpeed; //1.0

float circleSDF(vec2 p, float r){
    return length(p)-r;
}

out vec4 fragColor;
vec2 fragCoord = gl_FragCoord.xy;

void main()
{
    vec2 uv = fragCoord/iResolution.xy;
    float aspect = iResolution.x / iResolution.y;
    
    float sky = sin(uv.y*0.25+iTime*sunSpeed)*1.0 + 0.5;
    vec3 col = mix(skyBot, skyTop, uv.y+sky);
    
    uv = (2.0*fragCoord-iResolution.xy)/iResolution.y;
    
    float blur = 15.0*aspect/iResolution.x;
    
    vec2 sunPos = uv + vec2(-0.02, 0.6+sin(iTime*sunSpeed)*1.0);
    col = mix(col, sunColor, smoothstep(blur,-blur,circleSDF(sunPos,sunRadius)));
    
    float backHills = (0.5 + cos(uv.x*1.75)*0.75) * (1 + sin(uv.x)*1.25);
    backHills = step(backHills,-uv.y);

    col = mix(col, backHillColor, backHills);

    float hills = 0.5 + (0.8 + sin(uv.x*2.0)*0.3) * (0.5*sin(uv.x)*0.5);
    hills = step(hills,-uv.y);
    
    col = mix(col, hillColor, hills);


    // Output to screen
    fragColor = vec4(col,1.0);
}