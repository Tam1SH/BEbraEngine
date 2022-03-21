﻿
#include "stdafx.h"

#include "VulkanTextureFactory.hpp"
#include <stb-master/stb_image.h>
#include <stb-master/stb_image_write.h>
#include "VulkanRender.hpp"
#include "Debug.hpp"

namespace BEbraEngine {
    optional<Material*> VulkanTextureFactory::createMaterialAsync(const Material::CreateInfo& info, function<void(Material*)> onComplete)
    {
        auto strColorLow = ((info.color.parent_path().string() / info.color.stem()).string() + "_low");
        auto strExensionColor = info.color.extension().string();
        auto image = std::shared_ptr<Texture>(create(strColorLow + strExensionColor, false));

        
        tbb::task_arena g;
        auto mat = new Material(image);

        g.enqueue([=] {
            auto strColor = ((info.color.parent_path().string() / info.color.stem()).string());
            auto strSpecular = ((info.specular.parent_path().string() / info.specular.stem()).string());
            auto strNormal = ((info.normal.parent_path().string() / info.normal.stem()).string());

            auto strExensionColor = info.color.extension().string();
            auto strExensionSpecular = info.specular.extension().string();
            auto strExensionNormal = info.normal.extension().string();

            auto color = create(strColor + strExensionColor, true);
            //Хуй знает, есть ли смысл создавать мип лвла для всего этого
            auto specular = create(strSpecular + strExensionSpecular, false);
            auto normal = create(strNormal + strExensionNormal, false);
            auto image_temp = std::shared_ptr<Texture>();
            
            mat->color.swap(image_temp);
            destroyTextureAsync(image_temp);
            *mat = Material(color, specular, normal);

            onComplete(mat);

            });
        return mat;
    }
    Texture* VulkanTextureFactory::createAsync(const boost::filesystem::path& path, std::function<void(Texture*)> onComplete)
    {

        auto str = ((path.parent_path().string() / path.stem()).string() + "_low");
        auto str1 = path.extension().string();
        auto path_low = str + str1;
        auto image = create(path_low, false);

        tbb::task_arena g;
        g.enqueue([=] {
            auto str = ((path.parent_path().string() / path.stem()).string());
            auto str1 = path.extension().string();

            std::string path = str + str1;
            Texture* image = create(path, true);
            
            onComplete(image);
        });

        return image;
    }
    Texture* VulkanTextureFactory::create(const boost::filesystem::path& path, bool generateMip)
    {
        VulkanTexture* image = new VulkanTexture();
        int texWidth, texHeight, texChannels;
       // stbi_set_flip_vertically_on_load(true);
        
        stbi_uc* pixels = stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        image->setHeight(texHeight);
        image->setWidth(texWidth);
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels)
        {
            if(!path.string().empty())
                DEBUG_LOG2("failed to upload texture. uncorrect path or file don't exist. | path: " + path.string(),
                    image, "", Debug::ObjectType::Empty, Debug::MessageType::Error);
            pixels = new unsigned char[4];
            pixels[0] = 255;
            pixels[1] = 255;
            pixels[2] = 255;
            texWidth = 1, texHeight = 1;
            imageSize = 4 * texWidth * texHeight;
            image->setHeight(texHeight);
            image->setWidth(texWidth);
        }
        if (generateMip)
            image->mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;
        else {

            image->mipLevels = 1;
            image->setLoaded();
        }

        render->createVkImage(pixels, image, imageSize);
        image->imageView = render->createImageView(image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        stbi_image_free(pixels);
        
        render->createTextureSampler(image);
        return image;
    }
    Texture* VulkanTextureFactory::createEmpty()
    {
        return create("", false);
    }
    void VulkanTextureFactory::saveImage(const char* fileName, int width, int height, int channel_num, const BitMap& pixels, int quality)
    {
        tbb::task_arena t;
        t.enqueue([=] {
            vector<uint8_t> pixelsConvert;
            pixelsConvert.resize(width * height * channel_num);
            int index{};
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    pixelsConvert[index++] = pixels.pixels[y][x].x;
                    pixelsConvert[index++] = pixels.pixels[y][x].y;
                    pixelsConvert[index++] = pixels.pixels[y][x].z;
                }
            }
            stbi_write_jpg(fileName, width, height, channel_num, pixelsConvert.data(), quality);
            });
       
    }
    void VulkanTextureFactory::setDestroyer(IVisitorGameComponentDestroyer& destroyer)
    {
        this->destroyer = &destroyer;
    }

    VulkanTextureFactory::VulkanTextureFactory(AbstractRender* render) : render(dynamic_cast<VulkanRender*>(render))
    { 
        if (!render) throw std::runtime_error("render isn't VulkanRender"); 
    }
    void VulkanTextureFactory::destroyTexture(Texture& texture)
    {
        auto& vTexture = texture.as<VulkanTexture>();
       // render->destroyTexture(vTexture);
    }
    void VulkanTextureFactory::destroyTextureAsync(shared_ptr<Texture> texture)
    {
        auto& vTexture = std::static_pointer_cast<VulkanTexture>(texture);
        render->destroyTextureAsync(vTexture);
    }
}
