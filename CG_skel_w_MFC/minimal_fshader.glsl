
#version 150

// Shadow defines
#define FLAT                0
#define GOURAUD             1
#define PHONG               2   
#define TOON                3
// Light type defines
#define POINT_SOURCE        0   
#define PARALLEL_SOURCE     1   
// Number of supported lights
#define LIGHT_SOURCES       10

// In Arguments
in  vec3 vOriginalPosition;
in  vec3 vertexColor;
in  vec3 vertexNormal;
in  vec3 fragmentPosition;
flat in  int shadow;
flat in vec3 polygonColor;
in  vec2 TexCoord;
in  vec3 TangentLightPos[LIGHT_SOURCES];
in  vec3 TangentViewPos;
in  vec3 TangentFragPos;
in  float back_face;
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
uniform float Ke;
uniform float alpha;

//texture
uniform sampler2D texture1;
uniform bool useTexture;

//normal map
uniform bool useNormalMap;
uniform sampler2D normalMap;

//enviroment
uniform samplerCube enviromentTexture;
uniform bool useEnviromentTexture;

uniform float time;
uniform int useColorAnimation;
uniform int useColorAnimationGradient;

uniform int toonColorNumber;

varying vec3 normal;

// marble
uniform bool isMarble;

//ununiform material
uniform bool isNonUniform;
uniform float MaxY;

void phong_shadow();
void gouraud_shadow();
void flat_shadow();
void toon_shadow();
vec3 marble(vec3 p);

vec4 textureColor;
vec3 curColor;
float cnoise(vec3 P);
vec3 calcColorSpeciel();

void main() 
{ 
	if(useEnviromentTexture)
	{
		vec3 I = normalize(fragmentPosition);
		vec3 R = reflect(I, normalize(vertexNormal));
		textureColor = texture(enviromentTexture, R);
	}
	else if(isMarble)
	{
		textureColor =  vec4(marble(vOriginalPosition),1);
	}
	else
	{
		textureColor = texture(texture1, TexCoord);
	}

	if(shadow == FLAT)
	{
		curColor = polygonColor;
		flat_shadow();
	}
	else if(shadow == GOURAUD)
	{
		curColor = vertexColor;
		gouraud_shadow();
	}
	else if(shadow == PHONG)
	{
		if(isNonUniform || useColorAnimationGradient == 1)
		{
			curColor =  calcColorSpeciel();
		}
		else
		{
			curColor = vertexColor;
		}
		phong_shadow();
	}
	else if(shadow == TOON)
	{
		if(isNonUniform ||  useColorAnimationGradient == 1)
		{
			curColor =  calcColorSpeciel();
		}
		else
		{
			curColor = vertexColor;
		}
		toon_shadow();
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
	float v_Ka, v_Kd, v_Ks;
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

		if(isNonUniform)
        {
            v_Ka = 0.4 + sin(cnoise(fragmentPosition));
            v_Kd = sin(cnoise(2*fragmentPosition));
            v_Ks = sin(cnoise(3*fragmentPosition));  
        }
        else
        {
            v_Ka = Ka;
            v_Kd = Kd;
            v_Ks = Ks;
        }
		float shininess;
		if(lights_number > 1)
        {
            shininess = alpha;
        }
        else
        {
            shininess = 2;
        }
		reflectDir = reflect(-lightDirection, normalizeNormal); 
		ambient  = v_Ka * La[i] * lightColor[i];
		diffuse  = v_Kd * Ld[i] * max(dot(normalizeNormal, lightDirection), 0.0) * lightColor[i]; 
		specular = v_Ks * Ls[i] * pow(max(dot(viewDirection, reflectDir), 0.0), shininess) * lightColor[i]; 
		result += (ambient + diffuse + specular) * curColor + Ke;
    }

	if(useColorAnimation == 1)
	{
		fColor = vec4(result*time,1.0) * (useTexture ? textureColor : vec4(1,1,1,1));
	}
	else
	{
		fColor = vec4(result, 1.0)* (useTexture ? textureColor : vec4(1,1,1,1));
	}
}



void gouraud_shadow()
{
	if(useColorAnimation == 1)
	{
		fColor = vec4(curColor*time, 1.0) * (useTexture ? textureColor : vec4(1,1,1,1));
	}
	else
	{
		fColor = vec4(curColor, 1.0) * (useTexture ? textureColor : vec4(1,1,1,1));
	}

}


void flat_shadow()
{
	if(useColorAnimation == 1)
	{
		fColor = vec4(curColor * time,1.0) * (useTexture ? textureColor : vec4(1,1,1,1));
	}
	else
	{
		fColor = vec4(curColor,1.0) * (useTexture ? textureColor : vec4(1,1,1,1));
	}
}

void toon_shadow()
{
	float intensity;

	vec4 color;
	vec3 lightDirection;
	for(int i = 0; i < lights_number; i++)
    {
        if(light_type[i] == POINT_SOURCE)
        {
            lightDirection = normalize(lightPosition[i] - fragmentPosition);
        }
        else    // PARALLEL_SOURCE
        {
            lightDirection = normalize(lightPosition[i]);
        }
	}

	intensity = dot(lightDirection, normal);


	if(back_face < 0)
	{
		color = vec4(0,0,0,1.0);
	}
	else
	{ 
		color = (max(round(intensity * toonColorNumber),1) / toonColorNumber) * vec4(curColor,1);
	}
	fColor = color;
}

vec3 marble_color (float val)
{
    vec3 color;
    val = sqrt(sqrt(sqrt(0.5*(val+1))));          
    color = vec3(.2 + .75*val); 
    color.y*=0.95;            
    return color;
}

