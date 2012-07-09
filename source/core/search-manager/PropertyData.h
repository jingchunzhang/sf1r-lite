/**
 * @file PropertyData.h
 * @author Jun Jiang
 * @date Created <2011-11-10>
 * @brief for a specific property type, it stores each property value for each doc
 */
#ifndef PROPERTY_DATA_H
#define PROPERTY_DATA_H

#include <common/type_defs.h>

namespace sf1r{

struct PropertyData
{
    PropertyDataType type_;
    void* data_;
    size_t size_;
    time_t lastLoadTime_;
    bool temp_;

    PropertyData(PropertyDataType type, void* data, size_t size, bool temp = false)
        : type_(type) , data_(data) , size_(size), temp_(temp)
    {
        resetLoadTime();
    }

    void resetLoadTime()
    {
        lastLoadTime_ = std::time(NULL);
    }

    time_t elapsedFromLastLoad()
    {
        return (std::time(NULL) - lastLoadTime_);
    }

    ~PropertyData()
    {
        if (temp_)
        {
            switch (type_)
            {
            case INT32_PROPERTY_TYPE:
                delete[] (int32_t*)data_;
                break;

            case INT64_PROPERTY_TYPE:
            case DATETIME_PROPERTY_TYPE:
                delete[] (int64_t*)data_;
                break;

            case FLOAT_PROPERTY_TYPE:
                delete[] (float*)data_;
                break;

            case DOUBLE_PROPERTY_TYPE:
                delete[] (double*)data_;
                break;

            case STRING_PROPERTY_TYPE:
                delete[] (uint32_t*)data_;
                break;
            default:
                break;
            }
        }
    }
};

} // namespace sf1r

#endif // PROPERTY_DATA_H
