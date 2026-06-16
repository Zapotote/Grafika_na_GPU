#pragma once

#include <vector>

#include "camera.hpp"
#include "spotlight.hpp"
#include "framebuffer.hpp"
#include "shadowmap_framebuffer.hpp"
#include "ogl_material_factory.hpp"
#include "ogl_geometry_factory.hpp"

class QuadRenderer {
public:
	QuadRenderer()
		: mQuad(generateQuadTex())
	{}

	void render(const OGLShaderProgram &aShaderProgram, MaterialParameterValues &aParameters) const {
		aShaderProgram.use();
		aShaderProgram.setMaterialParameters(aParameters, MaterialParameterValues());
		GL_CHECK(glBindVertexArray(mQuad.vao.get()));
  		GL_CHECK(glDrawElements(mQuad.mode, mQuad.indexCount, GL_UNSIGNED_INT, reinterpret_cast<void*>(0)));
	}
protected:

	IndexedBuffer mQuad;
};

inline std::vector<CADescription> getColorNormalPositionAttachments() {
	return {
		{ GL_RGBA, GL_FLOAT, GL_RGBA },
		// To store values outside the range [0,1] we need different internal format then normal GL_RGBA
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
	};
}

inline std::vector<CADescription> getSingleColorAttachment() {
	return {
		{ GL_RGBA, GL_FLOAT, GL_RGBA32F },
	};
}


class Renderer {
public:
	Renderer(OGLMaterialFactory &aMaterialFactory)
		: mMaterialFactory(aMaterialFactory)
	{
		mCompositingShader = std::static_pointer_cast<OGLShaderProgram>(
				mMaterialFactory.getShaderProgram("compositing"));
		mDoFShader = std::static_pointer_cast<OGLShaderProgram>(
				mMaterialFactory.getShaderProgram("dof"));
		// mShadowMapShader = std::static_pointer_cast<OGLShaderProgram>(
		// 	mMaterialFactory.getShaderProgram("solid_color"));
		mShadowMapShader = std::static_pointer_cast<OGLShaderProgram>(
			mMaterialFactory.getShaderProgram("shadowmap"));
	}

