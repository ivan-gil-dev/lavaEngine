#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

layout(binding = 8) uniform UniformBufferObject2{
    mat4 lightSpaceMatrix;
}ubo2;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Color;
layout(location = 2) in vec2 in_UVmap;
layout(location = 3) in vec3 in_Normals;

layout(location = 0) out vec3 frag_Color;
layout(location = 1) out vec2 frag_UVmap;
layout(location = 2) out vec3 frag_Normals;
layout(location = 3) out vec3 frag_Pos;
layout(location = 4) out vec4 frag_PosLightSpace;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main(){
    
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_Position,1.0);
    frag_Color = in_Color;
    frag_UVmap = in_UVmap;
    frag_Normals = mat3(transpose(inverse(ubo.model))) * in_Normals;
    frag_Pos = vec3(ubo.model * vec4(in_Position,1.0f));
    frag_PosLightSpace = biasMat * ubo2.lightSpaceMatrix * ubo.model * vec4(in_Position,1.0);
}