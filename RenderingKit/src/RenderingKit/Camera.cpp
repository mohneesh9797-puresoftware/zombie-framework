
#include <RenderingKit/utility/Camera.hpp>

#include <framework/utility/essentials.hpp>

namespace RenderingKit
{
    static void s_EyeAndUpFromAngles( const Float3& center, float eyeDistance, float yaw, float pitch, Float3& eye_out, Float3& up )
    {
        using zfw::f_pi;

        if (pitch > f_pi * 0.5f - 10e-4f)
            pitch = f_pi * 0.5f;
        else if (pitch < -f_pi * 0.5f + 10e-4f)
            pitch = -f_pi * 0.5f;

        const float ca = std::cos( yaw );
        const float sa = std::sin( yaw );

        const float ca2 = std::cos( pitch );
        const float sa2 = std::sin( pitch );

        const float radius = ca2 * eyeDistance;
        const float upRadius = radius - sa2;

        const Float3 cam( ca * radius, -sa * radius, sa2 * eyeDistance );

        eye_out = center + cam;

        if (std::fabs(radius) > 10e-5f)
            up = Float3( ca * upRadius - cam.x, -sa * upRadius - cam.y, ca2 );
    }

    static void convert( const Float3& eye, const Float3& center, const Float3& up, float& dist, float& angle, float& angle2 )
    {
        const Float3 cam = eye - center;

        dist = glm::length( cam );
        angle = std::atan2( -cam.y, cam.x );
        angle2 = std::atan2( cam.z, glm::length( glm::vec2( cam ) ) );
    }

    Camera::Camera(const char* name, CoordinateSystem coordSystem)
            : name(name), coordSystem(coordSystem)
    {
        proj = kProjectionInvalid;
        nearZ = 1.0f;
        farZ = 1000.0f;
        vfov = 45.0f * zfw::f_pi / 180.0f;
    }

    void Camera::BuildModelView()
    {
        switch (coordSystem)
        {
            case CoordinateSystem::leftHanded:
                // TODO: isn't this needlessly complicated? we should be able to just flip Z in clip space or something
                modelView = glm::lookAt( Float3( eye.x, -eye.y, eye.z ), Float3( center.x, -center.y, center.z ), Float3( up.x, -up.y, up.z ) );
                modelView = glm::scale( modelView, Float3( 1.0f, -1.0f, 1.0f ) );
                break;

            case CoordinateSystem::rightHanded:
                modelView = glm::lookAt( eye, center, up );
                break;
        }
    }

    float Camera::CameraGetDistance()
    {
        return glm::length( eye - center );
    }

    void Camera::CameraMove( const glm::vec3& vec )
    {
        eye += vec;
        center += vec;
        BuildModelView();
    }

    void Camera::CameraRotateXY( float alpha, bool absolute )
    {
        float dist, angle, angle2;

        convert( eye, center, up, dist, angle, angle2 );

        if ( absolute )
            angle2 = alpha;
        else
            angle2 += alpha;

        s_EyeAndUpFromAngles( center, dist, angle, angle2, eye, up );
        BuildModelView();
    }

    void Camera::CameraRotateZ(float alpha, bool absolute)
    {
        float dist, angle, angle2;

        convert( eye, center, up, dist, angle, angle2 );

        if ( absolute )
            angle = alpha;
        else
            angle += alpha;

        s_EyeAndUpFromAngles( center, dist, angle, angle2, eye, up );
        BuildModelView();
    }

    void Camera::CameraZoom(float amount, bool absolute)
    {
        float dist, angle, angle2;

        convert( eye, center, up, dist, angle, angle2 );

        if ( absolute )
            dist = amount;
        else if ( dist + amount > 0.0f )
            dist += amount;

        s_EyeAndUpFromAngles( center, dist, angle, angle2, eye, up );
        BuildModelView();
    }

    void Camera::BuildProjectionModelViewMatrices(const Int2& viewportSize, glm::mat4x4* projection_out, glm::mat4x4* modelView_out)
    {
        switch (proj)
        {
            case kProjectionInvalid:
                ZFW_DBGASSERT(this->proj != kProjectionInvalid)
                break;

            case kProjectionOrtho:
                projection = glm::ortho(left, right, bottom, top, nearZ, farZ);
                modelView = glm::mat4x4();
                break;

            case kProjectionOrthoFakeFov:
            {
                const float aspectRatio = (float) viewportSize.x / viewportSize.y;
                const float vfovHeight = CameraGetDistance() * std::tan(vfov);

                const Float2 vfov( -vfovHeight / 2, vfovHeight / 2 );
                const Float2 hfov( vfov * aspectRatio );

                projection = glm::ortho(hfov.x, hfov.y, vfov.x, vfov.y, nearZ, farZ);
                break;
            }

            case kProjectionOrthoScreenSpace:
                projection = glm::ortho(0.0f, (float) viewportSize.x, (float) viewportSize.y, 0.0f, nearZ, farZ);
                modelView = glm::mat4x4();
                break;

            case kProjectionPerspective:
            {
                const float aspectRatio = (float) viewportSize.x / viewportSize.y;

                const float vfov2 = ( float )( nearZ * std::tan(this->vfov / 2.0f) );
                const float hfov2 = vfov2 * aspectRatio;

                projection = glm::frustum(-hfov2, hfov2, -vfov2, vfov2, nearZ, farZ);
                break;
            }
        }

        *projection_out = projection;
        *modelView_out = modelView;
    }

    void Camera::SetClippingDist(float nearClip, float farClip)
    {
        this->nearZ = nearClip;
        this->farZ = farClip;
    }

    void Camera::SetOrtho(float left, float right, float top, float bottom)
    {
        proj = kProjectionOrtho;
        this->left = left;
        this->right = right;
        this->top = top;
        this->bottom = bottom;
    }

    void Camera::SetView(const Float3& eye, const Float3& center, const Float3& up)
    {
        this->eye = eye;
        this->center = center;
        this->up = up;
        BuildModelView();
    }

    void Camera::SetView2(const Float3& center, const float eyeDistance, float yaw, float pitch)
    {
        this->center = center;
        this->up = Float3(0.0f, -1.0f, 0.0f);
        s_EyeAndUpFromAngles(center, eyeDistance, yaw, pitch, this->eye, this->up);
        BuildModelView();
    }
}
