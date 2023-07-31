#version 460 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform mat3 kernel;
uniform float exposure;
uniform float gamma;

const float offset = 1.0 / 300.0;  

// This fragment shader is for postprocessing
// https://gamedev.stackexchange.com/questions/147952/what-is-the-order-of-postprocessing-effects
void main()
{ 
    vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

    // Convolution
    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++)
            col += sampleTex[i * 3 + j] * kernel[i][j];
    
    // Simple exposure Tone mapping HDR
    vec3 mapped = vec3(1.0) - exp(-col * exposure);
    // Gamma correction
    mapped = pow(mapped, vec3(1.0 / gamma));

    FragColor = vec4(mapped, 1.0);
} 
