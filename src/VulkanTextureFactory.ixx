﻿module;

export module VulkanTextureFactory;
import VulkanRender;
import Material;
import CRender;
import Render;
import Task;
import BitMap;
import Texture;
import Concepts;
import <variant>;
import <optional>;
import <functional>;
import <memory>;
import <string>;

using std::shared_ptr;
using std::function;
using std::optional;

namespace BEbraEngine {
    export struct VulkanTextureFactory {

        Task<optional<Material*>> createMaterialAsync(const MaterialCreateInfo& info);

        Task<optional<Texture*>> createAsync(const std::string& path);

        Texture* create(const std::string& path, bool generateMip);

        Texture* createEmpty();

        void saveImage(const char* fileName, int width, int height, int channel_num, const void* rows, int quality);
        
        void saveImage(const char* fileName, int width, int height, int channel_num, BEbraEngine::BitMap& bitMap, int quality);

       // void setDestroyer(VisitorGameComponentDestroyer& destroyer); //override;

        void destroyTexture(Texture& texture);

        void destroyTextureAsync(shared_ptr<Texture> texture);

        VulkanTextureFactory(VulkanRender& render);
        VulkanTextureFactory() {}

        VulkanTextureFactory(const VulkanTextureFactory&) = delete;
        VulkanTextureFactory& operator=(const VulkanTextureFactory&) = delete;

        VulkanTextureFactory(VulkanTextureFactory&&) noexcept = default;
        VulkanTextureFactory& operator=(VulkanTextureFactory&&) noexcept = default;

    private:
        VulkanRender* render;
        //VisitorGameComponentDestroyer* destroyer;

    };
    static_assert(OnlyMovable<VulkanTextureFactory>);

}

module :private;
import CTextureFactory;

namespace BEbraEngine {
    static_assert(CTextureFactory<VulkanTextureFactory>);
}