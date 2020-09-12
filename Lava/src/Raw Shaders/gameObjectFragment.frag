#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D sampler2d;
layout(binding = 2) uniform Light{ 
    vec3 lightPosition;        
    vec3 lightColor;
}light;
layout(binding = 3) uniform CameraPos{vec3 cameraPos;}Camera;
layout(binding = 4) uniform Material{
    float ambient;
    float diffuse;
    float specular;
}material;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextureCoord;
layout(location = 2) in vec3 fragNormals;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 outColor;

void main(){
    //float ambientStrength = 0.1f;
    vec3 ambient = material.ambient * light.lightColor;

    vec3 norm = normalize(fragNormals);
    vec3 lightDir = normalize(light.lightPosition - fragPos);

    float diff = max(dot(norm,lightDir), 0.0);
    vec3 diffuse = diff * light.lightColor * material.diffuse;

    //float specularStrength = 4.0f;
    vec3 viewDir = normalize(Camera.cameraPos - fragPos);
    vec3 reflectDir = reflect(-lightDir,norm);

    float spec = pow(max(dot(viewDir,reflectDir),0.0),32);
    vec3 specular = material.specular * spec * light.lightColor;

    vec3 result = (ambient + diffuse + specular) * texture(sampler2d,fragTextureCoord).rgb;
    outColor = vec4(result,1.0f);
}