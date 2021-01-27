#version    450
#extension  GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D sampler2d;

struct Spotlight{
    vec3 Position;        
    vec3 Color;
};

layout(binding = 2) uniform Light{Spotlight spotlight;}light;

layout(binding = 3) uniform CameraPos{vec3 cameraPos;}Camera;

layout(binding = 4) uniform Material{
    float ambient;
    float diffuse;
    float specular;
}material;

layout(location = 0) in vec3 frag_Color;
layout(location = 1) in vec2 frag_UVmap;
layout(location = 2) in vec3 frag_Normals;
layout(location = 3) in vec3 frag_Pos;

layout(location = 0) out vec4 outColor;

vec3 CalculateSpotlight(Spotlight spot, vec3 normals, vec3 ViewDir, vec3 fragPos){
    vec3 ambient = material.ambient * spot.Color;
    vec3 lightDir = normalize(spot.Position - fragPos);

    float diff = max(dot(normals,lightDir), 0.0);
    vec3 diffuse = diff * spot.Color * material.diffuse;

    vec3 reflectDir = reflect(-lightDir,normals);

    float spec = pow(max(dot(ViewDir,reflectDir),0.0),32);
    vec3 specular = material.specular * spec * spot.Color;
    return (ambient + diffuse + specular);
}

void main(){
    vec3 norm = normalize(frag_Normals);
    vec3 viewDir = normalize(Camera.cameraPos - frag_Pos);
    vec3 result;
    result += CalculateSpotlight(light.spotlight,norm,viewDir,frag_Pos);
    result *= vec3(texture(sampler2d,frag_UVmap));
    outColor = vec4(result,1.0f);
    
}