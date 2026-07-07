#version 430 core  
layout (location = 0) out vec4 color; 
layout (location = 1) out vec4 normal;

in vec3 vNVS;
in vec3 vPVS;

void main(void) 
{ 
 	color = vec4(gl_FragCoord.z,vPVS);
 	normal = vec4(normalize(vNVS)*0.5+0.5,1.0);

} 