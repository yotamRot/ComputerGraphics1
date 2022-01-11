
#version 150

#define FLAT        0
#define GOURAUD     1
#define PHONG       2   

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
out vec3 TangentLightPos;
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
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float La;
uniform float Ld;
uniform float Ls;
uniform float Ka;
uniform float Kd;
uniform float Ks;

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
            TangentLightPos = TBN * lightPosition;
            TangentViewPos  = vec3(0,0,0); // we are in camera space
            TangentFragPos  = TBN * fragmentPosition;
    }

    if(shadow_type == PHONG)
    {
        shadow = PHONG;
    }
    else // GOURAUD or FLAT
    {
	    vec3 normalizeNormal;
	    vec3 lightDirection;
	    vec3 viewDirection;
        if (useNormalMap)
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