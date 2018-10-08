#version 420 core
// This code is based on code from here https://learnopengl.com/#!PBR/Lighting

out vec4 fragColour;

in vec2 FragTexCoords; //TexCoords
in vec3 FragPosition; //WorldPos
in vec3 FragNormal; //Normal
in mat3 TBN;

//material parameters
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform float texCoordScale = 10.0f;


// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform vec3 camPos;
uniform float exposure;

//PI for albedo
const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------



void main()
{


    vec3 np = normalize(FragNormal);

    vec3 viewer = normalize(camPos - FragPosition);


    // reflectance at normal incidence
    // if it's a metal, their albedo color is F0
    vec3 F0 = albedo;

    // reflectance equation
    // ADDED 4 MORE LIGHTS
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 6; ++i)
    {
        // calculate per-light radiance
        vec3 light = normalize(lightPositions[i] - FragPosition);
        vec3 halfVect = normalize(viewer + light);
        float distance = length(lightPositions[i] - FragPosition);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        //BRDF
        float NDF = DistributionGGX(np, halfVect, roughness);
        float G   = GeometrySmith(np, viewer, light, roughness);
        vec3 F    = fresnelSchlick(max(dot(halfVect, viewer), 0.0), vec3(1.0f, 1.0f, 1.0f));

        vec3 nominator    = NDF * G * F;
        float denominator = 6 * max(dot(viewer, np), 0.0) * max(dot(light, np), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 brdf = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(np, light), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    vec3 ambient = vec3(0.3) * albedo * ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));


    fragColour = vec4(color, 1.0);
}