vec4 permute(vec4 x){return mod(((x*34.0)+1.0)*x, 289.0);}
vec4 taylorInvSqrt(vec4 r){return 1.79284291400159 - 0.85373472095314 * r;}
vec3 fade(vec3 t) {return t*t*t*(t*(t*6.0-15.0)+10.0);}

float cnoise(vec3 P){
  vec3 Pi0 = floor(P); // Integer part for indexing
  vec3 Pi1 = Pi0 + vec3(1.0); // Integer part + 1
  Pi0 = mod(Pi0, 289.0);
  Pi1 = mod(Pi1, 289.0);
  vec3 Pf0 = fract(P); // Fractional part for interpolation
  vec3 Pf1 = Pf0 - vec3(1.0); // Fractional part - 1.0
  vec4 ix = vec4(Pi0.x, Pi1.x, Pi0.x, Pi1.x);
  vec4 iy = vec4(Pi0.yy, Pi1.yy);
  vec4 iz0 = Pi0.zzzz;
  vec4 iz1 = Pi1.zzzz;

  vec4 ixy = permute(permute(ix) + iy);
  vec4 ixy0 = permute(ixy + iz0);
  vec4 ixy1 = permute(ixy + iz1);

  vec4 gx0 = ixy0 / 7.0;
  vec4 gy0 = fract(floor(gx0) / 7.0) - 0.5;
  gx0 = fract(gx0);
  vec4 gz0 = vec4(0.5) - abs(gx0) - abs(gy0);
  vec4 sz0 = step(gz0, vec4(0.0));
  gx0 -= sz0 * (step(0.0, gx0) - 0.5);
  gy0 -= sz0 * (step(0.0, gy0) - 0.5);

  vec4 gx1 = ixy1 / 7.0;
  vec4 gy1 = fract(floor(gx1) / 7.0) - 0.5;
  gx1 = fract(gx1);
  vec4 gz1 = vec4(0.5) - abs(gx1) - abs(gy1);
  vec4 sz1 = step(gz1, vec4(0.0));
  gx1 -= sz1 * (step(0.0, gx1) - 0.5);
  gy1 -= sz1 * (step(0.0, gy1) - 0.5);

  vec3 g000 = vec3(gx0.x,gy0.x,gz0.x);
  vec3 g100 = vec3(gx0.y,gy0.y,gz0.y);
  vec3 g010 = vec3(gx0.z,gy0.z,gz0.z);
  vec3 g110 = vec3(gx0.w,gy0.w,gz0.w);
  vec3 g001 = vec3(gx1.x,gy1.x,gz1.x);
  vec3 g101 = vec3(gx1.y,gy1.y,gz1.y);
  vec3 g011 = vec3(gx1.z,gy1.z,gz1.z);
  vec3 g111 = vec3(gx1.w,gy1.w,gz1.w);

  vec4 norm0 = taylorInvSqrt(vec4(dot(g000, g000), dot(g010, g010), dot(g100, g100), dot(g110, g110)));
  g000 *= norm0.x;
  g010 *= norm0.y;
  g100 *= norm0.z;
  g110 *= norm0.w;
  vec4 norm1 = taylorInvSqrt(vec4(dot(g001, g001), dot(g011, g011), dot(g101, g101), dot(g111, g111)));
  g001 *= norm1.x;
  g011 *= norm1.y;
  g101 *= norm1.z;
  g111 *= norm1.w;

  float n000 = dot(g000, Pf0);
  float n100 = dot(g100, vec3(Pf1.x, Pf0.yz));
  float n010 = dot(g010, vec3(Pf0.x, Pf1.y, Pf0.z));
  float n110 = dot(g110, vec3(Pf1.xy, Pf0.z));
  float n001 = dot(g001, vec3(Pf0.xy, Pf1.z));
  float n101 = dot(g101, vec3(Pf1.x, Pf0.y, Pf1.z));
  float n011 = dot(g011, vec3(Pf0.x, Pf1.yz));
  float n111 = dot(g111, Pf1);

  vec3 fade_xyz = fade(Pf0);
  vec4 n_z = mix(vec4(n000, n100, n010, n110), vec4(n001, n101, n011, n111), fade_xyz.z);
  vec2 n_yz = mix(n_z.xy, n_z.zw, fade_xyz.y);
  float n_xyz = mix(n_yz.x, n_yz.y, fade_xyz.x); 
  return 2.2 * n_xyz;
}

float turbulence (vec3 cord)
{
    float val = 0.0;
    float div = 1.0;
      for (int i=0; i<10; i++) {
         val += abs (cnoise (cord*div) / div);
         div *= 2.07;
      }
    return val;
}

vec3 marble(vec3 p)
{
	float temp = 6.28 * p.x + 8 * turbulence(p);;
    temp = sin(temp);
	return marble_color(temp);
}

vec3 calcColorSpeciel()
{
    float precent;
    float curY;
    if(useColorAnimationGradient == 1)
    {
        curY =mod((vOriginalPosition.y + time * MaxY),MaxY);
    }
    else
    {
        curY = vOriginalPosition.y;
    }
	if(2*curY < MaxY)
	{
		precent = 2* curY/MaxY;
		return vec3(1,0,0) *  precent + (1-precent) * vec3(0,1,0);
	}
	else
	{
		precent = 2*( curY- 0.5*MaxY)/(MaxY - 0.5*MaxY);
		return vec3(0,1,0) *  precent + (1-precent) * vec3(1,0,0);
	}
}
