#ifndef NODE_REQUEST_LOG_H_
#define NODE_REQUEST_LOG_H_

#include <boost/thread/mutex.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <3rdparty/msgpack/msgpack.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <memory.h>
#include <vector>
#include <set>
#include <map>

namespace sf1r
{

enum ReqLogType
{
    Req_None = 0,
    // if the write request can be redo correctly just using the json data from the
    // request then you can use this type of request. Otherwise, you should define a new
    // request type and add addition member data to it.
    Req_NoAdditionDataReq = 1,
    // this kind of request need backup before processing.
    Req_NoAdditionData_NeedBackup_Req,
    // if the request will not change any data if failed.
    // Then you can use this kind. NoRollback means no backup too.
    Req_NoAdditionDataNoRollback,
    // used for handle cron job task.
    Req_CronJob,
    // this kind of request is remote callback.
    // During a write request on node A, send some write with parameter to other shard nodes.
    // Note: If the write request has an api method, please do not use callback.
    Req_Callback,
    // this request has only a timestamp with it.
    Req_WithTimestamp,
    // index request need the scd file list which is not included in the request json data,
    // so we define a new request type, and add addition member.
    Req_Index,
    Req_CreateOrUpdate_Doc,
    Req_Product,
    Req_UpdateConfig,
    Req_Recommend_Index,
    Req_Rebuild_FromSCD,
};

#pragma pack(1)
struct ReqLogHead
{
    uint32_t inc_id;
    uint32_t reqtype;
    uint32_t req_data_offset;
    uint32_t req_data_len;
    uint32_t req_data_crc;
    //char reqtime[16];
};
#pragma pack()

struct CommonReqData
{
    uint32_t inc_id;
    uint32_t reqtype;
    std::string req_json_data;
    CommonReqData() : inc_id(0), reqtype(Req_None) {}
    virtual ~CommonReqData(){}
    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        pk.pack(inc_id);
        pk.pack(reqtype);
        pk.pack(req_json_data);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&inc_id);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&reqtype);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&req_json_data);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack common data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct NoAdditionReqLog: public CommonReqData
{
    NoAdditionReqLog()
    {
        reqtype = Req_NoAdditionDataReq;
    }
};

struct NoAdditionNeedBackupReqLog: public CommonReqData
{
    NoAdditionNeedBackupReqLog()
    {
        reqtype = Req_NoAdditionData_NeedBackup_Req;
    }
};

struct NoAdditionNoRollbackReqLog: public CommonReqData
{
    NoAdditionNoRollbackReqLog()
    {
        reqtype = Req_NoAdditionDataNoRollback;
    }
};

struct CronJobReqLog: public CommonReqData
{
    // note : for cron job, the req_json_data in CommonReqData
    // is the job name.
    CronJobReqLog()
    {
        reqtype = Req_CronJob;
        cron_time = 0;
    }
    int64_t cron_time;
    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(cron_time);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&cron_time);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack Req_CronJob data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct TimestampReqLog: public CommonReqData
{
    TimestampReqLog()
    {
        reqtype = Req_WithTimestamp;
        timestamp = 0;
    }
    int64_t timestamp;
    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(timestamp);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&timestamp);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack Req_WithTimestamp data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct CreateOrUpdateDocReqLog: public CommonReqData
{
    CreateOrUpdateDocReqLog()
    {
        reqtype = Req_CreateOrUpdate_Doc;
        timestamp = 0;
    }
    int64_t timestamp;
    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(timestamp);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&timestamp);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack Req_CreateOrUpdate_Doc data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct IndexReqLog: public CommonReqData
{
    IndexReqLog()
    {
        reqtype = Req_Index;
        timestamp = 0;
    }
    //CommonReqData common_data;
    // index request addition member for scd file list.
    std::vector<std::string> scd_list;
    int64_t timestamp;

    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(scd_list);
        pk.pack(timestamp);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&scd_list);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&timestamp);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack index data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct ProductReqLog: public CommonReqData
{
    ProductReqLog()
    {
        reqtype = Req_Product;
    }
    std::vector<std::string> str_uuid_list;

    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(str_uuid_list);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&str_uuid_list);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack ProductReqLog data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct UpdateConfigReqLog: public CommonReqData
{
    UpdateConfigReqLog()
    {
        reqtype = Req_UpdateConfig;
    }

    // (config_file_name, file_binary_content)
    std::map<std::string, std::string> config_file_list;

    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(config_file_list);
    }

    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&config_file_list);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack UpdateConfigReqLog data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct UpdateRecCallbackReqLog: public CommonReqData
{
    UpdateRecCallbackReqLog()
    {
        reqtype = Req_Callback;
    }
    std::list<uint32_t> oldItems;
    std::list<uint32_t> newItems;
    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(oldItems);
        pk.pack(newItems);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&oldItems);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&newItems);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack UpdateRecCallbackReqLog data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct BuildPurchaseSimCallbackReqLog: public CommonReqData
{
    BuildPurchaseSimCallbackReqLog()
    {
        reqtype = Req_Callback;
    }
    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        return true;
    }
};


