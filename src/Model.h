#pragma once
/*
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>
#include <fstream>
#include <vector>

#include "Image.hpp"
#include "Vertex.h"
namespace BEbraEngine {
    class ImageCreator;

    class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture*> textures;
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture*> textures)
            : vertices(vertices), indices(indices), textures(textures) {}
        Mesh() {}
    };
    class Model
    {
    private:
        static ImageCreator* creator;

    public:
        static void SetCreator(ImageCreator* creator) {
            Model::creator = creator;
        }
        static Model* New();
        // ������ ������ 
        std::vector<Texture*> textures_loaded; // (�����������) ��������� ��� ����������� ��������, ����� ���������, ��� ��� �� ��������� ����� ������ ����
        std::vector<Mesh> meshes;
        std::string path;
        bool gammaCorrection;

        // ����������� � �������� ��������� ���������� ���� � 3D-������
        Model(std::string const& path, bool gamma = false);

        // ������������ ������, � ������ � ��� � ����
        void Draw();

    private:
        // ��������� ������ � ������� Assimp � ��������� ���������� ���� � ������� meshes
        void loadModel(std::string const& path);

        // ����������� ��������� ����. ������������ ������ ��������� ���, ������������� � ����, � ��������� ���� ������� ��� ����� �������� ����� (���� ������ ������ �������)
        void processNode(aiNode* node, const aiScene* scene);

        Mesh processMesh(aiMesh* mesh, const aiScene* scene);

        // ��������� ��� �������� ���������� ��������� ���� � �������� ��������, ���� ��� ��� �� ���� ���������.
        // ����������� ���������� ������������ � ���� ��������� Texture
        std::vector<Texture*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    };

}
*/