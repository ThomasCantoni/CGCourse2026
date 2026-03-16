#version 410
in vec2 localPosition;
in vec3 aColor;
out vec3 vColor;

uniform mat4 parentMatrix;
uniform mat4 scale;
uniform mat4 rotation;
uniform mat4 translation;

uniform mat4 localMatrix;


void main(void)
{
	gl_Position = parentMatrix * localMatrix *vec4(localPosition,0,1);
	
	/*
	gl_Position = parentMatrix * translation*scale*rotation*vec4(localPosition,0,1);
	gl_Position =  parentMatrix * vec4(aPosition, 0.0, 1.0);
	*/
	vColor = aColor;
}