struct RecommendIndexReqLog: public CommonReqData
{
    RecommendIndexReqLog()
    {
        reqtype = Req_Recommend_Index;
        timestamp = 0;
    }
    // build recommend need the user and order scd file list.
    std::vector<std::string> user_scd_list;
    std::vector<std::string> order_scd_list;
    int64_t timestamp;

    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(user_scd_list);
        pk.pack(order_scd_list);
        pk.pack(timestamp);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&user_scd_list);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&order_scd_list);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&timestamp);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack recommend index data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct RebuildCronTaskReqLog: public CommonReqData
{
    RebuildCronTaskReqLog()
    {
        reqtype = Req_CronJob;
    }
    int64_t cron_time;
    std::vector<uint32_t> replayed_id_list;

    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(cron_time);
        pk.pack(replayed_id_list);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&cron_time);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&replayed_id_list);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack RebuildCronTaskReqLog data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};

struct RebuildFromSCDReqLog: public CommonReqData
{
    RebuildFromSCDReqLog()
    {
        reqtype = Req_Rebuild_FromSCD;
        timestamp = 0;
    }
    std::vector<std::string> scd_list;
    int64_t timestamp;
    std::vector<uint32_t> replayed_id_list;

    virtual void pack(msgpack::packer<msgpack::sbuffer>& pk) const
    {
        CommonReqData::pack(pk);
        pk.pack(scd_list);
        pk.pack(timestamp);
        pk.pack(replayed_id_list);
    }
    virtual bool unpack(msgpack::unpacker& unpak)
    {
        if (!CommonReqData::unpack(unpak))
            return false;
        try
        {
            msgpack::unpacked msg;
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&scd_list);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&timestamp);
            if (!unpak.next(&msg))
                return false;
            msg.get().convert(&replayed_id_list);
        }
        catch(const std::exception& e)
        {
            std::cerr << "unpack RebuildFromSCDReqLog data error: " << e.what() << std::endl;
            return false;
        }
        return true;
    }
};



