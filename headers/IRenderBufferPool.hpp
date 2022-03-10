#include "stdafx.h"
#include "platform.hpp"
#include "AbstractRender.hpp"
#include "RenderBuffer.hpp"
#include "IRenderData.hpp"

using BE_STD::shared_ptr;
using BE_STD::optional;

namespace BEbraEngine {

	enum class RenderBufferPoolUsage {
		//������ ������������� ������ ������.
		SeparateOneBuffer

	};

	template<typename RenderData>
	class IRenderBufferPool {

	public:

		virtual ~IRenderBufferPool() { }
		/// <summary>
		/// �������� ������.
		/// </summary>
		/// <param name="count">: ���������� �������������, ������� ����� ������������ ���.</param>
		/// <param name="sizeofData">: ������ ����� ������, ��������������� ������� �������������.</param>
		/// <param name="type">: ��� ������.</param>
		virtual void allocate(uint32_t count, uint32_t sizeofData, AbstractRender::TypeBuffer type) = 0;

		virtual void deallocate(uint32_t count) = 0;

		/// <summary>
		/// �������� ������ � �����, �������� ����� bindData(), � ������ ���������� ������ ��������������� ��� ���������.
		/// </summary>
		virtual void map() = 0;

		virtual void free(shared_ptr<RenderBufferView> view) = 0;

		virtual void setContext(AbstractRender* render) = 0;

		virtual shared_ptr<RenderBuffer> getBuffer() = 0;

		virtual void setUsage(RenderBufferPoolUsage usage) = 0;

		virtual size_t getCount() = 0;

		/// <summary>
		/// ��������� ����� ������, ������� �������� �� ������ ������, � ����� ������������ �����, �������� ��� ������ allocate().
		/// </summary>
		virtual optional<shared_ptr<RenderBufferView>> get() = 0;
	};

}