#version 410
in vec2 aPosition;
in vec3 aColor;
out vec3 vColor;

uniform vec3 uniColor;
void main(void)
{
 gl_Position = vec4(aPosition, 0.0, 1.0);
 vColor = uniColor;
}