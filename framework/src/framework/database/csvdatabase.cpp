#include <framework/database/keyvaluedatabase.hpp>

#include <rapidcsv.h>
#include <framework/datamodel.hpp>
#include <framework/utility/util.hpp>

namespace zfw
{
    using std::make_unique;
    using std::string;

    // ====================================================================== //
    //  class declaration(s)
    // ====================================================================== //

    class CsvDatabase : public IKeyValueDatabase {
    public:
        CsvDatabase(IFileSystem* fs, const char* path) : fs(fs), path(path), doc(path) {}

        bool GetRecordById(const char* tableName, uint64_t recordId, const StructType& record_type, void* record_out) final;

    private:
        IFileSystem* fs;
        string path;

        rapidcsv::Document doc;
    };

    // ====================================================================== //
    //  class CsvDatabase
    // ====================================================================== //

    unique_ptr<IKeyValueDatabase> IKeyValueDatabase::CreateCsvDatabase(IFileSystem *fs, const char *path) {
        return make_unique<CsvDatabase>(fs, path);
    }

    bool CsvDatabase::GetRecordById(const char* tableName, uint64_t recordId, const StructType& record_type, void* record_out) {
        for (size_t i = 0; i < record_type.numFields; i++) {
            try {
                auto value = doc.GetCell<string>(string(record_type.fieldNames[i]), std::to_string(recordId));

                auto accessor = record_type.AccessField(record_type, record_out, i);
                accessor.type.NewFromString(accessor.type, accessor.value, value.c_str());
            }
            catch (std::out_of_range& ex) {
                return false;
            }
        }

        return true;
    }
}
