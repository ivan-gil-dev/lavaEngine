#version 450
#extension GL_ARB_separate_shader_objects : enable
layout(location = 0) in vec3 inPos;
layout(binding = 0) uniform UBO{
    mat4 model;
    mat4 projection;
}ubo;
layout(location = 0) out vec3 uv;
void main(){
    uv = inPos;

    uv.xy *= -1;
    
    gl_Position = ubo.projection * ubo.model * vec4(inPos,1.0f) ;
}