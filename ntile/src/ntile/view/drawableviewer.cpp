#include "view.hpp"

#include "../world.hpp"

#include <framework/components/model3d.hpp>
#include <framework/entityworld2.hpp>

namespace ntile {
    using std::make_unique;

    void DrawableViewer::Draw(RenderingKit::IRenderingManager* rm, zfw::IEntityWorld2* world, intptr_t entityId) {
        if (!model) {
            // TODO: resource management is being given zero thought here

            auto drawable = world->GetEntityComponent<Model3D>(entityId);
            zombie_assert(drawable);

            this->model = make_unique<CharacterModel>(g_eb, g_res.get());
            this->model->Load(drawable->modelPath.c_str());
        }

        this->model->Draw();
    }
}
