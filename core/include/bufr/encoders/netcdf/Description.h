/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>
#include <netcdf>

#include "eckit/config/LocalConfiguration.h"
#include "bufr/QueryParser.h"

namespace nc = netCDF;

namespace bufr {
namespace encoders {
namespace netcdf {
    struct Range
    {
        float start;
        float end;
    };

    struct DimensionDescription
    {
        std::string name;
        std::vector<Query> paths;
        std::string source;
    };

    struct VariableDescription
    {
        std::string name;
        std::string source;
        std::vector<std::string> dimensions;
        std::string longName;
        std::string units;
        std::shared_ptr<std::string> coordinates;  // Optional
        std::shared_ptr<Range> range;  // Optional
//        std::vector<ioda::Dimensions_t> chunks;  // Optional
        int compressionLevel;  // Optional
    };

    struct GlobalDescriptionBase
    {
        std::string name;
        virtual void addTo(nc::NcFile& file) = 0;
        virtual ~GlobalDescriptionBase() = default;
    };

    template<typename T>
    struct is_vector : public std::false_type {};

    template<typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type {};

    template<typename T>
    struct GlobalDescription : public GlobalDescriptionBase
    {
        T value;

        nc::NcType getNcType()
        {
            if (std::is_same<T, float>::value)
            {
                return nc::NcType::nc_FLOAT;
            }
            else if (std::is_same<T, double>::value)
            {
                return nc::NcType::nc_DOUBLE;
            }
            else if (std::is_same<T, uint32_t>::value)
            {
                return nc::NcType::nc_UINT;
            }
            else if (std::is_same<T, uint64_t>::value)
            {
                return nc::NcType::nc_UINT64;
            }
            else if (std::is_same<T, int32_t>::value)
            {
                return nc::NcType::nc_INT;
            }
            else if (std::is_same<T, int64_t>::value)
            {
                return nc::NcType::nc_INT64;
            }
            else if (std::is_same<T, std::string>::value)
            {
                return nc::NcType::nc_CHAR;
            }
            else
            {
                throw eckit::BadParameter("Unsupported global attribute type");
            }
        }

        void addTo(nc::NcFile& file) final
        {
            _addTo(file);
        }

     private:
        // T is something other than a std::vector
        template<typename U = void>
        void _addTo(nc::NcFile& file,
                    typename std::enable_if<!is_vector<T>::value, U>::type* = nullptr)
        {
            file.putAtt(name, getNcType(), value);
        }

        // T is a vector
        template<typename U = void>
        void _addTo(nc::NcFile& file,
                    typename std::enable_if<is_vector<T>::value, U>::type* = nullptr)
        {
            file.putAtt(name, getNcType(), value.size(), value.data());
        }
    };

    template<>
    struct GlobalDescription<std::string> : public GlobalDescriptionBase
    {
        std::string value;

        void addTo(nc::NcFile& file) final
        {
            file.putAtt(name, value);
        }
    };

    typedef std::vector<DimensionDescription> DimDescriptions;
    typedef std::vector<VariableDescription> VariableDescriptions;
    typedef std::vector<std::shared_ptr<GlobalDescriptionBase>> GlobalDescriptions;

    /// \brief Describes how to write data to IODA.
    class Description
    {
     public:
        Description() = default;
        explicit Description(const std::string& yamlFile);
        explicit Description(const eckit::Configuration& conf);

        /// \brief Add Dimension defenition
        void addDimension(const DimensionDescription& dim);

        /// \brief Add Variable defenition
        void addVariable(const VariableDescription& variable);

        /// \brief Add Variable defenition
        void py_addVariable(const std::string& name,
                         const std::string& source,
                         const std::string& unit,
                         const std::string& longName = "");

        /// \brief Add Globals defenition
        void addGlobal(const std::shared_ptr<GlobalDescriptionBase>& global);

        // Getters
        /// \brief Get the descriptions for the dimensions
        inline DimDescriptions getDims() const { return dimensions_; }

        /// \brief Get the description of the variables
        inline VariableDescriptions getVariables() const { return variables_; }

        /// \brief Get the description of the global attributes
        inline GlobalDescriptions getGlobals() const { return globals_; }

        /// \brief Get the output path template
        inline std::string getOutputPathTemplate() const { return outputPathTemplate_; }

     private:
        /// \brief The template to use output file to create
        std::string outputPathTemplate_;

        /// \brief Collection of defined dimensions
        DimDescriptions dimensions_;

        /// \brief Collection of defined variables
        VariableDescriptions variables_;

        /// \brief Collection of defined globals
        GlobalDescriptions globals_;

        void init(const eckit::Configuration& conf);
    };
}  // namespace netcdf
}  // namespace encoders
}  // namespace bufr
