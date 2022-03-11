#pragma once
#include "GameComponent.hpp"
#include "matrix.hpp"
#include "Vector3.hpp"
#include "Vector2.hpp"
#include "IReusable.hpp"
namespace BEbraEngine {
    class RenderBufferView;

}
//TODO: ну, почти.
namespace BEbraEngine {
        enum Camera_Movement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };



    const float YAW = -90.0f;
    const float PITCH = 0.0f;
    const float SPEED = 2.5f;
    const float SENSITIVITY = 1.f;
    const float ZOOM = 45.0f;

   
    class SimpleCamera : public GameComponent, public IReusable
    {
    public:
        struct ShaderData {
            Matrix4 proj;
            Matrix4 view;
            alignas(16) Vector3 position;

        };
    public:
        RenderBufferView* cameraData{};
        
        Vector3 Position{};
        Vector3 Front{};
        Vector3 Up{};
        Vector3 Right{};
        Vector3 WorldUp{};

    public:

        SimpleCamera() {}
        ~SimpleCamera();
        SimpleCamera(const Vector2& size, const Vector3& position = Vector3(0.0f, 0.0f, 0.0f), Vector3 up = Vector3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);

        void destroy(IVisitorGameComponentDestroyer& destroyer) override;

        glm::mat4 getViewMatrix();

        void moveTo(const Vector3& newPos);
        
        void processKeyboard(Camera_Movement direction, float deltaTime);

        void release() override;

        void _move(float& x, float& y);

        void processMouseMovement(bool constrainPitch = true);

        void processMouseScroll(float yoffset);

        void update();

        void resize(Vector2 newSize);

        void lookAt(const Vector3& at);
        
        bool isMain() { return _isMain; }

        void setMain(bool value) { _isMain = value; }

    private:
        void updateCameraVectors();
    private:

        float Yaw{};
        float Pitch{};

        float MovementSpeed{};
        float MouseSensitivity{};
        float Zoom{};

        int lastX{};
        int lastY{};

        int cursorX{};
        int cursorY{};
        bool _isMain{};
        Vector2 rectViewport{};


        

    };
}