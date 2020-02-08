#include "Ms3dModelLayer.hpp"
#include "rendering/Ms3dModel.hpp"

#include <Gfx/RenderingContext.hpp>

#include <framework/components/model3d.hpp>
#include <framework/components/position.hpp>
#include <framework/componenttype.hpp>
#include <framework/resourcemanager2.hpp>

#include <glm/gtc/quaternion.hpp>

using glm::mat4x4;
using std::get;

namespace Client {

Ms3dModelLayer::Ms3dModelLayer(zfw::IBroadcastHandler& bh, zfw::IEngine& engine, zfw::IEntityWorld2& world,
    zfw::IResourceManager2& resMgr, RenderingKit::IRenderingManager& rm)
    : renderables([this](auto&&... args) { UpdateInPlace(args...); })
    , ms3dModelResourceProvider(engine, rm)
    , world(world)
    , resMgr(resMgr) {

    ms3dModelResourceProvider.RegisterWith(resMgr);

    renderables.SubscribeToComponentTypes(bh, *this);
}

Ms3dModelLayer::~Ms3dModelLayer() {
    // TODO: use RAII (ResourceProviderRegistration)
    resMgr.UnregisterResourceProvider(&ms3dModelResourceProvider);
}

void Ms3dModelLayer::OnComponentEvent(
    zfw::IEntityWorld2& world, intptr_t entityId, zfw::IComponentType& type, void* data, zfw::ComponentEvent event) {
    if (&world != &this->world) {
        return;
    }

    renderables.OnComponentEvent(world, entityId, type, data, event);
}

void Ms3dModelLayer::Render(RenderingContext const& ctx) {
    for (auto const& [entityId, renderable] : renderables.entities) {
        auto const& [model, components] = renderable;

        auto& position = get<zfw::Position&>(components);

        auto transform = glm::translate(mat4x4(1.0f), position.pos) * glm::mat4_cast(position.rotation)
            * glm::scale({}, position.scale);

        //             auto translate  = glm::translate({}, a_player.loc);
        //             auto rotate = glm::rotate(translate, a_player.angle, glm::vec3{ 0.0f, 0.0f, 1.0f });
        model->Draw(transform);

        // {
        //     Transform transforms[] = {
        //             {Transform::translate, loc},
        //             {Transform::rotate,    {0.0f, 0.0f, 1.0f}, angle}, // rotate with player
        //             {Transform::translate, {0.2f, 0.15f, 0.6f}}, // position to player
        //             {Transform::rotate,    {0.0f, 1.0f, 0.0f}, M_PI / 5.0f}, // roll
        //             {Transform::rotate,    {1.0f, 0.0f, 0.0f}, M_PI / 3.0f}, // yaw
        //     };

        //     sword->render(transforms);
        // }
    }
}

void Ms3dModelLayer::UpdateInPlace(zfw::IEntityWorld2& world, zfw::EntityId entityId,
    decltype(renderables)::References components, Ms3dModel*& model) {
    // FIXME: must handle errors somehow

    auto& model3d = get<zfw::Model3D&>(components);
    model = resMgr.GetResourceByPath<Ms3dModel>(model3d.modelPath.c_str(), 0);
}

}