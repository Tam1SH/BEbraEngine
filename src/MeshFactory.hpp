#pragma once
#include "stdafx.h"
#include "Model.hpp"

namespace BEbraEngine {
    class AbstractRender;
}
namespace BEbraEngine {
    class MeshFactory {
    public:

        std::optional<Model*> create(const std::string& path);
        std::shared_ptr<Model> getDefaultModel(const std::string& name);
        MeshFactory(AbstractRender* render);
    private:

        void downloadDefaultModels();
        // ����������� ��������� ����. ������������ ������ ��������� ���, ������������� � ����, � ��������� ���� ������� ��� ����� �������� ����� (���� ������ ������ �������)
        void processNode(Model* model, aiNode* node, const aiScene* scene, const std::string& path);

        Mesh processMesh(Model* model, aiMesh* mesh, const aiScene* scene, const std::string& path);

        // ��������� ��� �������� ���������� ��������� ���� � �������� ��������, ���� ��� ��� �� ���� ���������.
        // ����������� ���������� ������������ � ���� ��������� Texture
        std::vector<Texture*> loadMaterialTextures(Model* model, aiMaterial* mat, aiTextureType type, std::string typeName, const std::string& path);

        VulkanTextureFactory* _textureFactory;

        AbstractRender* render;

        std::map<std::string, std::shared_ptr<Model>> default_models;

    };
}