static const uint32_t _crc32tab[] = {
    0x0,0x77073096,0xee0e612c,0x990951ba,0x76dc419,0x706af48f,0xe963a535,0x9e6495a3,0xedb8832,0x79dcb8a4,
    0xe0d5e91e,0x97d2d988,0x9b64c2b,0x7eb17cbd,0xe7b82d07,0x90bf1d91,0x1db71064,0x6ab020f2,0xf3b97148,0x84be41de,
    0x1adad47d,0x6ddde4eb,0xf4d4b551,0x83d385c7,0x136c9856,0x646ba8c0,0xfd62f97a,0x8a65c9ec,0x14015c4f,0x63066cd9,
    0xfa0f3d63,0x8d080df5,0x3b6e20c8,0x4c69105e,0xd56041e4,0xa2677172,0x3c03e4d1,0x4b04d447,0xd20d85fd,0xa50ab56b,
    0x35b5a8fa,0x42b2986c,0xdbbbc9d6,0xacbcf940,0x32d86ce3,0x45df5c75,0xdcd60dcf,0xabd13d59,0x26d930ac,0x51de003a,
    0xc8d75180,0xbfd06116,0x21b4f4b5,0x56b3c423,0xcfba9599,0xb8bda50f,0x2802b89e,0x5f058808,0xc60cd9b2,0xb10be924,
    0x2f6f7c87,0x58684c11,0xc1611dab,0xb6662d3d,0x76dc4190,0x1db7106,0x98d220bc,0xefd5102a,0x71b18589,0x6b6b51f,
    0x9fbfe4a5,0xe8b8d433,0x7807c9a2,0xf00f934,0x9609a88e,0xe10e9818,0x7f6a0dbb,0x86d3d2d,0x91646c97,0xe6635c01,
    0x6b6b51f4,0x1c6c6162,0x856530d8,0xf262004e,0x6c0695ed,0x1b01a57b,0x8208f4c1,0xf50fc457,0x65b0d9c6,0x12b7e950,
    0x8bbeb8ea,0xfcb9887c,0x62dd1ddf,0x15da2d49,0x8cd37cf3,0xfbd44c65,0x4db26158,0x3ab551ce,0xa3bc0074,0xd4bb30e2,
    0x4adfa541,0x3dd895d7,0xa4d1c46d,0xd3d6f4fb,0x4369e96a,0x346ed9fc,0xad678846,0xda60b8d0,0x44042d73,0x33031de5,
    0xaa0a4c5f,0xdd0d7cc9,0x5005713c,0x270241aa,0xbe0b1010,0xc90c2086,0x5768b525,0x206f85b3,0xb966d409,0xce61e49f,
    0x5edef90e,0x29d9c998,0xb0d09822,0xc7d7a8b4,0x59b33d17,0x2eb40d81,0xb7bd5c3b,0xc0ba6cad,0xedb88320,0x9abfb3b6,
    0x3b6e20c,0x74b1d29a,0xead54739,0x9dd277af,0x4db2615,0x73dc1683,0xe3630b12,0x94643b84,0xd6d6a3e,0x7a6a5aa8,
    0xe40ecf0b,0x9309ff9d,0xa00ae27,0x7d079eb1,0xf00f9344,0x8708a3d2,0x1e01f268,0x6906c2fe,0xf762575d,0x806567cb,
    0x196c3671,0x6e6b06e7,0xfed41b76,0x89d32be0,0x10da7a5a,0x67dd4acc,0xf9b9df6f,0x8ebeeff9,0x17b7be43,0x60b08ed5,
    0xd6d6a3e8,0xa1d1937e,0x38d8c2c4,0x4fdff252,0xd1bb67f1,0xa6bc5767,0x3fb506dd,0x48b2364b,0xd80d2bda,0xaf0a1b4c,
    0x36034af6,0x41047a60,0xdf60efc3,0xa867df55,0x316e8eef,0x4669be79,0xcb61b38c,0xbc66831a,0x256fd2a0,0x5268e236,
    0xcc0c7795,0xbb0b4703,0x220216b9,0x5505262f,0xc5ba3bbe,0xb2bd0b28,0x2bb45a92,0x5cb36a04,0xc2d7ffa7,0xb5d0cf31,
    0x2cd99e8b,0x5bdeae1d,0x9b64c2b0,0xec63f226,0x756aa39c,0x26d930a,0x9c0906a9,0xeb0e363f,0x72076785,0x5005713,
    0x95bf4a82,0xe2b87a14,0x7bb12bae,0xcb61b38,0x92d28e9b,0xe5d5be0d,0x7cdcefb7,0xbdbdf21,0x86d3d2d4,0xf1d4e242,
    0x68ddb3f8,0x1fda836e,0x81be16cd,0xf6b9265b,0x6fb077e1,0x18b74777,0x88085ae6,0xff0f6a70,0x66063bca,0x11010b5c,
    0x8f659eff,0xf862ae69,0x616bffd3,0x166ccf45,0xa00ae278,0xd70dd2ee,0x4e048354,0x3903b3c2,0xa7672661,0xd06016f7,
    0x4969474d,0x3e6e77db,0xaed16a4a,0xd9d65adc,0x40df0b66,0x37d83bf0,0xa9bcae53,0xdebb9ec5,0x47b2cf7f,0x30b5ffe9,
    0xbdbdf21c,0xcabac28a,0x53b39330,0x24b4a3a6,0xbad03605,0xcdd70693,0x54de5729,0x23d967bf,0xb3667a2e,0xc4614ab8,
    0x5d681b02,0x2a6f2b94,0xb40bbe37,0xc30c8ea1,0x5a05df1b,0x2d02ef8d};


// each log file will store 1000 write request.
// 1 - 99999  saved in 0.req.log
// 100000 - 199999 saved in 1.req.log and so on.
// head.req.log store the offset, length and some other info for each write request.
class ReqLogMgr
{
public:
    ReqLogMgr()
        :inc_id_(1),
        last_writed_id_(0)
    {
    }

