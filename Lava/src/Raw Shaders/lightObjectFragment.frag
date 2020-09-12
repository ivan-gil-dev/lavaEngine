#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTextureCoord;
layout(location = 2) in vec3 fragNormals;

layout(binding = 2) uniform LightColor{vec3 lightColor;}lColor;
layout(binding = 1) uniform sampler2D sampler2d;

layout(location = 0) out vec4 outColor;
void main(){
    outColor = vec4(lColor.lightColor,1.0f) * texture(sampler2d,fragTextureCoord);
}