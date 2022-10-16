module;
#include <vulkan.h>
#include <boost/filesystem.hpp>
export module Texture;
import GameComponent;
import Debug;
import GameComponentDestroyerDecl;

namespace BEbraEngine {
    
    export class Texture : public GameComponent {
    public:

        void destroy(GameComponentDestroyer& destroyer) override;

        bool isLoaded() { return isLoaded_; }
        void setLoaded() { isLoaded_ = true; }

        uint32_t width() { return width_; }
        uint32_t height() { return height_; }

        Texture() { }
        ~Texture() { }

    protected:
        bool isLoaded_{};
        uint32_t width_{};
        uint32_t height_{};
    };

    export class VulkanTexture : public Texture {
    public:
        void setWidth(uint32_t value) { width_ = value; }
        void setHeight(uint32_t value) { height_ = value; }
        VulkanTexture& operator=(VulkanTexture&& other) noexcept {
            if (this == &other) return *this;
            image = other.image;
            memory = other.memory;
            imageView = other.imageView;
            sampler = other.sampler;
            other.image = 0;
            other.memory = 0;
            other.imageView = 0;
            other.sampler = 0;

            return *this;
        }
        VulkanTexture& operator=(const VulkanTexture& other) {
            this->image = other.image;
            this->imageView = other.imageView;
            this->memory = other.memory;
            this->sampler = other.sampler;
            this->mipLevels = other.mipLevels;

            return *this;
        }
        VulkanTexture(const VulkanTexture& other) {
            this->image = other.image;
            this->imageView = other.imageView;
            this->memory = other.memory;
            this->sampler = other.sampler;
            this->mipLevels = other.mipLevels;

        }
        VulkanTexture() {}
        VulkanTexture(VulkanTexture&& other) noexcept {
            if (this == &other) return;
            image = other.image;
            memory = other.memory;
            imageView = other.imageView;
            sampler = other.sampler;
            other.image = 0;
            other.memory = 0;
            other.imageView = 0;
            other.sampler = 0;
        }
        uint32_t mipLevels;
        VkImage image;
        VkImageView imageView;
        VkSampler sampler;
        VkDeviceMemory memory;
    };
}