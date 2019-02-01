
class CxxStruct:
    def __init__(self, name, fields):
        self.name = name
        self.fields = fields

class ZfwGenerator:
    def generate(self, thing):
        if isinstance(thing, CxxStruct):
            struct = thing

            template = '''
class Struct_{sanitizedTypeName} : public IStruct {
public:
    void New(void* value) final { li::constructPointer(reinterpret_cast<{typeName}*>(value)); }
    void Delete(void* value) final { li::destructPointer(reinterpret_cast<{typeName}*>(value)); }
    size_t Sizeof() final { return sizeof({{ typeName }}); }
    const char* TypeName()  final { return "{{ typeName }}"; }

    IArray* AsArray() final { return nullptr; }
    IStruct* AsStruct() final { return this; }
    
    virtual size_t GetNumFields() final { return {{len(fields}}; }
    virtual ValueAccessor GetFieldAccessor(void* struct_, size_t index) = 0;
};

template <typename T>
IStruct& GetStruct();

template <typename T>
ITypedef& GetTypedef();
'''

            print()
