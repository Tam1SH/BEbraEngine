﻿module;

export module Model;
import Texture;
import Vertex;
import RenderBufferView;
import <vector>;
import <string>;

namespace BEbraEngine {

    export class Mesh {
    public:
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
        std::vector<Texture*> textures;
        RenderBufferView* vertices_view;
        RenderBufferView* indices_view;
        Mesh(std::vector<Vertex> vertices, std::vector<uint32_t> indices, std::vector<Texture*> textures)
            : vertices(vertices), indices(indices), textures(textures) {}
        Mesh() {}
        ~Mesh() {

        }
    };

    export struct ModelCreateInfo 
    {
        std::string path;
    };

    export struct Model
    {
    public:
        std::vector<Texture*> textures_loaded;

        std::vector<Mesh> meshes;

        Model() {}
        ~Model() {

            for (auto& mesh : meshes) {
               // mesh.vertices_view->buffer->destroy();
               // mesh.indices_view->buffer->destroy();
                delete mesh.vertices_view;
                delete mesh.indices_view;
            }
        }
    };

}
