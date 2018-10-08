#version 420 core
// This code is based on code from here https://learnopengl.com/#!PBR/Lighting

out vec4 fragColour;

in vec2 FragTexCoords; //TexCoords
in vec3 FragPosition; //WorldPos
in vec3 FragNormal; //Normal
in mat3 TBN;

//material parameters
uniform vec3 albedo;
uniform float ao;

uniform float texCoordScale = 10.0f;

uniform sampler2D textMap;
uniform sampler2D glossMap;
uniform sampler2D renderedTexture;
uniform samplerCube envMap;

// Set the maximum environment level of detail (cannot be queried from GLSL apparently)
uniform int envMaxLOD = 8;

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

const vec2 k_size = vec2(2.0,0.0);
const ivec2 k_offset = ivec2(-1, 0);

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}


float fbm (in vec2 st) {
    // Initial values
float value = -0.072;
float amplitud = 0.308;
    float frequency = 0.;
    //
    // Loop of octaves
    for (int i = 0; i < 7; i++) {
        value += amplitud * noise(st);
        st *= 8.064;
        amplitud *= 0.612;
    }
    return value;
}


void main()
{
    vec3 height_map = texture2D(renderedTexture, FragTexCoords).rgb;

    vec3 np = normalize(FragNormal);

    vec3 viewer = normalize(camPos - FragPosition);

    vec2 st = FragTexCoords*0.1;

    float v = smoothstep(0.5, 0.51, fbm(st * 12.9))+0.1;

    float metallic2 =1-v;
    float roughness2 =v;

    vec3 lookup = reflect(viewer,np);

    // This call actually finds out the current LOD
    float lod = textureQueryLod(envMap, lookup).x;

    // Determine the gloss value from our input texture, and scale it by our LOD resolution
    float gloss = (1.0 - texture(glossMap, FragTexCoords*2).r) * float(envMaxLOD);

    // This call determines the current LOD value for the texture map
    vec4 colour = textureLod(envMap, lookup, 1.0);

    // This call just retrieves whatever you want from the environment map
    //vec4 colour = texture(envMap, lookup);


    //ASTEA DOUA MERG CU UNS TRI!!!!!
    //vec3 text = texture2D(textMap, (FragTexCoords*0.5-vec2(0.0, 0.4))).rgb;
    vec3 text = texture2D(textMap, FragTexCoords*0.4-vec2(0.5f, 0.0f)).rgb;
    //POATE SCRIS MAI MARE!!!

    //vec3 albedo2 = vec3( max((albedo.x-text.x), 0), max((albedo.y-text.y), 0), max((albedo.z-text.z), 0) );
    //INMULTIT CU DISPLACEMENT
    float v2 = smoothstep(0.5, 0.51, fbm(st * 12.9));
    vec3 albedo2 = vec3( min((albedo.x+text.x), 1), min((albedo.y+text.y), 1), min((albedo.z+text.z), 1) )-v2*vec3(0.5, 0.5, 0.5);

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
        float NDF = DistributionGGX(np, halfVect, roughness2);
        float G   = GeometrySmith(np, viewer, light, roughness2);
        vec3 F    = fresnelSchlick(max(dot(halfVect, viewer), 0.0), vec3(1.0f, 1.0f, 1.0f));
           
        vec3 nominator    = NDF * G * F; 
        float denominator = 6 * max(dot(viewer, np), 0.0) * max(dot(light, np), 0.0) + 0.001; // 0.001 to prevent divide by zero.
        vec3 brdf = nominator / denominator;
        
        // kS is equal to Fresnel
        vec3 kS = F;
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;

        kD *= 1.0 - metallic2;

        // scale light by NdotL
        float NdotL = max(dot(np, light), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * (albedo2*roughness2) / PI + brdf) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }   

    vec3 ambient = vec3(0.3) * albedo2 * ao;

    vec3 color = ambient + Lo;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    //vec3 intermediate = texture2D(glossMap, FragTexCoords).rgb;

    fragColour = vec4(color, 1.0);
}