    static void initWriteRequestSet();
    static inline bool isWriteRequest(const std::string& controller, const std::string& action)
    {
        if (write_req_set_.find(controller + "_" + action) != write_req_set_.end())
            return true;
        return false;
    }
    static inline bool isReplayWriteReq(const std::string& controller, const std::string& action)
    {
        if (replay_write_req_set_.find(controller + "_" + action) != replay_write_req_set_.end())
            return true;
        return false;
    }
    static inline bool isAutoShardWriteReq(const std::string& controller, const std::string& action)
    {
        if (auto_shard_write_set_.find(controller + "_" + action) != auto_shard_write_set_.end())
            return true;
        return false;
    }
    static void packReqLogData(const CommonReqData& reqdata, std::string& packed_data)
    {
        msgpack::sbuffer buf;
        msgpack::packer<msgpack::sbuffer> pk(&buf);
        reqdata.pack(pk);
        packed_data.assign(buf.data(), buf.size());
    }
    static bool unpackReqLogData(const std::string& packed_data, CommonReqData& reqdata)
    {
        msgpack::unpacker unpak;
        unpak.reserve_buffer(packed_data.size());
        memcpy(unpak.buffer(), packed_data.data(), packed_data.size());
        unpak.buffer_consumed(packed_data.size());
        return reqdata.unpack(unpak);
    }

    static void replaceCommonReqData(const CommonReqData& old_common, const CommonReqData& new_common, std::string& packed_data)
    {
        std::string old_common_packed_data;
        packReqLogData(old_common, old_common_packed_data);
        std::string new_common_packed_data;
        packReqLogData(new_common, new_common_packed_data);
        packed_data.replace(0, old_common_packed_data.size(), new_common_packed_data);
    }

    static uint32_t crc(uint32_t crc, const char* data, const int32_t len)
    {
        int32_t i;
        for (i = 0; i < len; ++i)
        {
            crc = (crc >> 8) ^ _crc32tab[(crc ^ (*data)) & 0xff];
            data++;
        }
        return (crc);
    }

    inline std::string getRequestLogPath() const
    {
        return base_path_;
    }
    void init(const std::string& basepath);
    bool prepareReqLog(CommonReqData& prepared_reqdata, bool isprimary);
    bool getPreparedReqLog(CommonReqData& reqdata);
    void delPreparedReqLog();
    bool appendTypedReqLog(CommonReqData& reqdata)
    {
        std::string packed_data;
        packReqLogData(reqdata, packed_data);
        return appendReqData(packed_data);
    }

    bool appendReqData(const std::string& req_packed_data);
    inline uint32_t getLastSuccessReqId() const
    {
        return last_writed_id_;
    }
    // you can use this to read all request data in sequence.
    bool getReqDataByHeadOffset(size_t& headoffset, ReqLogHead& rethead, std::string& req_packed_data);
    // get request with inc_id or the minimize that not less than inc_id if not exist.
    // if no more request return false.
    bool getReqData(uint32_t& inc_id, ReqLogHead& rethead, size_t& headoffset, std::string& req_packed_data);
    bool getHeadOffset(uint32_t& inc_id, ReqLogHead& rethead, size_t& headoffset);

    void getReqLogIdList(uint32_t start, uint32_t max_return, bool needdata,
        std::vector<uint32_t>& req_logid_list,
        std::vector<std::string>& req_logdata_list);

private:
    bool getHeadOffsetWithoutLock(uint32_t& inc_id, ReqLogHead& rethead, size_t& headoffset);
    bool getReqPackedDataByHead(const ReqLogHead& head, std::string& req_packed_data);
    std::string getDataPath(uint32_t inc_id);
    ReqLogHead getHeadData(std::ifstream& ifs, size_t offset);
    void loadLastData();

    std::string base_path_;
    std::string head_log_path_;
    uint32_t  inc_id_;
    uint32_t  last_writed_id_;
    std::vector<CommonReqData> prepared_req_;
    std::map<uint32_t, size_t> cached_head_offset_;
    boost::mutex  lock_;

    static std::set<std::string> write_req_set_;
    static std::set<std::string> replay_write_req_set_;
    static std::set<std::string> auto_shard_write_set_;
};

}
#endif
