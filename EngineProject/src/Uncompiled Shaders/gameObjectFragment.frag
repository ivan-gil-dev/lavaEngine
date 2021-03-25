#version    450
#extension  GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D sampler2d;

struct Spotlight_t{
    vec3 Position;        
    vec3 Color;
};

#define MAX_SPOTLIGHTS 10 

layout(binding = 2) uniform Light{Spotlight_t spotlight[MAX_SPOTLIGHTS];}light;

layout(binding = 3) uniform CameraPos{vec3 cameraPos;}Camera;

struct Material_t{
    float ambient;
    float diffuse;
    float specular;
};

layout(binding = 4) uniform Material{
    Material_t material;
}material;

layout(location = 0) in vec3 frag_Color;
layout(location = 1) in vec2 frag_UVmap;
layout(location = 2) in vec3 frag_Normals;
layout(location = 3) in vec3 frag_Pos;

layout(location = 0) out vec4 outColor;

vec3 CalculateSpotlight(Spotlight_t spotlight_p, vec3 normals_p, vec3 viewDir_p){
    if(spotlight_p.Color != vec3(0,0,0)){
        vec3 ambient = material.material.ambient * spotlight_p.Color;
        vec3 lightDir = normalize(spotlight_p.Position - frag_Pos);

        float diff = max(dot(normals_p,lightDir), 0.0);
        vec3 diffuse = diff * spotlight_p.Color * material.material.diffuse;

        vec3 reflectDir = reflect(-lightDir,normals_p);

        float spec = pow(max(dot(viewDir_p,reflectDir),0.0),32);
        vec3 specular = material.material.specular * spec * spotlight_p.Color;
        
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
    
    result *= vec3(texture(sampler2d,frag_UVmap));


    outColor = vec4(result,1.0f);
    
}