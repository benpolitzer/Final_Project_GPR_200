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
uniform float _Rotate;
uniform float iTime;
uniform int _Voronoi;

const vec2 RESOLUTION = vec2(1920,1080);
const float PI = 3.1415926;

vec2 makeNoise( vec2 p )
{
    // procedural white noise	
	return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec3 voronoi( in vec2 x )
{
    vec2 ip = floor(x);
    vec2 fp = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mg, mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
		vec2 o = makeNoise( ip + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*sin( iTime + 6.2831*o );
        #endif	
        vec2 r = g + o - fp;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
		vec2 o = makeNoise( ip + g );
		#ifdef ANIMATE
        o = 0.5 + 0.5*sin( iTime + 6.2831*o );
        #endif	
        vec2 r = g + o - fp;

        if( dot(mr-r,mr-r)>0.00001 )
        md = min( md, dot( 0.5*(mr+r), normalize(r-mr) ) );
    }

    return vec3( md, mr );
}

void main(){
    //Experementing with voronoi from https://www.shadertoy.com/view/MlVBRR
    if(_Voronoi==1){
    vec3 fragCoord = gl_FragCoord.xyz;
    vec2 p = (2.0*fragCoord.xy-RESOLUTION.xy) / RESOLUTION.y;
    //vec3 ro = Position - cameraPos; //Camera pos??
	vec3 rd = normalize( vec3(p,-2.0) );
    vec3 q =  1.0 * rd;
    vec3 n = -normalize(q);
        
    float r = length(n);
    float theta = acos(n.y/r) / PI;
    float phi   = (atan(n.z,n.x) / PI + 1.0)/2.0;
    phi = mod(phi + _Rotate*iTime, 1.0);
    float phiN = fract(phi);    
    float thetaN = fract(theta);

        
    vec2 x = vec2(thetaN, phiN);
    //vec2 p = (2.0*fragCoord.xy-RESOLUTION.xy) / RESOLUTION.y;
    vec3 v = voronoi(vec2(50.0,100.0)*x);

    vec3 I = normalize(Position - cameraPos);   
    vec3 R = reflect(I, normalize(Normal)) * vec3(1.0, 1.0, -1.0);
    vec3 C = texture(skybox, R).rgb * mix(0.25, 1.0, smoothstep( 0.0, 0.07, v.x ) );
    //FragColor = vec4(texture(skybox, R).rgb, 1.0);
    FragColor = vec4(C, 1.0);
    }




    else{
    vec3 I = normalize(Position - cameraPos);   
    vec3 R = reflect(I, normalize(Normal)) * vec3(1.0, 1.0, -1.0);
    FragColor = vec4(texture(skybox, R).rgb, 1.0);
    }
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