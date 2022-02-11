#pragma once
#include "stdafx.h"
#include "platform.hpp"
#include "Model.hpp"

using BE_STD::optional;
using BE_STD::shared_ptr;
using BE_STD::string;
using BE_STD::map;

namespace BEbraEngine {
    class AbstractRender;
}
namespace BEbraEngine {
    class MeshFactory {
    public:

        optional<Model*> create(const string& path);
        optional<Model*> createAsync(const string& path);
        shared_ptr<Model> getDefaultModel(const string& name);
        MeshFactory(AbstractRender* render);
    private:

        void downloadDefaultModels();
        // ����������� ��������� ����. ������������ ������ ��������� ���, ������������� � ����, � ��������� ���� ������� ��� ����� �������� ����� (���� ������ ������ �������)
        void processNode(Model* model, aiNode* node, const aiScene* scene, const string& path);

        Mesh processMesh(Model* model, aiMesh* mesh, const aiScene* scene, const string& path);

        // ��������� ��� �������� ���������� ��������� ���� � �������� ��������, ���� ��� ��� �� ���� ���������.
        // ����������� ���������� ������������ � ���� ��������� Texture
       vector<Texture*> loadMaterialTextures(Model* model, aiMaterial* mat, aiTextureType type, string typeName, const string& path);

        VulkanTextureFactory* _textureFactory;

        AbstractRender* render;

        map<string, shared_ptr<Model>> default_models;

    };
}

