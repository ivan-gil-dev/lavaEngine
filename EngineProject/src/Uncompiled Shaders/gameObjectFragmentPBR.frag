#version    450
#extension  GL_ARB_separate_shader_objects : enable

const float PI = 3.14159265359;
  
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);


struct Spotlight_t{
    vec3 Position;        
    vec3 Color;

    float Ambient;
    float Diffuse;
    float Specular;

    float Constant;
    float Linear;
    float Quadrantic;
};

#define MAX_SPOTLIGHTS 100 

layout(binding = 2) uniform Light{Spotlight_t spotlight[MAX_SPOTLIGHTS];}light;

layout(binding = 3) uniform CameraPos{vec3 cameraPos;}Camera;

struct Material_t{
    float shininess;
    float metallic;
    float roughness;
    float ao;
};

layout(binding = 4) uniform Material{
    Material_t material;
}material;

struct DirectionalLight_t{
    vec3 Direction;        
    vec3 Color;

    float Ambient;
    float Diffuse;
    float Specular;
};

#define MAX_DLIGHTS 20 

layout(binding = 5) uniform DirectionalLight{DirectionalLight_t directionalLight_t[MAX_DLIGHTS];}directionalLight;



layout( push_constant ) uniform constants
{
	int MaterialID;

} PushConstants;

#define MAX_MATERIALS 256
layout(binding = 1) uniform sampler2D diffuseColorMaps[MAX_MATERIALS];
layout(binding = 6) uniform sampler2D specularColorMaps[MAX_MATERIALS];
layout(binding = 7) uniform sampler2D shadowMap;


layout(location = 0) in vec3 frag_Color;
layout(location = 1) in vec2 frag_UVmap;
layout(location = 2) in vec3 frag_Normals;
layout(location = 3) in vec3 frag_Pos;
layout(location = 4) in vec4 frag_PosLightSpace;

layout(location = 0) out vec4 outColor;

float CalculateShadow(vec4 shadowCoord)
{
    shadowCoord = shadowCoord/shadowCoord.w;
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, shadowCoord.st).r;
		if ( shadowCoord.w > 0.0 && dist < shadowCoord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}


vec3 CalculateSpotlight(Spotlight_t spotlight_p, vec3 normals_p, vec3 viewDir_p){
    if(spotlight_p.Color != vec3(0,0,0)){
        
        vec4 diffuseColor = texture(diffuseColorMaps[PushConstants.MaterialID],frag_UVmap);
        if(diffuseColor.a<0.5){
            discard;
        }

        vec3 F0 = vec3(0.04);
        F0 = mix(F0, vec3(diffuseColor), material.material.metallic);
        vec3 L0 = vec3(0.0);

        //Radiance
        vec3 L = normalize(spotlight_p.Position - frag_Pos);
        vec3 H = normalize(viewDir_p + L);
        float distance = length(spotlight_p.Position - frag_Pos);
        float attenuation = 1.0/(distance*distance);
        vec3 radiance = spotlight_p.Color * attenuation;

        //Cook-Torrance brdf
        float NDF = DistributionGGX(normals_p,H,material.material.roughness);
        float G = GeometrySmith(normals_p,viewDir_p,L,material.material.roughness);
        vec3 F = fresnelSchlick(max(dot(H,viewDir_p),0.0),F0);

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
        kD *= 1.0 - material.material.metallic;

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(normals_p,viewDir_p),0.0) 
                                * max(dot(normals_p,L), 0.0);

        vec3 specular = numerator / max(denominator, 0.001);

        specular *= material.material.shininess * vec3(texture(specularColorMaps[PushConstants.MaterialID],frag_UVmap));
        
        float NdotL = max(dot(normals_p,L),0.0);
        L0 += (kD * vec3(diffuseColor) * spotlight_p.Diffuse / PI + specular * spotlight_p.Specular) * radiance * NdotL;

	    float shadow = CalculateShadow(frag_PosLightSpace);

        return  (shadow)*L0;
    }else{
        return vec3(0,0,0);
    }
    
}

vec3 CalculateDirectionalLight(DirectionalLight_t directionalLight_p, vec3 normals_p, vec3 viewDir_p){
    if(directionalLight_p.Color != vec3(0,0,0)){
        vec3 ambient = directionalLight_p.Ambient * directionalLight_p.Color;
        vec3 lightDir = normalize(-directionalLight_p.Direction);

        float diff = max(dot(normals_p,lightDir), 0.0);
        vec3 diffuse = diff * directionalLight_p.Color * directionalLight_p.Diffuse;

        vec3 reflectDir = reflect(-lightDir,normals_p);

        float spec = pow(max(dot(viewDir_p,reflectDir),0.0),material.material.shininess);
        vec3 specular = directionalLight_p.Specular * spec * directionalLight_p.Color;

        vec4 diffuseColor = texture(diffuseColorMaps[PushConstants.MaterialID],frag_UVmap);
        //float diffuseColor = texture(shadowMap,frag_UVmap).r;

        if(diffuseColor.a<0.5){
            discard;
        }
        

        ambient *= vec3(diffuseColor);
        diffuse *= vec3(diffuseColor);
        specular *= vec3(texture(specularColorMaps[PushConstants.MaterialID],frag_UVmap));

        float shadow = CalculateShadow(frag_PosLightSpace);

        return (ambient + shadow*diffuse + specular);
    }else{
        return vec3(0,0,0);
    }
    
}

vec3 result; 

void main(){
    result = vec3(0,0,0);

    vec3 norm = normalize(frag_Normals);
    vec3 viewDir = normalize(Camera.cameraPos - frag_Pos);
    
    for (int i = 0; i < MAX_SPOTLIGHTS; i++){
        result += CalculateSpotlight(light.spotlight[i],norm,viewDir);
    }
    
    vec3 ambient = material.material.ao * vec3(texture(diffuseColorMaps[PushConstants.MaterialID],frag_UVmap));
    result += ambient;
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));  



    for (int i = 0; i < MAX_DLIGHTS; i++){
       result += CalculateDirectionalLight(directionalLight.directionalLight_t[i],norm,viewDir);
    }

    outColor = vec4(result,1.0f);
    
}


vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}