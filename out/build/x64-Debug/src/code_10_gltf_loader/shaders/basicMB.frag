#version 430 core  
out vec4 color; 
in vec2 fragCoord;

uniform vec2 uMB1;

uniform vec2 uMB2;

uniform vec3 uColor;
uniform vec3 uBorderColor;
uniform vec3 uIsovalueColor;



uniform float uControlRadius;

uniform float uMergingThreshold;


float MetaballFunction(vec2 metaballPos, float controlRadius)
{
	vec2 diffVec = fragCoord.xy - metaballPos;
	float distanceSquared = dot(diffVec,diffVec);
	if(distanceSquared <= controlRadius*controlRadius)
	{
		float toRet = 1.0- distanceSquared/(controlRadius*controlRadius);
		return toRet*toRet;

	}

	return 0.0;
}


void main(void) 
{  
	float alpha = max(0.01,uMergingThreshold); //isovalue
	float value1 = MetaballFunction(uMB1,uControlRadius);
	float value2 = MetaballFunction(uMB2,uControlRadius);

	float result = value1 + value2 ;
	float absRes = abs(result);
	if(result -2*alpha > 0) 
			color = vec4((result-2*alpha)*uColor,1.0);
	else if(abs(result -2*alpha )  <0.01)
		color = vec4(uIsovalueColor,1.0);
	else if(0.001< abs(result ) && abs(result )  <0.01)
			color = vec4(uBorderColor,1.0);
	else
		color = vec4(0.0,0.0,0.0,1.0);
} 