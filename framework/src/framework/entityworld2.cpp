#include <framework/componenttype.hpp>
#include <framework/entity2.hpp>
#include <framework/entityworld2.hpp>
#include <framework/utility/util.hpp>

#include <unordered_map>
#include <framework/broadcasthandler.hpp>

namespace zfw
{
    using std::make_unique;

    // ====================================================================== //
    //  class declaration(s)
    // ====================================================================== //

    // TODO: re-implement in a cache friendly way
    class PerInstanceDataPool {
    public:
        PerInstanceDataPool(IComponentType& type) : type(type) {}
        ~PerInstanceDataPool();

        void* Get(EntityId entityId);
        void* GetOrAlloc(EntityId entityId, bool* wasCreated_out);
        void Iterate(std::function<void(EntityId entityId, void* component_data)> callback);

    private:
        IComponentType& type;
        std::unordered_map<EntityId, void*> perInstanceData;
    };

    class EntityWorld2 : public IEntityWorld2 {
    public:
        EntityWorld2(IBroadcastHandler& broadcast) : broadcast(broadcast) {}

        EntityId CreateEntityId() final;

        void DestroyEntity(EntityId id) final;

        void* GetEntityComponent(EntityId id, IComponentType &type) final;
        void* SetEntityComponent(EntityId id, IComponentType &type, const void *data) final;

        void IterateEntitiesByComponent(IComponentType &type, std::function<void(EntityId entityId, void* component_data)> callback) final;

    private:
        IBroadcastHandler& broadcast;

        EntityId nextId = 0;

        // TODO: re-do as vector by component id
        std::unordered_map<IComponentType*, unique_ptr<PerInstanceDataPool>> componentPools;
    };

    // ====================================================================== //
    //  class EntityWorld2
    // ====================================================================== //

    unique_ptr<IEntityWorld2> IEntityWorld2::Create(IBroadcastHandler& broadcast) {
        return std::make_unique<EntityWorld2>(broadcast);
    }

    EntityId EntityWorld2::CreateEntityId() {
        return this->nextId++;
    }

    void EntityWorld2::DestroyEntity(EntityId id) {
        // FIXME!
    }

    void* EntityWorld2::GetEntityComponent(EntityId id, IComponentType &type) {
        if (id == kInvalidEntity) {
            return nullptr;
        }

        auto iter = componentPools.find(&type);

        if (iter == componentPools.end()) {
            return nullptr;
        }
        else {
            return iter->second->Get(id);
        }
    }

    void EntityWorld2::IterateEntitiesByComponent(IComponentType &type, std::function<void(EntityId entityId, void* component_data)> callback) {
        auto iter = componentPools.find(&type);

        if (iter != componentPools.end()) {
            iter->second->Iterate(callback);
        }
    }

    void* EntityWorld2::SetEntityComponent(EntityId id, IComponentType &type, const void *data) {
        auto iter = componentPools.find(&type);

        PerInstanceDataPool* pool;

        if (iter == componentPools.end()) {
            auto pool_ = make_unique<PerInstanceDataPool>(type);
            pool = pool_.get();
            componentPools[&type] = move(pool_);
        }
        else {
            pool = iter->second.get();
        }

        bool wasCreated;
        void* component_stored = pool->GetOrAlloc(id, &wasCreated);

        if (!wasCreated) {
            type.Destruct(component_stored);
        }

        type.ConstructFrom(component_stored, data);

        if (wasCreated) {
            broadcast.BroadcastComponentEvent(*this, id, type, component_stored, ComponentEvent::created);
        }
        else {
            //broadcast.BroadcastComponentEvent(*this, id, type, component_stored, ComponentEvent::updated);
        }

        return component_stored;
    }

    // ====================================================================== //
    //  class PerInstanceDataPool
    // ====================================================================== //

    PerInstanceDataPool::~PerInstanceDataPool() {
        for (auto& pair : perInstanceData) {
            type.Destruct(pair.second);
            free(pair.second);
        }
    }

    void* PerInstanceDataPool::Get(EntityId entityId) {
        auto iter = perInstanceData.find(entityId);

        if (iter == perInstanceData.end()) {
            return nullptr;
        }
        else {
            return iter->second;
        }
    }

    void* PerInstanceDataPool::GetOrAlloc(EntityId entityId, bool* wasCreated_out) {
        auto iter = perInstanceData.find(entityId);

        if (iter == perInstanceData.end()) {
            auto size = type.sizeof_;
            auto data = malloc(size);
            perInstanceData[entityId] = data;
            *wasCreated_out = true;
            return data;
        }
        else {
            *wasCreated_out = false;
            return iter->second;
        }
    }

    void PerInstanceDataPool::Iterate(std::function<void(EntityId entityId, void* component_data)> callback) {
        for (const auto& pair : this->perInstanceData) {
            callback(pair.first, pair.second);
        }
    }
}
