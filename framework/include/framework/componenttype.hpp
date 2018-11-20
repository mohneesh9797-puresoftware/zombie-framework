#ifndef framework_componenttype_hpp
#define framework_componenttype_hpp

#include <framework/base.hpp>
#include <littl/Allocator.hpp>

namespace zfw
{
    /**
     * This interface provides a full reflection of a certain aspect.
     *
     * It must be provided for every aspect type (look for declaration of GetComponentType).
     * If the data type is not implemented in native code, this will be an instance of some relevant generic class.
     *
     * TODO: perhaps we could just have a generic Compound Type Reflection interface?
     */
    struct IComponentType
    {
        size_t sizeof_;
        //std::type_index type;

        virtual void ConstructFrom(void* aspect, const void* source) = 0;
        virtual void Destruct(void* aspect) = 0;

        //virtual void Serialize(OutputStream* output) = 0;
        // TODO: API for complete introspection
    };

    template <typename T>
    struct BasicComponentType : public IComponentType {
    public:
        BasicComponentType() {
            this->sizeof_ = sizeof(T);
        }

        void ConstructFrom(void* aspect, const void* source) final {
            li::constructPointer(static_cast<T*>(aspect), *static_cast<const T*>(source));
        }

        void Destruct(void* aspect) final {
            li::destructPointer(static_cast<T*>(aspect));
        }
    };

    // Attempt to cast component data. Return pointer if type matches, nullptr otherwise.
    template <typename T>
    T* Cast(IComponentType &type, void *data) {
        if (&type == &GetComponentType<T>()) {
            return reinterpret_cast<T*>(data);
        }
        else {
            return nullptr;
        }
    }
}

#endif
