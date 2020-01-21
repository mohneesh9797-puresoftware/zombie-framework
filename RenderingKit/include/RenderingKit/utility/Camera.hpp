#include "../RenderingKit.hpp"

#include <string>
#include <utility>

namespace RenderingKit
{
    enum RKProjectionType_t
    {
        kProjectionInvalid,
        kProjectionOrtho,
        kProjectionOrthoFakeFov,
        kProjectionOrthoScreenSpace,
        kProjectionPerspective
    };

    class Camera final : public ICamera
    {
    public:
        Camera(const char* name, CoordinateSystem coordSystem);
        Camera(CoordinateSystem coordSystem) : Camera("", coordSystem) {}

        virtual const char* GetName() override { return name.c_str(); }

        virtual float CameraGetDistance() override;
        virtual void CameraMove(const Float3& vec) override;
        virtual void CameraRotateXY(float alpha, bool absolute) override;
        virtual void CameraRotateZ(float alpha, bool absolute) override;
        virtual void CameraZoom(float amount, bool absolute) override;

        virtual Float3 GetCenter() override { return center; }
        virtual Float3 GetEye() override { return eye; }
        virtual Float3 GetUp() override { return up; }

        virtual void GetModelViewMatrix(glm::mat4* output) override
        {
            *output = modelView;
        }

        virtual void GetProjectionModelViewMatrix(glm::mat4* output) override
        {
            *output = (projection * modelView);
        }

        virtual void SetClippingDist(float nearClip, float farClip) override;
        virtual void SetOrtho(float left, float right, float top, float bottom) override;
        virtual void SetOrthoFakeFOV() override { proj = kProjectionOrthoFakeFov; }
        virtual void SetOrthoScreenSpace() override { proj = kProjectionOrthoScreenSpace; }
        virtual void SetPerspective() override { proj = kProjectionPerspective; }
        virtual void SetVFov(float vfov_radians) override { this->vfov = vfov_radians; }
        virtual void SetView(const Float3& eye, const Float3& center, const Float3& up) override;
        virtual void SetView2(const Float3& center, const float eyeDistance, float yaw, float pitch) override;

        void BuildProjectionModelViewMatrices(const Int2& viewportSize, glm::mat4x4* projection_out, glm::mat4x4* modelView_out);

    private:
        void BuildModelView();

        std::string name;
        CoordinateSystem coordSystem;

        RKProjectionType_t proj;
        Float3 eye, center, up;
        float vfov;
        float left, right, top, bottom;
        float nearZ, farZ;

        glm::mat4 projection, modelView;
    };
}
