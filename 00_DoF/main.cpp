#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <array>

#include "ogl_resource.hpp"
#include "error_handling.hpp"
#include "window.hpp"
#include "shader.hpp"


#include "scene_definition.hpp"
#include "renderer.hpp"

#include "ogl_geometry_factory.hpp"
#include "ogl_material_factory.hpp"

#include <glm/gtx/string_cast.hpp>


void toggle(const std::string &aToggleName, bool &aToggleValue) {

	aToggleValue = !aToggleValue;
	std::cout << aToggleName << ": " << (aToggleValue ? "ON\n" : "OFF\n");
}

struct Config {
	int currentSceneIdx = 0;
	bool showSolid = true;
	bool showWireframe = false;
	bool useZOffset = false;
};

void printDoFSettings(const Renderer &renderer) {
	std::cout
		<< "DoF focus distance: " << renderer.focusDistance()
		<< ", focus range: " << renderer.focusRange()
		<< ", max blur radius: " << renderer.maxBlurRadius()
		<< ", debug view: " << (renderer.showDoFDebugView() ? "ON" : "OFF")
		<< "\n";
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
		const glm::vec3 initialCameraPosition(0.0f, 10.0f, 50.0f);
		const glm::vec3 initialCameraTarget(0.0f, 0.0f, 0.0f);

		Camera camera(window.aspectRatio());
		camera.setPosition(initialCameraPosition);
		camera.lookAt(initialCameraTarget);
		SpotLight light;
		light.setPosition(glm::vec3(25.0f, 40.0f, 30.0f));
		light.lookAt(glm::vec3());



		window.onCheckInput([&camera, &mouseTracking](GLFWwindow *aWin) {
				mouseTracking.update(aWin);
				if (glfwGetMouseButton(aWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
					camera.orbit(-0.4f * mouseTracking.offset(), glm::vec3());
				}
			});
		OGLMaterialFactory materialFactory;
		materialFactory.loadShadersFromDir("./shaders/");
		materialFactory.loadTexturesFromDir("./data/textures/");

		OGLGeometryFactory geometryFactory;


		std::array<SimpleScene, 1> scenes {
			createCottageScene(materialFactory, geometryFactory),
		};

		Renderer renderer(materialFactory);
		window.setKeyCallback([&renderer, &camera, initialCameraPosition, initialCameraTarget](GLFWwindow *aWin, int key, int scancode, int action, int mods)  {
				if (action == GLFW_PRESS || action == GLFW_REPEAT) {
					switch (key) {
					case GLFW_KEY_ENTER:
						camera.setPosition(initialCameraPosition);
						camera.lookAt(initialCameraTarget);
						break;
					case GLFW_KEY_Q:
						renderer.adjustFocusDistance(-2.0f);
						printDoFSettings(renderer);
						break;
					case GLFW_KEY_E:
						renderer.adjustFocusDistance(2.0f);
						printDoFSettings(renderer);
						break;
					case GLFW_KEY_Z:
						renderer.adjustFocusRange(-0.5f);
						printDoFSettings(renderer);
						break;
					case GLFW_KEY_X:
						renderer.adjustFocusRange(0.5f);
						printDoFSettings(renderer);
						break;
					case GLFW_KEY_C:
						renderer.adjustMaxBlurRadius(-1.0f);
						printDoFSettings(renderer);
						break;
					case GLFW_KEY_V:
						renderer.adjustMaxBlurRadius(1.0f);
						printDoFSettings(renderer);
						break;
					case GLFW_KEY_B:
						renderer.toggleDoFDebugView();
						printDoFSettings(renderer);
						break;
					}
				}
			});
		window.onResize([&camera, &window, &renderer](int width, int height) {
				camera.setAspectRatio(window.aspectRatio());
				renderer.initialize(width, height);
			});


		renderer.initialize(window.size()[0], window.size()[1]);
		window.runLoop([&] {
			renderer.shadowMapPass(scenes[config.currentSceneIdx], light);
			 // renderer.shadowMapPass(scenes[config.currentSceneIdx], camera);

			renderer.clear();
			renderer.geometryPass(scenes[config.currentSceneIdx], camera, RenderOptions{"solid"});
			renderer.compositingPass(light);
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
