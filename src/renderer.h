#ifndef RENDERER_H
#define RENDERER_H

#include <glad/gl.h> 
#include <glm/glm.hpp>

#include "scene.h"
#include "shader.h"
#include "camera.h"


enum RenderType {
	DEFERRED,
	FORWARD,
	DEBUG_DEPTH_CUBEMAP,
	COUNT // only for the count
};

class Renderer {
private:
	Camera* camera;
	unsigned int width, height;

	// G Buffer shader
	Shader geometryPassShader = Shader("../src/g_buffer.vert", "../src/g_buffer.frag");
	Shader deferredLightingShader = Shader("../src/deferred_lighting.vert", "../src/deferred_lighting.frag");

	// Forward screen shader
	Shader screenShader = Shader("../src/screen.vert", "../src/screen.frag");

	// Debug skybox shader
	Shader skyboxShader = Shader("../src/skybox.vert", "../src/skybox.frag");

	// quad
	float quadVertices[24] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};

	float skyboxVertices[108] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	// Simple HDR tone mapping
	float exposure = 1.0;
	// Post-processing kernel convolution
	glm::mat3 kernel = glm::mat3(
		0, 0, 0,
		0, 1, 0,
		0, 0, 0
	);

	// GL variables
	unsigned int uboMatrices;
	// screen quad VAO
	unsigned int quadVAO, quadVBO;
	// Skybox VAO
	unsigned int skyboxVAO, skyboxVBO;

	// Deferred rendering GL variables
	unsigned int gBuffer;
	// Corresponding textures to attach to gBuffer
	unsigned int gPosition, gNormal, gAlbedoSpec;
	// Depth test buffer renderbufferobject
	unsigned int gRbo;

	// Forward rendering GL variables
	unsigned int forwardFbo;
	// Color texture buffer
	unsigned int forwardColorbuffer;
	// Depth test and stencil buffer
	unsigned int forwardRbo;
public:
	Renderer(unsigned int width, unsigned int height, Camera* camera);
	~Renderer();

	void render(Scene& scene, RenderType renderType);

	void setExposure(float exposure);
	void setKernel(glm::mat3 kernel);

private:
	// Refers to projection and view matrices
	void setupMatrices();
	void updateMatrices();
	void setupScreenQuad();

	// g buffer
	void setupDeferredResources();
	// Simple deferred rendering
	void deferred(Scene& scene);

	// Setup for post-processing screen quad
	void setupForwardResources();
	// Forward rendering
	void forward(Scene& scene);

	//TESTING FOR CUBEMAPS
	void setupDebugCubemapResources();
	void debugDepthCubemap(Scene& scene);

	void deferredForward(Scene& scene);
};

#endif
