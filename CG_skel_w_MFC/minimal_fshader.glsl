
#version 150

// Shadow defines
#define FLAT                0
#define GOURAUD             1
#define PHONG               2   
// Light type defines
#define POINT_SOURCE        0   
#define PARALLEL_SOURCE     1   
// Number of supported lights
#define LIGHT_SOURCES       10

// In Arguments
in  vec3 vertexColor;
in  vec3 vertexNormal;
in  vec3 fragmentPosition;
flat in  int shadow;
flat in vec3 polygonColor;
in vec2 TexCoord;
in vec3 TangentLightPos[LIGHT_SOURCES];
in  vec3 TangentViewPos;
in   vec3 TangentFragPos;
// Out Arguments
out vec4 fColor;


// Uniforms declaration
uniform int lights_number;
uniform vec3 lightPosition[LIGHT_SOURCES];
uniform vec3 lightColor[LIGHT_SOURCES];
uniform float La[LIGHT_SOURCES];
uniform float Ld[LIGHT_SOURCES];
uniform float Ls[LIGHT_SOURCES];
uniform int light_type[LIGHT_SOURCES];
uniform float Ka;
uniform float Kd;
uniform float Ks;
//uniform float alpha;

//texture
uniform sampler2D texture1;
uniform bool useTexture;

//normal map
uniform bool useNormalMap;
uniform sampler2D normalMap;


void phong_shadow();
void gouraud_shadow();
void flat_shadow();

void main() 
{ 
	if(shadow == FLAT)
	{
		flat_shadow();
	}
	else if(shadow == GOURAUD)
	{
		gouraud_shadow();
	}
	else if(shadow == PHONG)
	{
		phong_shadow();
	}
} 


void phong_shadow()
{
    vec3 result = vec3(0);
    vec3 normalizeNormal;
    vec3 lightDirection;
    vec3 reflectDir;
    vec3 viewDirection;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    for(int i = 0; i < lights_number; i++)
    {
		if(useNormalMap)
		{
			normalizeNormal = texture(normalMap, TexCoord).rgb;
			normalizeNormal = normalize(normalizeNormal * 2.0 - 1.0);  // this normal is in tangent space
			viewDirection = normalize(TangentViewPos - TangentFragPos);
			if(light_type[i] == POINT_SOURCE)
            {
                lightDirection =  normalize(TangentLightPos[i] - TangentFragPos);
            }
            else    // PARALLEL_SOURCE
            {
                lightDirection = normalize(TangentLightPos[i]);
            }
		}
		else
		{
            normalizeNormal = normalize(vertexNormal);
            viewDirection = normalize(0 - fragmentPosition);
            if(light_type[i] == POINT_SOURCE)
            {
                lightDirection = normalize(lightPosition[i] - fragmentPosition);
            }
            else    // PARALLEL_SOURCE
            {
                lightDirection = normalize(lightPosition[i]);
            }
		}
		reflectDir = reflect(-lightDirection, normalizeNormal); 
		ambient  = Ka * La[i] * lightColor[i];
		diffuse  = Kd * Ld[i] * max(dot(normalizeNormal, lightDirection), 0.0) * lightColor[i]; 
		specular = Ks * Ls[i] * pow(max(dot(viewDirection, reflectDir), 0.0), 2) * lightColor[i]; 
		result += (ambient + diffuse + specular) * vertexColor;
    }
	fColor = vec4(result, 1.0)* (useTexture ? texture(texture1, TexCoord) : vec4(1,1,1,1));
}

void gouraud_shadow()
{
	fColor = vec4(vertexColor,1.0) * (useTexture ? texture(texture1, TexCoord) : vec4(1,1,1,1));
}
void flat_shadow()
{
	fColor = vec4(polygonColor,1.0) * (useTexture ? texture(texture1, TexCoord) : vec4(1,1,1,1));
}