	void initialize(int aWidth, int aHeight) {
		mWidth = aWidth;
		mHeight = aHeight;
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));

		mFramebuffer = std::make_unique<Framebuffer>(aWidth, aHeight, getColorNormalPositionAttachments());
		mCompositingFramebuffer = std::make_unique<Framebuffer>(aWidth, aHeight, getSingleColorAttachment());
		mShadowmapFramebuffer = std::make_unique<Framebuffer>(600, 600, getSingleColorAttachment());
		// mShadowmapFramebuffer = std::make_unique<ShadowmapFramebuffer>(600, 600);
		mCompositingParameters = {
			{ "u_diffuse", TextureInfo("diffuse", mFramebuffer->getColorAttachment(0)) },
			{ "u_normal", TextureInfo("diffuse", mFramebuffer->getColorAttachment(1)) },
			{ "u_position", TextureInfo("diffuse", mFramebuffer->getColorAttachment(2)) },
			// { "u_shadowMap", TextureInfo("shadowMap", mShadowmapFramebuffer->getDepthMap()) },
			{ "u_shadowMap", TextureInfo("shadowMap", mShadowmapFramebuffer->getColorAttachment(0)) },
		};
		mDoFParameters = {
			{ "u_scene", TextureInfo("scene", mCompositingFramebuffer->getColorAttachment(0)) },
			{ "u_position", TextureInfo("position", mFramebuffer->getColorAttachment(2)) },
		};
		updateDoFParameters();
	}

	void adjustFocusDistance(float aDelta) {
		mFocusDistance += aDelta;
		if (mFocusDistance < 0.5f) {
			mFocusDistance = 0.5f;
		}
		updateDoFParameters();
	}

	void adjustFocusRange(float aDelta) {
		mFocusRange += aDelta;
		if (mFocusRange < 0.25f) {
			mFocusRange = 0.25f;
		}
		updateDoFParameters();
	}

	void adjustMaxBlurRadius(float aDelta) {
		mMaxBlurRadius += aDelta;
		if (mMaxBlurRadius < 0.0f) {
			mMaxBlurRadius = 0.0f;
		}
		updateDoFParameters();
	}

	void toggleDoFDebugView() {
		mShowDoFDebugView = !mShowDoFDebugView;
		updateDoFParameters();
	}

	float focusDistance() const { return mFocusDistance; }
	float focusRange() const { return mFocusRange; }
	float maxBlurRadius() const { return mMaxBlurRadius; }
	bool showDoFDebugView() const { return mShowDoFDebugView; }

	void clear() {
		mFramebuffer->bind();
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	template<typename TScene, typename TCamera>
	void geometryPass(const TScene &aScene, const TCamera &aCamera, RenderOptions aRenderOptions) {
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		mFramebuffer->bind();
		mFramebuffer->setDrawBuffers();
		auto projection = aCamera.getProjectionMatrix();
		auto view = aCamera.getViewMatrix();

		std::vector<RenderData> renderData;
		for (const auto &object : aScene.getObjects()) {
			auto data = object.getRenderData(aRenderOptions);
			if (data) {
				renderData.push_back(data.value());
			}
		}

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_solidColor"] = glm::vec4(0,0,0,1);
		fallbackParameters["u_viewPos"] = aCamera.getPosition();
		fallbackParameters["u_near"] = aCamera.near();
		fallbackParameters["u_far"] = aCamera.far();
		for (const auto &data: renderData) {
			const glm::mat4 modelMat = data.modelMat;
			const MaterialParameters &params = data.mMaterialParams;
			const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(data.mShaderProgram);
			const OGLGeometry &geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			shaderProgram.use();
			shaderProgram.setMaterialParameters(params.mParameterValues, fallbackParameters);

			geometry.bind();
			geometry.draw();
		}
		mFramebuffer->unbind();
	}

	template<typename TLight>
	void compositingPass(const TLight &aLight) {
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		mCompositingFramebuffer->bind();
		mCompositingFramebuffer->setDrawBuffers();
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
		mCompositingParameters["u_lightPos"] = aLight.getPosition();
		mCompositingParameters["u_lightMat"] = aLight.getViewMatrix();
		mCompositingParameters["u_lightProjMat"] = aLight.getProjectionMatrix();
		mQuadRenderer.render(*mCompositingShader, mCompositingParameters);
		mCompositingFramebuffer->unbind();

		GL_CHECK(glViewport(0, 0, mWidth, mHeight));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
		mDoFParameters["u_texelSize"] = glm::vec2(1.0f / float(mWidth), 1.0f / float(mHeight));
		mQuadRenderer.render(*mDoFShader, mDoFParameters);
	}

	template<typename TScene, typename TLight>
	void shadowMapPass(const TScene &aScene, const TLight &aLight) {
		GL_CHECK(glEnable(GL_DEPTH_TEST));
		mShadowmapFramebuffer->bind();
		GL_CHECK(glViewport(0, 0, 600, 600));
		GL_CHECK(glClearColor(1.0f, 1.0f, 1.0f, 1.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		mShadowmapFramebuffer->setDrawBuffers();
		auto projection = aLight.getProjectionMatrix();
		auto view = aLight.getViewMatrix();

		MaterialParameterValues fallbackParameters;
		fallbackParameters["u_projMat"] = projection;
		fallbackParameters["u_viewMat"] = view;
		fallbackParameters["u_viewPos"] = aLight.getPosition();

		std::vector<RenderData> renderData;
		RenderOptions renderOptions = {"solid"};
		for (const auto &object : aScene.getObjects()) {
			auto data = object.getRenderData(renderOptions);
			if (data) {
				renderData.push_back(data.value());
			}
		}
		mShadowMapShader->use();
		for (const auto &data: renderData) {
			const glm::mat4 modelMat = data.modelMat;
			const MaterialParameters &params = data.mMaterialParams;
			const OGLShaderProgram &shaderProgram = static_cast<const OGLShaderProgram &>(data.mShaderProgram);
			const OGLGeometry &geometry = static_cast<const OGLGeometry&>(data.mGeometry);

			fallbackParameters["u_modelMat"] = modelMat;
			fallbackParameters["u_normalMat"] = glm::mat3(modelMat);

			mShadowMapShader->setMaterialParameters(fallbackParameters, {});

			geometry.bind();
			geometry.draw();
		}



		mShadowmapFramebuffer->unbind();
	}

protected:
	void updateDoFParameters() {
		mDoFParameters["u_focusDistance"] = mFocusDistance;
		mDoFParameters["u_focusRange"] = mFocusRange;
		mDoFParameters["u_maxBlurRadius"] = mMaxBlurRadius;
		mDoFParameters["u_debugView"] = mShowDoFDebugView ? 1 : 0;
	}

	int mWidth = 100;
	int mHeight = 100;
	float mFocusDistance = 50.0f;
	float mFocusRange = 2.5f;
	float mMaxBlurRadius = 18.0f;
	bool mShowDoFDebugView = false;
	std::unique_ptr<Framebuffer> mFramebuffer;
	std::unique_ptr<Framebuffer> mCompositingFramebuffer;
	std::unique_ptr<Framebuffer> mShadowmapFramebuffer;
	// std::unique_ptr<ShadowmapFramebuffer> mShadowmapFramebuffer;
	MaterialParameterValues mCompositingParameters;
	MaterialParameterValues mDoFParameters;
	QuadRenderer mQuadRenderer;
	std::shared_ptr<OGLShaderProgram> mCompositingShader;
	std::shared_ptr<OGLShaderProgram> mDoFShader;
	std::shared_ptr<OGLShaderProgram> mShadowMapShader;
	OGLMaterialFactory &mMaterialFactory;
};
