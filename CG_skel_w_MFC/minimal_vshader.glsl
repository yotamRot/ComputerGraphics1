
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
in  vec3 vPosition;
in  vec3 vNormal;
in  vec2 vTexture;
in  vec3 vTangent;
in  vec3 vBiTangent;

// Out Arguments
out vec3 vertexColor;
out vec3 vertexNormal;
out vec3 fragmentPosition;
flat out int shadow;
flat out vec3 polygonColor;
out vec2 TexCoord;
out vec3 TangentLightPos[LIGHT_SOURCES];
out vec3 TangentViewPos;
out vec3 TangentFragPos;



// Uniforms declaration
// location
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;


//lights
uniform vec3 color;
uniform int shadow_type;
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


//normal map
uniform bool useNormalMap;
uniform sampler2D normalMap;



void main()
{
    vertexColor = color;
    fragmentPosition = vec3(modelViewMatrix * modelMatrix * vec4(vPosition, 1.0));
    vertexNormal = mat3(modelViewMatrix * normalMatrix ) * vNormal;
    TexCoord = vec2(vTexture.x, vTexture.y);
   
   if (useNormalMap)
   {
        vec3 T = normalize(mat3(modelViewMatrix * normalMatrix ) * vTangent);
        vec3 N = normalize(mat3(modelViewMatrix * normalMatrix ) * vNormal);
        T = normalize(T - dot(T, N) * N);
        vec3 B = cross(N, T);
        mat3 TBN = transpose(mat3(T, B, N));    
        for(int i = 0; i < lights_number; i++)
        {
            
            if(light_type[i] == POINT_SOURCE)
            {
                TangentLightPos[i] = TBN * lightPosition[i];
            }
            else    // PARALLEL_SOURCE
            {
                TangentLightPos[i] = TBN * vec3(0);
            }

	    }
        TangentViewPos  = TBN * vec3(0,0,0); // we are in camera space
        TangentFragPos  = TBN * fragmentPosition;
    }
    if(shadow_type == PHONG)
    {
        shadow = PHONG;
    }
    else // GOURAUD or FLAT
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
	        if (useNormalMap)
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

        if(shadow_type == GOURAUD)
        {
            shadow = GOURAUD;
	        vertexColor = result;
        }
        else // FLAT
        {
            polygonColor = result;
            shadow = FLAT;
        }
    }
    gl_Position = projectionMatrix * vec4(fragmentPosition,1.0);
}