#version 430 core  
out vec4 color; 

in vec2 vTexCoord;

uniform sampler2D uDepthMap;
uniform sampler2D uNormalMap;
uniform float uRadius;
uniform float uModelScale;
uniform vec2 uSize;
uniform vec2 uRND;
uniform vec3 uSamples[256];

uniform mat4 uT;
uniform mat4 uP;
uniform mat4 uV;


/* currently not used: try it */
uniform sampler2D uNoise;

bool test_sample(vec3 s){
	float z = texture2D(uDepthMap,s.xy).x;
	return   z > s.z;
}

mat3 basisFromNormal(vec3 N)
{
	int i_max = abs(N.z) > abs(N.x) ? (abs(N.z) > abs(N.y) ? 2 : 1) : (abs(N.x) > abs(N.y) ? 0 : 1); 
	vec3 a = vec3(0.0);
	a[i_max] = 1.0;

    vec3 T = normalize(cross(a, N));   // tangent
    vec3 B = cross(N, T);               // bitangent

    return mat3(T, B, N);
}

void main(void) 
{ 
	int n_samples = 256;
	float ao = 0.0;

	/* currently not used: try it */
	vec3 randomVec = texture(uNoise, vTexCoord * vec2(800.f) ).xyz; 
				
	//vec3 center = vec3(vTexCoord,texture2D(uDepthMap,vTexCoord).x);
	vec3 center = texture2D(uDepthMap,vTexCoord).yzw;
	float z = texture2D(uDepthMap,vTexCoord).x;
	vec3 normal = texture2D(uNormalMap,vTexCoord).xyz;

	vec3 rotate_axis = cross(vec3(0.0,0.0,1.0), normal);
	
 	if(z>0.99 ){
 			ao = 0.0;
		}
 	else
	{ 
		vec3 s;
		for(int i=0; i < n_samples; ++i)
			{
				mat3 B = basisFromNormal(normal);
				vec3 sn = B * uSamples[i];

				s = center + vec3(sn.x*uRadius*uModelScale,sn.y*uRadius*uModelScale, sn.z*uRadius*uModelScale);

				vec4 s_ndc = uP * vec4( s, 1.0);
				s_ndc /= s_ndc.w;
				s_ndc  = s_ndc * 0.5 + 0.5;

  				if( test_sample(s_ndc.xyz))
  					ao += 1.0 / float(n_samples);
			}	

		ao =clamp(ao,0.0,1.0);
	}
 	
	color = vec4(vec3(ao) ,1.0);
}