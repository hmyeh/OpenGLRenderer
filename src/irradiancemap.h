#ifndef IRRADIANCE_MAP_H
#define IRRADIANCE_MAP_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stb_image.h>

#include "shader.h"
#include "mesh.h"

#include <string>
#include <iostream>


class IrradianceMap {
private:
    Shader equirectangularToCubemapShader = Shader("cubemap.vert", "cubemap.frag");
    Shader irradianceShader = Shader("cubemap.vert", "irradiance_convolution.frag");
    Shader prefilterShader = Shader("cubemap.vert", "prefilter_convolution.frag");
    Shader brdfShader = Shader("precompute_brdf.vert", "precompute_brdf.frag");

    // static skybox
    Skybox skybox;
    ScreenQuad quad;

    // framebuffer
    unsigned int captureFBO, captureRBO;
    // Image based lighting maps
    unsigned int envCubemap;
    unsigned int irradianceMap;
    unsigned int prefilterMap;
    unsigned int brdfLUT;

    void setupFrameBuffer();
    void generateCubemapTexture();
public:
    IrradianceMap();
    IrradianceMap(std::string filepath);
    ~IrradianceMap();

    void loadHDRToCubemap(std::string filepath);

    // Diffuse convolution of cubemap
    void convoluteIrradianceMap();
    void convolutePrefilterMap();
    void computeBrdfLUT();

    
public:
   
    // Assumes hdr has been loaded as cubemap texture
    void precompute();
    void bind(Shader& shader);
    
    unsigned int& getIrradianceMap() {
        return irradianceMap;
    }
    unsigned int& getPrefilterMap() {
        return prefilterMap;
    }
    unsigned int& getBrdfLUT() {
        return brdfLUT;
    }
    unsigned int& getEnvironmentMap() {
        return envCubemap;
    }

};


#endif
