#include <iostream>
#include <cassert>
#include <chrono>
#include <memory>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#include "ogl_resource.hpp"
#include "error_handling.hpp"
#include "window.hpp"
#include "shader.hpp"

#include "particle_system.hpp"
#include "renderer.hpp"

#include "ogl_geometry_factory.hpp"
#include "ogl_material_factory.hpp"

struct Config {
	float elapsedTime = 0.0f;
	std::shared_ptr<ParticleSystem> currentParticleSystem = nullptr;
};

void setParticleEffect(Config &config, ParticleEffect effect, const std::string &name) {
	if (config.currentParticleSystem) {
		config.currentParticleSystem->getEmitter()->setEffect(effect);
		std::cout << "Particle effect: " << name << "\n";
	}
}

int main() {
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	try {
		auto window = Window();
		MouseTracking mouseTracking;
		Config config;
		Camera camera(window.aspectRatio());
		camera.setPosition(glm::vec3(0.0f,0.0f, -3.0f));
		camera.lookAt(glm::vec3());
		window.onResize([&camera, &window](int width, int height) {
				camera.setAspectRatio(window.aspectRatio());
			});

		window.onCheckInput([&camera, &mouseTracking](GLFWwindow *aWin) {
				mouseTracking.update(aWin);
				if (glfwGetMouseButton(aWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
					camera.orbit(-0.4f * mouseTracking.offset(), glm::vec3());
				}
			});
		window.setKeyCallback([&config, &camera](GLFWwindow *aWin, int key, int scancode, int action, int mods) {
				if (action == GLFW_PRESS) {
					switch (key) {
					case GLFW_KEY_ENTER:
						camera.setPosition(glm::vec3(0.0f,0.0f, -3.0f));
						camera.lookAt(glm::vec3());
						break;
					case GLFW_KEY_1:
						setParticleEffect(config, ParticleEffect::Fire, "Fire");
						break;
					case GLFW_KEY_2:
						setParticleEffect(config, ParticleEffect::Bubbles, "Bubbles");
						break;
					case GLFW_KEY_3:
						setParticleEffect(config, ParticleEffect::Snow, "Snow");
						break;
					case GLFW_KEY_4:
						setParticleEffect(config, ParticleEffect::Rain, "Rain");
						break;
					case GLFW_KEY_5:
						setParticleEffect(config, ParticleEffect::Smoke, "Smoke");
						break;
					case GLFW_KEY_6:
						setParticleEffect(config, ParticleEffect::Mist, "Mist");
						break;
					case GLFW_KEY_7:
						setParticleEffect(config, ParticleEffect::Sparks, "Sparks");
						break;
					case GLFW_KEY_8:
						setParticleEffect(config, ParticleEffect::Fountain, "Fountain");
						break;
					case GLFW_KEY_9:
						setParticleEffect(config, ParticleEffect::Explosion, "Explosion");
						break;
					case GLFW_KEY_0:
						setParticleEffect(config, ParticleEffect::Lightning, "Lightning");
						break;
					case GLFW_KEY_M:
						setParticleEffect(config, ParticleEffect::Magic, "Magic");
						break;
					}
				}
			});

		OGLMaterialFactory materialFactory;
		materialFactory.loadShadersFromDir("./shaders/");
		materialFactory.loadTexturesFromDir("./data/textures/");

		OGLGeometryFactory geometryFactory;

		// Create particle emitter and system
		auto particleEmitter = std::make_shared<ParticleEmitter>(glm::vec3(0.0f, -0.7f, 0.0f));
		auto particleSystem = std::make_shared<ParticleSystem>(particleEmitter);
		particleSystem->setName("PARTICLE_SYSTEM");
		particleSystem->addMaterial(
			"solid",
			MaterialParameters(
				"particle",
				RenderStyle::Solid,
				{
					{"u_solidColor", glm::vec4(1.0f, 0.8f, 0.5f, 1.0f)}
				}
			)
		);
		particleSystem->addMaterial(
			"wireframe",
			MaterialParameters(
				"particle",
				RenderStyle::Solid,
				{}
			)
		);
		particleSystem->prepareRenderData(materialFactory, geometryFactory);

		config.currentParticleSystem = particleSystem;

		Renderer renderer(materialFactory);

		renderer.initialize();

		auto lastTime = std::chrono::high_resolution_clock::now();

		window.runLoop([&] {
			// Update time delta
			auto now = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::microseconds>(now - lastTime);
			float deltaTime = duration.count() / 1000000.0f;
			lastTime = now;
			config.elapsedTime += deltaTime;

			if (config.currentParticleSystem) {
				config.currentParticleSystem->update(deltaTime);
			}

			renderer.clear();
			GL_CHECK(glDisable(GL_POLYGON_OFFSET_LINE));
			GL_CHECK(glPolygonOffset(0.0f, 0.0f));
			GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
			if (config.currentParticleSystem) {
				auto projection = camera.getProjectionMatrix();
				auto view = camera.getViewMatrix();

				auto renderData = config.currentParticleSystem->getRenderData(RenderOptions{"solid"});
				if (renderData) {
					const glm::mat4 &modelMat = renderData->modelMat;
					const MaterialParameters &params = renderData->mMaterialParams;
					const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(renderData->mShaderProgram);
					const OGLGeometry &geometry = static_cast<const OGLGeometry&>(renderData->mGeometry);

					MaterialParameterValues fallbackParameters;
					fallbackParameters["u_projMat"] = projection;
					fallbackParameters["u_viewMat"] = view;
					fallbackParameters["u_solidColor"] = glm::vec4(0,0,0,1);
					fallbackParameters["u_viewPos"] = camera.getPosition();
					fallbackParameters["u_modelMat"] = modelMat;
					fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

					shaderProgram.use();
					shaderProgram.setMaterialParameters(params.mParameterValues, fallbackParameters);
					geometry.bind();
					GL_CHECK(glDepthMask(GL_FALSE));
					geometry.draw();
					GL_CHECK(glDepthMask(GL_TRUE));
				}
			}
		});
	} catch (ShaderCompilationError &exc) {
		std::cerr
			<< "Shader compilation error!\n"
			<< "Shader type: " << exc.shaderTypeName()
			<<"\nError: " << exc.what() << "\n";
		return -3;
	} catch (OpenGLError &exc) {
		std::cerr << "OpenGL error: " << exc.what() << "\n";
		return -2;
	} catch (std::exception &exc) {
		std::cerr << "Error: " << exc.what() << "\n";
		return -1;
	}

	glfwTerminate();
	return 0;
}
