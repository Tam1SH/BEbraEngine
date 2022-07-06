
export module TransformFactory;
import Transform;
import <optional>;

namespace BEbraEngine {
    export class TransformFactory {
    public:
        std::optional<Transform*> create(const Transform::CreateInfo& info)
        {
            auto trans = new Transform(info.position);
            trans->setScale(info.scale);
            trans->setName("Transform");
            return std::optional<Transform*>(trans);
        }

        TransformFactory()
        {
        }
    private:
        friend class Transform;
    };
}