#version 410 core
// this demo is based on code from here https://learnopengl.com/#!PBR/Lighting
/// @brief the vertex passed in
layout (location = 0) in vec3 VertPosition; //inVert
/// @brief the normal passed in
layout (location = 2) in vec3 VertNormal; //inNormal
/// @brief the in uv
layout (location = 1) in vec2 TexCoords; //inUV

out vec2 FragTexCoords; //TexCoords
out vec3 FragPosition; //WorldPos
out vec3 FragNormal; //Normal

uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 M;


void main()
{

    FragPosition = vec3(M * vec4(VertPosition, 1.0f));
    FragNormal=normalMatrix*VertNormal;
    FragTexCoords=10*TexCoords;
    gl_Position = MVP*vec4(VertPosition,1.0);
}
