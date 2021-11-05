#pragma once
#include <vector>
#include "IRenderObjectFactory.hpp"
#include "RenderObject.hpp"
//TODO: ��� �����, ������������ ��� ��� ��������� ��� ������� ���� ��� ����������� �����, ������ ������� ���������� ����. ���
namespace BEbraEngine {
	class Vertex;
	class Camera;
	class BaseWindow;
}
namespace BEbraEngine {
	class RenderObject;
	
	class AbstractRender {
	public:
		enum class Type {
			Vulkan,
			DirectX,
			OpenGL_ES
		};
	public:
		virtual void Create(BaseWindow* window) = 0;
		virtual RenderBuffer* CreateIndexBuffer(std::vector<uint32_t> indices) = 0;
		virtual RenderBuffer* CreateVertexBuffer(std::vector<Vertex> vertices) = 0;
		virtual RenderBuffer* CreateUniformBuffer(size_t size) = 0;
		virtual void DestroyBuffer(RenderBuffer* buffer) = 0;
		
		//TODO: ������������ �����
		virtual void InitCamera(Camera* alloced_camera) = 0;
		virtual void AddObject(std::weak_ptr<RenderObject> object) = 0;
		virtual void addLight(std::weak_ptr<Light> light) = 0;

		virtual std::shared_ptr<IRenderObjectFactory> getRenderObjectFactory() = 0;
		virtual Type getType() = 0;
		virtual ~AbstractRender() {}
	};
}