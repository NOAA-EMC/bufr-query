/*
 * (C) Copyright 2020 NOAA/NWS/NCEP/EMC
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 */

#pragma once

#include <set>
#include <memory>
#include <string>
#include <vector>


#include "eckit/config/LocalConfiguration.h"
#include "bufr/QueryParser.h"


namespace bufr {
namespace encoders {
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
        std::vector<size_t> chunks;  // Optional
        int compressionLevel;  // Optional
    };

    struct GlobalWriterBase
    {
      public:
      GlobalWriterBase() = default;
      virtual ~GlobalWriterBase() = default;
    };

    template<typename T>
    struct GlobalWriter : public GlobalWriterBase
    {
      public:
        GlobalWriter() = default;
        virtual ~GlobalWriter() = default;
        virtual void write(const std::string& name, const T& data) = 0;
    };

    struct GlobalDescriptionBase
    {
        std::string name;
        virtual void writeTo(const std::shared_ptr<GlobalWriterBase>& writer) = 0;
        virtual ~GlobalDescriptionBase() = default;
    };

    template<typename T>
    struct GlobalDescription : public GlobalDescriptionBase
    {
        T value;

        void writeTo(const std::shared_ptr<GlobalWriterBase>& writer) final
        {
          if (auto writerPtr = std::dynamic_pointer_cast<GlobalWriter<T>>(writer))
          {
            writerPtr->write(name, value);
          }
          else
          {
            throw std::runtime_error("Invalid writer type");
          }
        }
    };

    typedef std::vector<DimensionDescription> DimDescriptions;
    typedef std::vector<VariableDescription> VariableDescriptions;
    typedef std::vector<std::shared_ptr<GlobalDescriptionBase>> GlobalDescriptions;

    /// \brief Describes how to write data to NetCDF.
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

        /// \brief Initialize the object from a configuration
        void init(const eckit::Configuration& conf);
    };
}  // namespace encoders
}  // namespace bufr
