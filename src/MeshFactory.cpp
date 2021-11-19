#include "stdafx.h"
#include "MeshFactory.hpp"
#include "VulkanTextureFactory.hpp"
#include "AbstractRender.hpp"
namespace BEbraEngine {
    std::optional<Model*> MeshFactory::create(const std::string& path)
    {
        auto model = new Model();
        // ������ ����� � ������� Assimp
        Assimp::Importer imposter;
        const aiScene* scene = imposter.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        // �������� �� ������
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // ���� �� 0
        {
            std::cout << "ERROR::ASSIMP:: " << imposter.GetErrorString() << std::endl;
            return std::make_optional<Model*>();
        }

        // ��������� ���� � �����
        //directory = path.substr(0, path.find_last_of('/'));
        // ����������� ��������� ��������� ���� Assimp
        processNode(model, scene->mRootNode, scene, path);
        auto model_out = std::make_optional<Model*>(model);
        return model_out;
    }
    std::shared_ptr<Model> MeshFactory::getDefaultModel(const std::string& name)
    {
        return default_models[name];
    }
    MeshFactory::MeshFactory(AbstractRender* render) { 
        this->render = render;
        downloadDefaultModels(); 
    }
    void MeshFactory::downloadDefaultModels()
    {
        default_models["BOX"] = std::shared_ptr<Model>(create("C:/.BEbraEngine/src/Models/BOX.fbx").value());
        auto vertices_view = new RenderBufferView();
        vertices_view->buffer = std::shared_ptr<RenderBuffer>(render->createVertexBuffer(default_models["BOX"]->meshes[0].vertices));
        default_models["BOX"]->meshes[0].vertices_view = vertices_view;
        auto indices_view = new RenderBufferView();
        indices_view->buffer = std::shared_ptr<RenderBuffer>(render->createIndexBuffer(default_models["BOX"]->meshes[0].indices));
        default_models["BOX"]->meshes[0].indices_view = indices_view;

        default_models["SPHERE"] = std::shared_ptr<Model>(create("C:/.BEbraEngine/src/Models/Sphere.fbx").value());
        vertices_view = new RenderBufferView();
        vertices_view->buffer = std::shared_ptr<RenderBuffer>(render->createVertexBuffer(default_models["SPHERE"]->meshes[0].vertices));
        default_models["SPHERE"]->meshes[0].vertices_view = vertices_view;
        indices_view = new RenderBufferView();
        indices_view->buffer = std::shared_ptr<RenderBuffer>(render->createIndexBuffer(default_models["SPHERE"]->meshes[0].indices));
        default_models["SPHERE"]->meshes[0].indices_view = indices_view;
    }
    void MeshFactory::processNode(Model* model, aiNode* node, const aiScene* scene, const std::string& path)
    {
        // ������������ ������ ��� �������� ����
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // ���� �������� ������ ������� �������� � �����.
            // ����� �� �������� ��� ������; ���� - ��� ���� ������ ����������� ������
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            model->meshes.push_back(processMesh(model, mesh, scene, path));
        }
        // ����� ����, ��� �� ���������� ��� ���� (���� ������� �������), �� �������� ���������� ������������ ������ �� �������� �����
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(model, node->mChildren[i], scene, path);
        }

    }
    Mesh MeshFactory::processMesh(Model* model, aiMesh* mesh, const aiScene* scene, const std::string& path)

    {
        // ������ ��� ����������
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture*> textures;

        // ���� �� ���� �������� ����
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // �� ��������� ������������� ������, �.�. Assimp ���������� ���� ����������� ��������� �����, ������� �� ������������� �������� � ��� glm::vec3, ������� ������� �� �������� ������ � ���� ������������� ������ ���� glm::vec3

            // ����������
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;

            // �������
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;

            // ���������� ����������
            if (mesh->mTextureCoords[0]) // ���� ��� �������� ���������� ����������
            {
                glm::vec2 vec;

                // ������� ����� ��������� �� 8 ��������� ���������� ���������. �� ������������, ��� �� �� ����� ������������ ������,
                // � ������� ������� ����� ��������� ��������� ���������� ���������, ������� �� ������ ����� ������ ����� (0)
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoord = vec;
            }
            else
                vertex.texCoord = glm::vec2(0.0f, 0.0f);

            // ����������� ������
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            // vertex.Tangent = vector;

             // ������ ���������
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            //  vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        // ������ ���������� �� ������ ����� ���� (����� - ��� ����������� ����) � ��������� ��������������� ������� ������
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];

            // �������� ��� ������� ������ � ��������� �� � ������� indices
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        // ������������ ���������
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // �� ������ ���������� �� ������ ��������� � ��������. ������ ��������� �������� ����� ���������� 'texture_diffuseN',
        // ��� N - ���������� ����� �� 1 �� MAX_SAMPLER_NUMBER. 
        // ���� ����� ��������� � � ������ ���������:
        // ��������� - texture_diffuseN
        // ��������� - texture_specularN
        // ������� - texture_normalN

        // 1. ��������� �����
        std::vector<Texture*> diffuseMaps = loadMaterialTextures(model, material, aiTextureType_DIFFUSE, "texture_diffuse", path);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 2. ����� ���������
        std::vector<Texture*> specularMaps = loadMaterialTextures(model, material, aiTextureType_SPECULAR, "texture_specular", path);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 3. ����� ��������
        std::vector<Texture*> normalMaps = loadMaterialTextures(model, material, aiTextureType_HEIGHT, "texture_normal", path);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 4. ����� �����
        std::vector<Texture*> heightMaps = loadMaterialTextures(model, material, aiTextureType_AMBIENT, "texture_height", path);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // ���������� ���-������, ��������� �� ������ ���������� ������
        auto _mesh = Mesh(vertices, indices, textures);
        auto indices_view = new RenderBufferView();
        indices_view->buffer = std::shared_ptr<RenderBuffer>(render->createIndexBuffer(_mesh.indices));
        auto vertices_view = new RenderBufferView();
        vertices_view->buffer = std::shared_ptr<RenderBuffer>(render->createVertexBuffer(_mesh.vertices));
        _mesh.indices_view = indices_view;
        _mesh.vertices_view = vertices_view;
        return _mesh;
    }
    std::vector<Texture*> MeshFactory::loadMaterialTextures(Model* model, aiMaterial* mat, aiTextureType type, std::string typeName, const std::string& path)

    {
        std::vector<Texture*> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);

            // ���������, �� ���� �� �������� ��������� �����, � ���� - ��, �� ���������� �������� ����� �������� � ��������� � ��������� ��������
            bool skip = false;
            for (unsigned int j = 0; j < model->textures_loaded.size(); j++)
            {
                if (std::strcmp(model->textures_loaded[j]->path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(model->textures_loaded[j]);
                    skip = true; // �������� � ��� �� ����� � ����� ��� ���������, ��������� � ��������� (�����������)
                    break;
                }
            }
            if (!skip)
            {   // ���� �������� ��� �� ���� ���������, �� ��������� �
                Texture* texture;
                texture = _textureFactory->create(path);
                textures.push_back(texture);
                model->textures_loaded.push_back(texture); // ��������� �������� � ������� � ��� ������������ ����������, ��� ����� ����������, ��� � ��� �� �������� ��� ������������� ��������� �������
            }
        }
        return textures;
    }
}