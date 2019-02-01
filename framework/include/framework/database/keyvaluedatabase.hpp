#ifndef framework_database_database_hpp
#define framework_database_database_hpp

#include <framework/base.hpp>

namespace zfw
{
    class IKeyValueDatabase {
    public:
        static unique_ptr<IKeyValueDatabase> CreateCsvDatabase(IFileSystem *fs, const char *path);

        virtual bool GetRecordById(const char* tableName, uint64_t recordId, const StructType& record_type, void* record_out) = 0;

        template<class RecordStruct>
        bool ById(const char* tableName, uint64_t recordId, RecordStruct* data_out) {
            return this->GetRecordById(tableName, recordId, GetStruct<RecordStruct>(), data_out);
        }
    };

//    class IBinaryRecordDatabase {
//    public:
//        virtual bool GetRecordById(const char* tableName, uint64_t recordId, const uint8_t** record_out, size_t* length_out) = 0;
//
//        template <class RecordStruct>
//        bool ById(uint64_t recordId, RecordStruct* data_out) {
//            auto& mapper = GetRecordMapper<RecordStruct>();
//
//            const uint8_t* record;
//            size_t length;
//
//            if (!this->GetRecordById(mapper.GetTableName(), recordId, &record, &length)) {
//                return false;
//            }
//
//            if (!mapper.FromRecord(record, length, data_out)) {
//                return false;
//            }
//
//            return true;
//        }
//    };
}

#endif
