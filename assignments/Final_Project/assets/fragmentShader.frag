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
    vec3 fragCoord = gl_FragCoord.xyz;
    vec2 p = fragCoord.xy/vec2(1920,1920);
    vec3 v = voronoi(8.0*p);

    vec3 I = normalize(Position - cameraPos);   
    vec3 R = reflect(I, normalize(Normal)) * vec3(1.0, 1.0, -1.0);
    R = mix(vec3(1.0,0.6,0.0), R, smoothstep( 0.0, 0.07, v.x ) );
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