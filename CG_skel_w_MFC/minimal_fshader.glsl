
#version 150


#define FLAT            0
#define GOURAUD         1
#define PHONG           2

// In Arguments
in  vec3 vertexColor;
in  vec3 vertexNormal;
in  vec3 fragmentPosition;
flat in  int shadow;
flat in vec3 polygonColor;
in vec2 TexCoord;
in vec3 TangentLightPos;
in  vec3 TangentViewPos;
in   vec3 TangentFragPos;
// Out Arguments
out vec4 fColor;


// Uniforms declaration
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform float La;
uniform float Ld;
uniform float Ls;
uniform float Ka;
uniform float Kd;
uniform float Ks;
//uniform float Alpha;

//texture
uniform sampler2D texture1;
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
	vec3 normalizeNormal;
	vec3 lightDirection;
	vec3 viewDirection;
	if(useNormalMap)
	{
		normalizeNormal = texture(normalMap, TexCoord).rgb;
		normalizeNormal = normalize(normalizeNormal * 2.0 - 1.0);  // this normal is in tangent space
		lightDirection =  normalize(TangentLightPos - TangentFragPos);
		viewDirection = normalize(TangentViewPos - TangentFragPos);
	}
	else
	{
		normalizeNormal = normalize(vertexNormal);
		lightDirection = normalize(lightPosition - fragmentPosition);
		viewDirection = normalize(0 - fragmentPosition);
		
	}

	vec3 reflectDir = reflect(-lightDirection, normalizeNormal); 

	vec3 ambient = Ka * La * lightColor;
	vec3 diffuse =  Kd * Ld * max(dot(normalizeNormal, lightDirection), 0.0) * lightColor; 
	vec3 specular  = Ks * Ls * pow(max(dot(viewDirection, reflectDir), 0.0), 2) * lightColor; 
	vec3 result = (ambient + diffuse + specular) * vertexColor;
	fColor = vec4(result, 1.0)* texture(texture1, TexCoord);

 
}

void gouraud_shadow()
{
	fColor = vec4(vertexColor,1.0) * texture(texture1, TexCoord);
}
void flat_shadow()
{
	fColor = vec4(polygonColor,1.0) * texture(texture1, TexCoord);
}

