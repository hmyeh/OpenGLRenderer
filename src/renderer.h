#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h> 
#include <glm/glm.hpp>

#include "scene.h"
#include "shader.h"
#include "camera.h"
#include "irradiancemap.h"


enum RenderType {
	DEFERRED,
	FORWARD,
	DEBUG_DEPTH_CUBEMAP,
	DEBUG_IRRADIANCE_CUBEMAP,
	DEBUG_BRDF_LUT,
	COUNT // only for the count
};

class Renderer {
private:
	Camera* camera;
	unsigned int width, height;

	// G Buffer shader
	Shader geometryPassShader = Shader("g_buffer.vert", "g_buffer.frag");
	Shader deferredLightingShader = Shader("deferred_lighting.vert", "deferred_lighting.frag");

	// Forward screen shader
	Shader screenShader = Shader("screen.vert", "screen.frag");

	// Blinn phong shader
	Shader blinnPhongShader = Shader("blinn_phong.vert", "blinn_phong.frag");

	// PBR shader
	Shader pbrShader = Shader("pbr.vert", "pbr.frag");

	// Debug skybox shader
	Shader skyboxShader = Shader("skybox.vert", "skybox.frag");

	// Meshes
	ScreenQuad quad;
	Skybox skybox;

	// Gamma correction
	float gamma = 2.2;
	// Simple HDR tone mapping
	float exposure = 1.0;
	// Post-processing kernel convolution
	glm::mat3 kernel = glm::mat3(
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	);

	IrradianceMap irradianceMap = IrradianceMap("../resources/newport_loft.hdr");
	//IrradianceMap irradianceMap = IrradianceMap("../resources/Winter_Forest/WinterForest_Env.hdr");

	// GL variables
	unsigned int uboMatrices;

	// Deferred rendering GL variables
	unsigned int gBuffer;
	// Corresponding textures to attach to gBuffer
	unsigned int gPosition, gNormal, gAlbedoSpec;
	// Depth test buffer renderbufferobject
	unsigned int gRbo;

	// Post process rendering GL variables
	unsigned int screenFbo;
	// Color texture buffer
	unsigned int screenColorbuffer;
	// Depth test and stencil buffer
	unsigned int screenRbo;
public:
	Renderer(unsigned int width, unsigned int height, Camera* camera);
	~Renderer();

	void render(Scene& scene, RenderType renderType);

	void setGamma(float gamma);
	void setExposure(float exposure);
	void setKernel(glm::mat3 kernel);

private:
	// Refers to projection and view matrices
	void setupMatrices();
	void updateMatrices();
	//void setupScreenQuad();

	// g buffer
	void setupDeferredResources();
	// Simple deferred rendering
	void deferred(Scene& scene);

	// Forward rendering
	void forward(Scene& scene);

	// Setup for post-processing screen quad
	void setupPostProcResources();
	void postProcess();

	//TESTING FOR CUBEMAPS
	void setupDebugCubemapResources();
	void debugDepthCubemap(Scene& scene);
	void debugIrradiancemap(Scene& scene);
	void debugBrdfLUT(Scene& scene);

};

#endif
