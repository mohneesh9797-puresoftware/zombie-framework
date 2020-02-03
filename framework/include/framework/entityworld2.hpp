#ifndef framework_entityworld2_hpp
#define framework_entityworld2_hpp

#include <framework/base.hpp>

#include <functional>

namespace zfw
{
    struct EntityHandle {
        IEntityWorld2* world;
        EntityId entityId;

        inline void Destroy() const;

        template <typename ComponentStruct>
        ComponentStruct const* GetComponent() const;

        template <typename ComponentStruct>
        void SetComponent(const ComponentStruct &data);
    };

    class IEntityWorld2
    {
        public:
            static unique_ptr<IEntityWorld2> Create(IBroadcastHandler& broadcastHandler);
            virtual ~IEntityWorld2() = default;

            virtual EntityId CreateEntityId() = 0;
//            intptr_t CreateEntityFromBlueprint(blueprint);

            virtual void DestroyEntity(EntityId id) = 0;

            /**
             * Get a cache-able pointer to a specific entity component.
             * @param id a valid entity ID or kInvalidEntity
             * @param type
             * @return pointer to component data, or nullptr if the component has not been set for this entity.
             *         Guaranteed to remain valid as long as the entity exists.
             */
            virtual void* GetEntityComponent(EntityId id, IComponentType &type) = 0;

            /**
             *
             * @param id
             * @param type
             * @param data component data; this will be copied into IEntityWorld2's private storage
             * @return pointer to IEntityWorld2-owned copy of component data. Guaranteed to remain valid
             *         as long as the entity exists.
             */
            virtual void* SetEntityComponent(EntityId id, IComponentType &type, const void *data) = 0;

            virtual void IterateEntitiesByComponent(IComponentType &type, std::function<void(EntityId entityId, void* component_data)> callback) = 0;

            EntityHandle CreateEntity() {
                return {this, this->CreateEntityId()};
            }

            template <typename ComponentStruct>
            ComponentStruct* GetEntityComponent(EntityId id) {
                return static_cast<ComponentStruct*>(this->GetEntityComponent(id, GetComponentType<ComponentStruct>()));
            }

            template <class ComponentStruct>
            void IterateEntitiesByComponent(std::function<void(EntityId entityId, ComponentStruct& component_data)> callback) {
                this->IterateEntitiesByComponent(GetComponentType<ComponentStruct>(), [callback](EntityId entityId, void* component_data) {
                    callback(entityId, *static_cast<ComponentStruct*>(component_data));
                });
            }

            template <typename ComponentStruct>
            void SetEntityComponent(EntityId id, const ComponentStruct &data) {
                this->SetEntityComponent(id, GetComponentType<ComponentStruct>(), &data);
            }
    };

    void EntityHandle::Destroy() const {
        world->DestroyEntity(entityId);
    }

    template <typename ComponentStruct>
    ComponentStruct const* EntityHandle::GetComponent() const {
        return static_cast<ComponentStruct*>(world->GetEntityComponent(entityId, GetComponentType<ComponentStruct>()));
    }

    template <typename ComponentStruct>
    void EntityHandle::SetComponent(const ComponentStruct &data) {
        world->SetEntityComponent(entityId, GetComponentType<ComponentStruct>(), &data);
    }
}

#endif
