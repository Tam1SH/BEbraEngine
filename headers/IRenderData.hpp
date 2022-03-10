#pragma once
namespace BEbraEngine {

	class IRenderData {
	public:
		virtual ~IRenderData() {}
		/// <summary>
		/// ��������� ��������� �� ������, ������� ����� �������������� � �������. 
		/// ���������� ������ ��� ��������� ����� �� �������������.
		/// </summary>
		virtual const void* getData() noexcept { return 0; }
		virtual size_t getDataSize() const noexcept { return 0; }
	};
}