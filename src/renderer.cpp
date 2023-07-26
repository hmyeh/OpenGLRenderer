#include "renderer.h"

#include <glm/gtc/type_ptr.hpp>

Renderer::Renderer(unsigned int width, unsigned int height, Camera* camera) : width(width), height(height), camera(camera) {
	this->setupMatrices();
	this->setupScreenQuad();

	this->setupDeferredResources();
	this->setupForwardResources();

	this->setupDebugCubemapResources();
}

Renderer::~Renderer() {
	// Clean up deferred rendering
	glDeleteFramebuffers(1, &gBuffer);
	glDeleteTextures(1, &gPosition);
	glDeleteTextures(1, &gNormal);
	glDeleteTextures(1, &gAlbedoSpec);
	glDeleteRenderbuffers(1, &gRbo);

	// Clean up forward rendering
	glDeleteFramebuffers(1, &forwardFbo);
	glDeleteTextures(1, &forwardColorbuffer);
	glDeleteRenderbuffers(1, &forwardRbo);

	// Clean up VAOs and VBOs
	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVBO);
}

void Renderer::render(Scene& scene, RenderType renderType) {
	switch (renderType) {
	case RenderType::DEFERRED:
		this->deferred(scene);
		break;
	case RenderType::FORWARD:
		this->forward(scene);
		break;
	case RenderType::DEBUG_DEPTH_CUBEMAP:
		this->debugDepthCubemap(scene);
		break;
	default:
		std::cerr << "Rendering Type does not exist" << std::endl;
	};
}

void Renderer::setGamma(float gamma) {
	this->gamma = gamma;
}

void Renderer::setExposure(float exposure) {
	this->exposure = exposure;
}

void Renderer::setKernel(glm::mat3 kernel) {
	this->kernel = kernel;
}

// Refers to projection and view matrices
void Renderer::setupMatrices() {
	glGenBuffers(1, &uboMatrices);

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
}

void Renderer::updateMatrices() {
	// view/projection transformations
	glm::mat4 projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);
	glm::mat4 view = camera->GetViewMatrix();

	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void Renderer::setupScreenQuad() {
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

// g buffer
void Renderer::setupDeferredResources() {
	// Create G-buffer for deferred rendering
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

	// - position color buffer
	glGenTextures(1, &gPosition);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

	// - normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

	// - color + specular color buffer
	glGenTextures(1, &gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

	// - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	// renderbuffer object not readable faster though than texture
	glGenRenderbuffers(1, &gRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, gRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gRbo);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "G-buffer failed to be created/completed" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}



// Simple deferred rendering
void Renderer::deferred(Scene& scene) {
	// First shadow map then normal rendering passes
	scene.computeShadowMaps();

	//Reset viewport size
	glViewport(0, 0, width, height);

	// Update 
	this->updateMatrices();

	// geometry pass
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	geometryPassShader.use();
	scene.draw(geometryPassShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// deferred lighting pass
	// Second pass
	glBindFramebuffer(GL_FRAMEBUFFER, forwardFbo); // back to default 0
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gPosition);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

	deferredLightingShader.use();
	deferredLightingShader.setInt("gPosition", 0);
	deferredLightingShader.setInt("gNormal", 1);
	deferredLightingShader.setInt("gAlbedoSpec", 2);
	deferredLightingShader.setVec3("viewPos", camera->Position);
	// Bind shadowmap data
	scene.bindLightsData(deferredLightingShader);

	// Draw to screen
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// FORWARD RENDERING PART
	// Copy Depth buffer from g-Buffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, forwardFbo); // write to default framebuffer
	glBlitFramebuffer(
		0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST
	);
	glBindFramebuffer(GL_FRAMEBUFFER, forwardFbo);

	// Special shader drawings
	scene.specialShadersDraw();

	this->postProcess();
}

// Setup for post-processing screen quad
void Renderer::setupForwardResources() {
	// Create framebuffer
	glGenFramebuffers(1, &forwardFbo);
	glBindFramebuffer(GL_FRAMEBUFFER, forwardFbo);

	// texture for framebuffer
	glGenTextures(1, &forwardColorbuffer);
	glBindTexture(GL_TEXTURE_2D, forwardColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// attach texture to framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, forwardColorbuffer, 0);

	// renderbuffer object not readable faster though than texture
	glGenRenderbuffers(1, &forwardRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, forwardRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, forwardRbo);


	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cerr << "Framebuffer failed to be created/completed" << std::endl;

	// Setup screenshader
	screenShader.use();
	screenShader.setInt("screenTexture", 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::forward(Scene& scene) {
	// First shadow map then normal rendering passes
	scene.computeShadowMaps();

	//Reset viewport size
	glViewport(0, 0, width, height);

	this->updateMatrices();

	// first pass
	glBindFramebuffer(GL_FRAMEBUFFER, forwardFbo);
	glEnable(GL_DEPTH_TEST);

	//glEnable(GL_STENCIL_TEST);
	//glEnable(GL_BLEND);

	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	blinnPhongShader.use();
	blinnPhongShader.setVec3("viewPos", camera->Position);
	// Bind shadowmap data
	scene.bindLightsData(deferredLightingShader);
	// Draw scene with blinn phong shader
	scene.draw(blinnPhongShader);
	
	// Special shader drawings
	scene.specialShadersDraw();

	this->postProcess();
}

void Renderer::postProcess() {
	// Postprocessing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	//glEnable(GL_FRAMEBUFFER_SRGB);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	screenShader.use();
	screenShader.setMat3("kernel", kernel);
	screenShader.setFloat("exposure", exposure);
	screenShader.setFloat("gamma", gamma);
	glBindVertexArray(quadVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, forwardColorbuffer);//forwardColorbuffer); //scene.dir_light.getDepthMap());//
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//glDisable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST);

}

//TESTING FOR CUBEMAPS
void Renderer::setupDebugCubemapResources() {
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// skybox VAO
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void Renderer::debugDepthCubemap(Scene& scene) {
	// First shadow map then normal rendering passes
	scene.computeShadowMaps();
	//Reset viewport size
	glViewport(0, 0, width, height);

	this->updateMatrices();

	// draw skybox as last
	glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
	skyboxShader.use();
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, scene.getDepthCubemap(0));
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default
}

void Renderer::deferredForward(Scene& scene) {
	// TODO: render deferred and afterwards forward rendering for transparent objects etc.
}
