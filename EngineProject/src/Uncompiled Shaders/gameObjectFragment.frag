#version    450
#extension  GL_ARB_separate_shader_objects : enable



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

#define MAX_SPOTLIGHTS 10 

layout(binding = 2) uniform Light{Spotlight_t spotlight[MAX_SPOTLIGHTS];}light;

layout(binding = 3) uniform CameraPos{vec3 cameraPos;}Camera;

struct Material_t{
    float shininess;
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

#define MAX_DLIGHTS 5 

layout(binding = 5) uniform DirectionalLight{DirectionalLight_t directionalLight_t[MAX_DLIGHTS];}directionalLight;



layout( push_constant ) uniform constants
{
	int MaterialID;

} PushConstants;

#define MAX_MATERIALS 256
layout(binding = 1) uniform sampler2D diffuseColorMaps[MAX_MATERIALS];
layout(binding = 6) uniform sampler2D specularColorMaps[MAX_MATERIALS];

layout(location = 0) in vec3 frag_Color;
layout(location = 1) in vec2 frag_UVmap;
layout(location = 2) in vec3 frag_Normals;
layout(location = 3) in vec3 frag_Pos;

layout(location = 0) out vec4 outColor;

vec3 CalculateSpotlight(Spotlight_t spotlight_p, vec3 normals_p, vec3 viewDir_p){
    if(spotlight_p.Color != vec3(0,0,0)){

        vec3 ambient = spotlight_p.Ambient * spotlight_p.Color;

        vec3 lightDir = normalize(spotlight_p.Position - frag_Pos);
        float diff = max(dot(normals_p,lightDir), 0.0);
        vec3 diffuse = diff * spotlight_p.Color * spotlight_p.Diffuse;

        vec3 reflectDir = reflect(-lightDir,normals_p);
        float spec = pow(max(dot(viewDir_p,reflectDir),0.0),material.material.shininess);
        vec3 specular = spotlight_p.Specular * spec * spotlight_p.Color;

        float distance = length(spotlight_p.Position - frag_Pos);
        float attenuation = 1.0/(spotlight_p.Constant + spotlight_p.Linear*distance + spotlight_p.Quadrantic*distance*distance);

        vec4 diffuseColor = texture(diffuseColorMaps[PushConstants.MaterialID],frag_UVmap);

        if(diffuseColor.a<0.5){
            discard;
        }
       
        ambient *= vec3(diffuseColor);
        diffuse *= vec3(diffuseColor);
        specular *= vec3(texture(specularColorMaps[PushConstants.MaterialID],frag_UVmap));
  
        return (ambient*attenuation + diffuse*attenuation + specular*attenuation);
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

        if(diffuseColor.a<0.5){
            discard;
        }
        

        ambient *= vec3(diffuseColor);
        diffuse *= vec3(diffuseColor);
        specular *= vec3(texture(specularColorMaps[PushConstants.MaterialID],frag_UVmap));

        return (ambient + diffuse + specular);
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
    

     for (int i = 0; i < MAX_DLIGHTS; i++){
        result += CalculateDirectionalLight(directionalLight.directionalLight_t[i],norm,viewDir);
     }

    outColor = vec4(result,1.0f);
    
}