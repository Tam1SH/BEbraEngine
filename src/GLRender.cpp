#include "stdafx.h"
#include "GLRender.hpp"
#include "Vertex.hpp"
#include "RenderBuffer.hpp"
#include "RenderObject.hpp"
#include "Camera.hpp"
#include "GLWindow.hpp"
#include <glad/glad.h>
namespace BEbraEngine {
	const char* vertexShaderSource = "#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 330 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
		"}\n\0";

	int vertexShader;
	int fragmentShader;
	int shaderProgram;
	unsigned int VBO;
	unsigned int VAO;
	void GLRender::create(BaseWindow* window)
	{
		this->window = dynamic_cast<GLWindow*>(window);
		// ��������� ������
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		// �������� �� ������� ������ �������������� ���������� �������
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// ����������� ������
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);

		// �������� �� ������� ������ �������������� ������������ �������
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// ���������� ��������
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		// �������� �� ������� ������ �������������� ���������� ��������
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);


		float vertices[] = {
	-0.5f, -0.5f, 0.0f, // ����� �������
	 0.5f, -0.5f, 0.0f, // ������ �������
	 0.0f,  0.5f, 0.0f  // ������� �������   
		};


		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		// ������� ��������� ������ ���������� �������, ����� ��������� � ������������� ��������� �����(�), � ����� ������������� ��������� �������(�)
		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// �������� ��������, ��� ������ �������� ���������, ����� glVertexAttribPointer() ��������������� VBO ��� ����������� ��������� �������� ������ ��� ���������� ��������, ��� ��� ����� ����� �� ����� �������� ��������� �������
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// �� ������ �������� �������� VAO ����� �����, ����� ������ ������ VAO �������� �� �������� ���� VAO (�� �������� �������� ����� ���������).
		// ����������� ������ VAO ������� ������ glBindVertexArray(), ������� �� ������ �� ������� �������� VAO (��� VBO), ����� ��� �� ��������� ��������
		glBindVertexArray(0);

	}
	RenderBuffer* GLRender::createIndexBuffer(std::vector<uint32_t> indices)
	{
		return nullptr;
	}
	RenderBuffer* GLRender::createVertexBuffer(std::vector<Vertex> vertices)
	{
		return nullptr;
	}
	RenderBuffer* GLRender::createUniformBuffer(size_t size)
	{
		return nullptr;
	}
	RenderBuffer* GLRender::createStorageBuffer(size_t size)
	{
		return nullptr;
	}
	void GLRender::destroyBuffer(RenderBuffer* buffer)
	{
	}

	void GLRender::addObject(std::shared_ptr<RenderObject> object)
	{
	}
	void GLRender::addLight(std::shared_ptr<PointLight> light)
	{
	}
	void GLRender::removeObject(std::shared_ptr<RenderObject> object)
	{
	}
	void GLRender::removeLight(std::shared_ptr<PointLight> light)
	{
	}
	void GLRender::addGlobalLight(std::shared_ptr<DirectionLight> globalLight)
	{
	}
	IRenderObjectFactory* GLRender::getRenderObjectFactory()
	{
		return nullptr;
	}
	void GLRender::drawFrame()
	{
		glClearColor(255 / 255.f, 174 / 255.f, 201 / 255.f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(shaderProgram);
		glBindVertexArray(VAO); // ��������� � ��� ���� ������ ���� VAO, �� ��� ������������� ��������� ��� ������ ��� (�� �� ������� ���, ����� �� ���� ������� ��������������)
		glDrawArrays(GL_TRIANGLES, 0, 3);

		window->swapWindowBuffer();
	}
	AbstractRender::Type GLRender::getType()
	{
		return Type::OpenGL_ES;
	}
	size_t GLRender::alignmentBuffer(size_t originalSize, AbstractRender::TypeBuffer type)
	{
		return size_t();
	}
	void GLRender::selectMainCamera(Camera* camera)
	{
	}
	void GLRender::addCamera(std::shared_ptr<Camera> camera)
	{
	}
	void GLRender::removeCamera(std::shared_ptr<Camera> camera)
	{
	}
}