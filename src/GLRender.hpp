#pragma once
#include "AbstractRender.hpp"
namespace BEbraEngine {
	class GLWindow;
}
namespace BEbraEngine {

	class GLRender : public AbstractRender
	{
	public:
		void create(BaseWindow* window) override;
		RenderBuffer* createIndexBuffer(std::vector<uint32_t> indices) override;
		RenderBuffer* createVertexBuffer(std::vector<Vertex> vertices) override;
		RenderBuffer* createUniformBuffer(size_t size) override;
		RenderBuffer* createStorageBuffer(size_t size) override;
		void destroyBuffer(RenderBuffer* buffer) override;
		void selectMainCamera(Camera* camera) override;
		void addObject(std::shared_ptr<RenderObject> object) override;
		void addLight(std::shared_ptr<PointLight> light) override;
		void removeObject(std::shared_ptr<RenderObject> object) override;
		void removeLight(std::shared_ptr<PointLight> light) override;
		void addGlobalLight(std::shared_ptr<DirectionLight> globalLight) override;
		void addCamera(std::shared_ptr<Camera> camera) override;
		void removeCamera(std::shared_ptr<Camera> camera) override;

		IRenderObjectFactory* getRenderObjectFactory() override;
		void drawFrame() override;
		Type getType() override;
		size_t alignmentBuffer(size_t originalSize, AbstractRender::TypeBuffer type) override;

	private:
		GLWindow* window;


	};

}

