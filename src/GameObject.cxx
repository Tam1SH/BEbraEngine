module GameObject;
//import GameComponentDestroyer;
import <typeinfo>;
using std::string;

namespace BEbraEngine {
    

    void GameObject::destroy(GameComponentDestroyer& destroyer)
    {
    //    for (auto& components : components_) {
    //        components->destroy(destroyer);
    //    }
    }

    GameObject::GameObject() noexcept
    {
        name_ = "GameObject";
    }
    bool GameObject::isComposite() const noexcept
    {
        return true;

    }
    GameComponent* GameObject::getComponentByName(const string& name) const noexcept
    {
        for (auto& component : components_) {

            //TODO: ��������� ��������� == ������� � ����.
            const std::string componentName = typeid(*component).name();

            if (componentName.find(name) != string::npos) {
                return component.get();
            }
        }
        return nullptr;
    }
}