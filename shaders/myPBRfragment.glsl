#version 330 core
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

//cam position and exposure
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
  //thetan = halfVect dot viewer
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------



void main()
{

    //vec3 albedo     = pow(texture(albedoMap, FragTexCoords).rgb, vec3(2.2));

    vec3 normal = normalize(FragNormal); //normal
    vec3 viewer = normalize(camPos - FragPosition); //view

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);


    // reflectance equation
    vec3 Lo = vec3(0.0);
    for(int i = 0; i < 4; ++i)
    {
        // calculate per-light radiance
        vec3 light = normalize(lightPositions[i] - FragPosition); //light vector for each light
        vec3 halfVect = normalize(viewer + light); //half vector for each light
        float distance = length(lightPositions[i] - FragPosition); //distance from lights to fragment
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(normal, halfVect, roughness);
        float G   = GeometrySmith(normal, viewer, light, roughness);
        vec3 F    = fresnelSchlick(max(dot(halfVect, viewer), 0.0), F0);

        vec3 nominator    = NDF * G * F;
        float denominator = 4 * max(dot(viewer, normal), 0.0) * max(dot(light, normal), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 brdf = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(normal, light), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (note that the next IBL tutorial will replace
    // this ambient lighting with environment lighting).
    vec3 ambient = vec3(0.03) * albedo * ao;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));

    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    fragColour = vec4(color, 1.0);
}
