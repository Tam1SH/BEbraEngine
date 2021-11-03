#pragma once
#include "stdafx.h"
#include "RenderBuffer.hpp"
#include "BaseVulkanRender.hpp"
//TODO: �������� ������� � ������������ ����������
namespace BEbraEngine {
	class Mesh1
	{
	public:
		RenderBuffer* VBO;
		RenderBuffer* EBO;
		Mesh1() {}
		Mesh1(RenderBuffer* VBO, RenderBuffer* EBO) : VBO(VBO), EBO(EBO) {}

		~Mesh1() {
			VBO->Destroy();
			EBO->Destroy();
		}
	};

}