#define PI 3.1415926
// set to reflection anim speed
#define ANIMATE (0.0) 
// set to rotation anim speed
#define ROTATE (0.025)
#define RIM_ATTENUATION 

uniform vec3 iResolution;

const vec2 scale = vec2(50.0,100.0);

// sphere of size ra centered at point ce
vec2 sphIntersect( in vec3 ro, in vec3 rd, in vec3 ce, float ra )
{
    vec3 oc = ro - ce;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - ra*ra;
    float h = b*b - c;
    if( h < 0.0 ) return vec2(-1.0); // no intersection
    h = sqrt( h );
    return vec2( -b-h, -b+h );
}

vec2 hash2( vec2 p )
{
    // procedural white noise	
	return fract(sin(vec2(dot(p,vec2(127.1,311.7)),dot(p,vec2(269.5,183.3))))*43758.5453);
}

vec2 perturb(vec2 o) {
    return  0.5+0.5*o*vec2(sin(6.2831*o.x + ANIMATE * iTime), cos(6.2831*o.x + ANIMATE * iTime));
}

vec2 mg;
vec2 mo;

vec3 voronoi( in vec2 x )
{
    vec2 n = floor(x);
    vec2 f = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
		vec2 o = hash2( mod(n + g,scale) );
		#ifdef ANIMATE
        o = perturb(o);
        #endif	
        vec2 r = g + o - f;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
            mo = o;
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
		vec2 o = hash2( mod(n + g,scale)  );
		#ifdef ANIMATE
        o = perturb(o);
        #endif	
        vec2 r = g + o - f;

        if( dot(mr-r,mr-r)>0.00001 )
        md = min( md, dot( 0.5*(mr+r), normalize(r-mr) ) );
    }
	
    return vec3( md, mr );
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
	{
        
	vec2 position=(fragCoord.xy/iResolution.xy);
	vec2 p = (2.0*fragCoord.xy-iResolution.xy) / iResolution.y;
   
	vec3 ro = vec3(0.0, 0.0, 4.0 );
	vec3 rd = normalize( vec3(p,-2.0) );
	        
    vec2 sphere = sphIntersect(ro, rd, vec3(0.0), 1.5);
        if (sphere.x < 0.0) {
         fragColor= 0.5*texture(iChannel0, rd);
         return;   
        }
        
    vec3 q = ro + sphere.x * rd;
    vec3 n = -normalize(q);
        
    float r = length(n);
    float theta = acos(n.y/r) / PI;
    float phi   = (atan(n.z,n.x) / PI + 1.0)/2.0;
    phi = mod(phi + ROTATE*iTime + iMouse.x/iResolution.x, 1.0);
    theta = mod(theta + (1.0-iMouse.y/iResolution.y), 1.0);
    float phiN = fract(phi);    
    float thetaN = fract(theta);

        
    vec2 x = vec2(thetaN, phiN);
    vec3 c = voronoi(scale * x);

	float dd = length( c.yz );
	// isolines
    vec3 col = mo.xyx;//vec3(0.2,0.4, 0.6);

    vec3 N = normalize(n + 0.2 * col);
    vec3 V = vec3(0.0, 0.0, 1.0);
    vec3 R = dot(N,V)*N*2.0 - V;
    
    vec4 refl = vec4(1.0,0.95,0.98,1.0)*texture(iChannel0, R); 
        // borders	
    float s = 0.05 + 0.2 * pow(1.0-dot(n,rd),3.0);
    refl *= mix( 0.25, 1.0, smoothstep( 0.0, s, c.x ) );
        
	fragColor = (0.5 + 0.7*(1.0 - dot(n,rd))) * refl;
        
#ifdef RIM_ATTENUATION
    fragColor *= vec4(-n.zzz, 1.0);
#endif
    
}