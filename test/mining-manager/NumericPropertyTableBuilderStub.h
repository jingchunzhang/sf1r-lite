///
/// @file NumericPropertyTableBuilderStub.h
/// @brief stub class to create NumericPropertyTable instance,
///        used to test groupby numeric property value.
/// @author Jun Jiang <jun.jiang@izenesoft.com>
/// @date Created 2011-09-07
///

#ifndef SF1R_NUMERIC_PROPERTY_TABLE_BUILDER_STUB_H
#define SF1R_NUMERIC_PROPERTY_TABLE_BUILDER_STUB_H

#include <search-manager/NumericPropertyTableBuilder.h>
#include <common/type_defs.h> // PropertyDataType
#include <common/PropertyValue.h>

#include <vector>
#include <string>

namespace sf1r
{

class Document;
class GroupConfig;

class NumericPropertyTableBuilderStub : public NumericPropertyTableBuilder
{
public:
    NumericPropertyTableBuilderStub(const std::vector<GroupConfig>& groupConfigs);

    boost::shared_ptr<NumericPropertyTableBase>& createPropertyTable(const std::string& propertyName);

    bool insertDocument(const Document& doc);

private:
    PropertyDataType getPropertyType_(const std::string& prop) const;

    bool insertProperty_(
        const std::string& prop,
        PropertyDataType type,
        const PropertyValue* propValue
    );

    void insertPropMap_(
        const std::string& prop,
        PropertyDataType type,
        const PropertyValue* propValue
    );

private:
    const std::vector<GroupConfig>& groupConfigs_;
    unsigned int lastDocId_;

    template <typename T>
    struct PropertyMap
    {
        typedef std::vector<T> table_type; // doc id => prop value
        typedef std::map<std::string, table_type> map_type; // prop name => prop value table
    };

    PropertyMap<int32_t>::map_type int32PropMap_;
    PropertyMap<int64_t>::map_type int64PropMap_;
    PropertyMap<float>::map_type floatPropMap_;
    PropertyMap<double>::map_type doublePropMap_;
    std::map<std::string, boost::shared_ptr<NumericPropertyTableBase> > numericPropertyTableMap_;
};

}

#endif
