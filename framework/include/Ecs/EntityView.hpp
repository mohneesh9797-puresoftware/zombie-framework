#ifndef OBS_ECS_ENTITYVIEW_HPP
#define OBS_ECS_ENTITYVIEW_HPP

#include <framework/base.hpp>
#include <framework/broadcasthandler.hpp>
#include <framework/componenttype.hpp>
#include <framework/entityworld2.hpp>
#include <framework/utility/essentials.hpp>

#include <functional>
#include <unordered_map>

namespace Obs::Ecs {

// adapted from https://stackoverflow.com/a/54225452/2524350
template<size_t I = 0, typename... Tp>
bool any_null(std::tuple<Tp...>& t) {
    if (std::get<I>(t) == nullptr) {
        return true;
    }

    if constexpr(I+1 != sizeof...(Tp)) {
        return any_null<I + 1>(t);
    }
    else {
        return false;
    }
}

// TODO API: public iterator
template <typename CustomData, typename... Components>
class EntityViewWithCustomData {
public:
    typedef std::tuple<std::add_lvalue_reference_t<Components>...> References;

    struct Tracked {
        CustomData customData;
        References const components;
    };

    typedef std::function<void(zfw::IEntityWorld2& world, zfw::EntityId entityId, References components, CustomData& customData)> UpdateCustomDataInPlaceFunction;

    std::unordered_map<intptr_t, Tracked> entities;
    UpdateCustomDataInPlaceFunction const updateCustomDataInPlace;

    EntityViewWithCustomData(UpdateCustomDataInPlaceFunction updateCustomDataInPlace) : updateCustomDataInPlace(updateCustomDataInPlace) {}

    void OnComponentEvent(zfw::IEntityWorld2& world, zfw::EntityId entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event);

    void SubscribeToComponentTypes(zfw::IBroadcastHandler& bh, zfw::IBroadcastSubscriber& sub) {
        (bh.SubscribeToComponentType(sub, zfw::GetComponentType<Components>()), ...);
    }
};

template <typename CustomData, typename... Components>
void EntityViewWithCustomData<CustomData, Components...>::OnComponentEvent(zfw::IEntityWorld2& world, zfw::EntityId entityId, zfw::IComponentType &type, void *data, zfw::ComponentEvent event) {
    zombie_assert(event == zfw::ComponentEvent::created);

    auto iter = entities.find(entityId);

    if (iter == entities.end()) {
        std::tuple<std::add_pointer_t<Components>...> tupleOfPointers { world.GetEntityComponent<Components>(entityId)... };

        // entity does not have all the components that we're interested in?
        if (any_null(tupleOfPointers)) {
            return;
        }

        auto [iter, inserted] = entities.emplace(entityId, Tracked { {}, References(*world.GetEntityComponent<Components>(entityId)...) });
        updateCustomDataInPlace(world, entityId, iter->second.components, iter->second.customData);
    }
    else {
        // TODO: assert / codify why we never need to update component pointers

        updateCustomDataInPlace(world, entityId, iter->second.components, iter->second.customData);
    }
}

}

#endif
