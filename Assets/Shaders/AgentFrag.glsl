#version 430 core

uniform vec3 lightPos;
uniform vec4 lightCol;
uniform vec3 cameraPos;
uniform bool useLighting = true;
uniform float bound = 50;

in Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

void main(void)
{		
	vec4 albedo = IN.colour;

	if (useLighting) {

		vec3  incident = normalize(lightPos - IN.worldPos);
		float lambert  = max (0.0, dot (incident, IN.normal)) * 0.9; 
	
		albedo.rgb = pow(albedo.rgb, vec3(2.2));
	
		fragColor.rgb = albedo.rgb * 0.15;
	
		fragColor.rgb += albedo.rgb * lightCol.rgb * lambert;
	
		fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));

	} else {
		fragColor = albedo;
	}
}