// Filename: Quad.frag
#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;
float InnerRadius = 0.5;    // inside radius
float OuterRadius = 0.48;    // outside radius
vec3 PaintColor = vec3(1.0, 0.5, 0.0);

vec3 Circle(vec2 uv, bool isSmooth)
{
    uv -= 0.5;
    float l = length(uv);
    float weight = isSmooth ? step(0.5, l) : smoothstep(0.4, 0.5, l);
    return mix(vec3(1., 0., 0.), vec3(1., 1., 0.), weight);
}

void DrawTexture()
{
    
}
void main() 
{
    vec3 c = Circle(fragTexCoord, true);
    outColor = vec4(c, 1.0);
return;
    vec2 uv = fragTexCoord;//fragCoord/iResolution.xy;
    vec3 col = 0.5 + 0.5*cos(uv.xyx+vec3(0,2,4));

    col *= 1.0 + 0.1*mod( floor(fragTexCoord.x) + floor(fragTexCoord.y), 2.0 );
    outColor = vec4(col, 1.0);
    float weight = 0.0f;
    float dx     = fragTexCoord.x - 0.5;
    float dy     = fragTexCoord.y - 0.5;
    float length = sqrt(dx * dx + dy * dy);
    weight = smoothstep( InnerRadius, OuterRadius, length );

    outColor = mix( texture(texSampler, fragTexCoord), vec4(outColor.xyz, 0.0), weight);
return;
/*
    vec2 iResolution = vec2(400, 300);
    vec2 p = fragTexCoord;//(fragTexCoord-.5*iResolution.xy) / iResolution.y;
    
    vec3 col = vec3(0.9,0.9,.0);
    col *= 1.0 + 0.1*mod( floor(p.x) + floor(p.y), 2.0 );
    outColor = vec4(col, 1.0);

return;
    // Pass through fragment color input as output
    //outColor = vec4(fragColor, 1.0);
    outColor = texture(texSampler, fragTexCoord);
    outColor = vec4(1.0, 1.0, 0.0, 1.0);
    float weight = 0.0f;
    float dx     = fragTexCoord.x - 0.5;
    float dy     = fragTexCoord.y - 0.5;
    float length = sqrt(dx * dx + dy * dy);
    outColor = texture(texSampler, vec2(dx, dy));
return;    
    // Calculate the weights
    weight = smoothstep( InnerRadius, OuterRadius, length );

    outColor = mix( vec4(PaintColor, 1.0), vec4(PaintColor, 0.0), weight);
    */
}