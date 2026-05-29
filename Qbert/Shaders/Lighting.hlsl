//  Blinn-Phong Lighting Calculations
// Parameters:
//   normal - the surface normal
//   worldPosition - the pixel's world position
//   toViewer - normalized vector from pixel to camera
// Returns: accumulated light color

float4 CalculateLighting(float3 normal, float3 worldPosition, float3 toViewer)
{
    float4 finalColor = float4(0, 0, 0, 1);

    for (int i = 0; i < MAX_POINT_LIGHTS; i++){
        PointLightData l = c_pointLight[i];
        if (l.isEnabled){
            float3 toLight = normalize(l.position - worldPosition);

            // Falloff
            float distance = length(worldPosition - l.position);
            float falloff = 1.0 - smoothstep(l.innerRadius, l.outerRadius, distance);

            // Diffuse lighting
            float3 diffuse = max(dot(normal, toLight) * l.lightColor, 0.0f);

            // Blinn-Phong Specular
            float3 halfVector = normalize(toViewer + toLight);
            float3 specular = c_specularColor * l.lightColor * pow(max(dot(normal, halfVector), 0.0f), c_specularPower);
            finalColor += falloff * (float4(specular, 0.0f) + float4(diffuse, 0.0f));
        }
    }

    // Ambient
    finalColor += float4(c_ambient, 0.0f);

    return finalColor;
}
