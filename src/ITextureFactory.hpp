
#include "stdafx.h"
namespace BEbraEngine {
    class Texture;
}
namespace BEbraEngine {

    class ITextureFactory {
    public:
        virtual Texture* createAsync(const std::string& path) = 0;
        virtual Texture* create(const std::string& path) = 0;
        virtual Texture* createEmpty() = 0;
        virtual ~ITextureFactory() {}

    };
}