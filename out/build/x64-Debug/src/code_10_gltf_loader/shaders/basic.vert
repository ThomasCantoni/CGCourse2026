#version 430 core 
layout (location = 0) in vec3 aPosition; 
out vec2 fragCoord;



void main(void) 
{ 
	
	
    gl_Position = vec4(aPosition.xy,0.0, 1.0); 
	fragCoord = aPosition.xy;
}