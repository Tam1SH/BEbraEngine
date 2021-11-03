#pragma once
#include <vector>
#include "IRenderObjectFactory.hpp"
//TODO: ��� �����, ������������ ��� ��� ��������� ��� ������� ���� ��� ����������� �����, ������ ������� ���������� ����. ���
namespace BEbraEngine {
	class RenderBuffer;
	class Vertex;
	class Camera;
	class BaseWindow;
	class RenderObject;

}
namespace BEbraEngine {
	class RenderObject;
	class AbstractRender {
	protected:
		std::unique_ptr<IRenderObjectFactory> factory;
	public:
		virtual void Create(BaseWindow* window) = 0;
		virtual RenderBuffer* CreateIndexBuffer(std::vector<uint32_t> indices) = 0;
		virtual RenderBuffer* CreateVertexBuffer(std::vector<Vertex> vertices) = 0;
		virtual RenderBuffer* CreateUniformBuffer(size_t size) = 0;
		virtual void DestroyBuffer(RenderBuffer* buffer) = 0;

		//TODO: ������������ �����
		virtual void InitCamera(Camera* alloced_camera) = 0;
		virtual void AddObject(std::weak_ptr<RenderObject> object) = 0;

		virtual ~AbstractRender() {}
	private:

	};
}