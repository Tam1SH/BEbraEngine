#pragma once
#include <string>

//TODO: ���, � ���������� ���� ����� ����� ����������?
// �� ����� �� ��� ���� ������� � ������
//TODO: ���� ��� ������ ������ ����� � ��� ����� ����� ��������, ������ ��� � ���
template<
    typename Function,
    typename Getter,
    typename Setter
    >
class IScriptObject {
public:
    virtual IScriptObject* SetProperty(std::string name, Getter get, Setter set) = 0;
    virtual IScriptObject* SetFunction(std::string name, Function func) = 0;
   // virtual IScriptObject* SetHandler(void* handler) = 0;
};