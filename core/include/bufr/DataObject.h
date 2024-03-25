/*
* (C) Copyright 2024 NOAA/NWS/NCEP/EMC
*
* This software is licensed under the terms of the Apache Licence Version 2.0
* which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
*/

#pragma once

#include <memory>
#include <iostream>
#include <vector>
#include <netcdf>

#include "encoders/ElementWriter.h"
#include "QueryParser.h"
#include "Data.h"

namespace nc = netCDF;


namespace bufr {

  struct Data;
  typedef std::vector<int> Dimensions;
  typedef Dimensions Location;

  struct DimensionDataBase
  {
    virtual ~DimensionDataBase() = default;
    virtual size_t size() = 0;

    virtual void write(const nc::NcVar var) = 0;

//    virtual void write(std::function<void(encoders::ElementWriter&)> writer) = 0;

//    template<typename U>
//    virtual void write(const encoders::ElementWriter<U>& writer) = 0;
  };

  template<typename T>
  struct DimensionData : public DimensionDataBase
  {
    std::string name;
    std::vector<T> data;

    DimensionData() = delete;

    virtual ~DimensionData() = default;

    explicit DimensionData(const std::string& dimname, size_t size) :
        name(dimname),
        data(std::vector<T>(size, _default()))
    {
    }

    size_t size() final
    {
        return data.size();
    }

    void write(const nc::NcVar var)
    {
        var.putVar(data.data());
    }

  private:
    template<typename U = void>
    T _default(typename std::enable_if<std::is_arithmetic<T>::value, U>::type* = nullptr)
    {
      return static_cast<T>(0);
    }

    template<typename U = void>
    T _default(typename std::enable_if<std::is_same<T, std::string>::value, U>::type* = nullptr)
    {
      return std::string("");
    }
  };

  class DataObjectBase
  {
    public:
      DataObjectBase() = default;
      virtual ~DataObjectBase() = default;

      bool hasSamePath(const std::shared_ptr<DataObjectBase>& dataObject);

      /// \brief Print the data object to a output stream.
      virtual void print(std::ostream& out) const = 0;

      /// \brief Get the data at the location as an integer.
      /// \return Integer data.
      virtual int getAsInt(const Location& loc) const = 0;

      /// \brief Get the data at the location as an float.
      /// \return Float data.
      virtual float getAsFloat(const Location& loc) const = 0;

      /// \brief Get the data at the Location as an string.
      /// \return String data.
      virtual std::string getAsString(const Location& loc) const = 0;

      /// \brief Is the element at the location the missing value.
      /// \return bool data.
      virtual bool isMissing(const Location& loc) const = 0;

      /// \brief Get the data at the index as an int.
      /// \return Int data.
      virtual int getAsInt(size_t idx) const = 0;

      /// \brief Get the data at the index as an float.
      /// \return Float data.
      virtual float getAsFloat(size_t idx) const = 0;

      /// \brief Get the data at the Location as an string.
      /// \return String data.
      virtual std::string getAsString(size_t idx) const = 0;

      /// \brief Is the element at the index the missing value.
      /// \return bool data.
      virtual bool isMissing(size_t idx) const = 0;

      /// \brief Multiply the stored values in this data object by a scalar.
      /// \param val Scalar to multiply to the data..
      virtual void multiplyBy(double val) = 0;

      /// \brief Add a scalar to the stored values in this data object.
      /// \param val Scalar to add to the data..
      virtual void offsetBy(double val) = 0;

      /// \brief Makes an ioda::Variable and adds it to the given ioda::ObsGroup
      /// \param obsGroup Obsgroup where to add the variable
      /// \param name The name to associate with the variable (ex "MetaData/latitude")
      /// \param dimensions List of Variables to use as the dimensions for this new variable
      /// \param chunks List of integers specifying the chunking dimensions
      /// \param compressionLevel The GZip compression level to use, must be 0-9
      virtual nc::NcVar createVariable(nc::NcGroup& group,
                                       const std::string& name,
                                       const std::vector<nc::NcVar>& dimensions,
                                       const std::vector<nc::NcVar>& chunks,
                                       int compressionLevel) const = 0;

      /// \brief Makes a new dimension scale using this data object as the source
      /// \param name The name of the dimension variable.
      /// \param dimIdx The idx of the data dimension to use.
      virtual std::shared_ptr<DimensionDataBase> createDimensionFromData(
        const std::string& name,
        std::size_t dimIdx) const = 0;

      virtual std::shared_ptr<DataObjectBase> slice(const std::vector<std::size_t>& rows) const = 0;

      virtual size_t size() const = 0;

      size_t idxFromLoc(const Location& loc) const
      {
        size_t dim_prod = 1;
        for (int dim_idx = dims_.size(); dim_idx > static_cast<int>(loc.size()); --dim_idx) {
          dim_prod *= dims_[dim_idx];
        }

        // Compute the index into the data array
        size_t index = 0;
        for (int dim_idx = loc.size() - 1; dim_idx >= 0; --dim_idx) {
          index += dim_prod * loc[dim_idx];
          dim_prod *= dims_[dim_idx];
        }

        return index;
      }

      /// \brief Makes a new blank dimension scale with default type.
      /// \param name The name of the dimension variable.
      /// \param dimIdx The idx of the data dimension to use.
      std::shared_ptr<DimensionDataBase> createEmptyDimension(const std::string& name,
                                                              std::size_t dimIdx) const
      {
        auto dimData = std::make_shared<DimensionData<int>>(name, getDims()[dimIdx]);
        return dimData;
      }

      // Setters
      void setFieldName(const std::string& fieldName);
      void setGroupByFieldName(const std::string& fieldName);
      void setDims(const std::vector<int> dims);
      void setQuery(const std::string& query);
      void setDimPaths(const std::vector<Query>& dimPaths);
      virtual void setData(const Data& data) = 0;

      // Getters
      std::string getFieldName() const { return fieldName_; }
      std::string getGroupByFieldName() const { return groupByFieldName_; }
      Dimensions getDims() const { return dims_; }
      std::string getPath() const { return query_; }
      std::vector<Query> getDimPaths() const { return dimPaths_; }


    protected:
      std::string fieldName_;
      std::string groupByFieldName_;
      std::vector<int> dims_;
      std::string query_;
      std::vector<Query> dimPaths_;
  };

  template <typename T>
  class DataObject : public DataObjectBase
  {
    public:

      /// \brief Get the missing value for this data type.
      constexpr static T missingValue()
      {
        return std::numeric_limits<T>::max();
      }

      DataObject() = default;

      /// \brief Print the data object to a output stream.
      void print(std::ostream& out) const final
      {
        out << "DataObjectImpl";
      }

      /// \brief Get the data at the location as an integer.
      /// \return Integer data.
      int getAsInt(const Location& loc) const final
      {
        return static_cast<int>(get(loc));
      }

      /// \brief Get the data at the location as an float.
      /// \return Float data.
      float getAsFloat(const Location& loc) const final
      {
        return static_cast<float>(get(loc));
      }

      /// \brief Get the data at the Location as an string.
      /// \return String data.
      std::string getAsString(const Location& loc) const final
      {
        return std::to_string(get(loc));
      }

      /// \brief Is the element at the location the missing value.
      /// \return bool data.
      bool isMissing(const Location& loc) const final
      {
        return isMissing(idxFromLoc(loc));
      }

      /// \brief Get the data at the index as an int.
      /// \return Int data.
      int getAsInt(size_t idx) const final
      {
        return static_cast<int>(data_[idx]);
      }

      /// \brief Get the data at the index as an float.
      /// \return Float data.
      float getAsFloat(size_t idx) const final
      {
        return static_cast<float>(data_[idx]);
      }

      /// \brief Get the data at the index as an string.
      /// \return String data.
      std::string getAsString(size_t idx) const final
      {
        return std::to_string(data_[idx]);
      }

      /// \brief Is the element at the index the missing value.
      /// \return bool data.
      bool isMissing(size_t idx) const final
      {
        return data_[idx] == missingValue();
      }

      /// \brief Get data associated with a given location.
      /// \param location The location to get data for.
      /// \return The data at the given location.
      T get(const Location& loc) const
      {
        return data_[idxFromLoc(loc)];
      };

      /// \brief Multiply the stored values in this data object by a scalar.
      /// \param val Scalar to multiply to the data..
      void multiplyBy(double val) final
      {
        if (typeid(T) == typeid(float) ||   // NOLINT
            typeid(T) == typeid(double) ||  // NOLINT
            trunc(val) == val)
        {
          for (size_t i = 0; i < data_.size(); i++)
          {
            if (data_[i] != missingValue())
            {
              data_[i] = static_cast<T>(static_cast<double>(data_[i]) * val);
            }
          }
        }
        else
        {
          std::ostringstream str;
          str << "Multiplying integer field \"" << fieldName_ << "\" with a non-integer is ";
          str << "illegal. Please convert it to a float or double.";
          throw eckit::BadParameter(str.str());
        }
      }

      /// \brief Add a scalar to the stored values in this data object.
      /// \param val Scalar to add to the data.
      void offsetBy(double val) final
      {
        for (size_t i = 0; i < data_.size(); i++)
        {
          if (data_[i] != missingValue())
          {
            data_[i] = data_[i] + static_cast<T>(val);
          }
        }
      }

      /// \brief Set the data associated with this data object (numeric DataObject).
      /// \param data The raw data
      /// \param dataMissingValue The number that represents missing values within the raw data
      void setData(const Data& data) final
      {
        if (data.isLongStr())
        {
          std::ostringstream str;
          str << "Can't make numerical field from string data.";
          throw eckit::BadParameter(str.str());
        }
        else
        {
          data_ = std::vector<T>(data.size());
          for (size_t idx = 0; idx < data.size(); ++idx)
          {
            if (!data.isMissing(idx))
            {
              data_[idx] = data.value.octets[idx];
            }
            else
            {
              data_[idx] = missingValue();
            }
          }
        }
      }

      // \brief Set the data associated with this data object.
      void setData(const std::vector<T>& data)
      {
        data_ = data;
      }

      /// \brief Makes an ioda::Variable and adds it to the given ioda::ObsGroup
      /// \param obsGroup Obsgroup were to add the variable
      /// \param name The name to associate with the variable (ex "MetaData/latitude")
      /// \param dimensions List of Variables to use as the dimensions for this new variable
      /// \param chunks List of integers specifying the chunking dimensions
      /// \param compressionLevel The GZip compression level to use, must be 0-9
      virtual nc::NcVar createVariable(nc::NcGroup& group,
                                       const std::string& name,
                                       const std::vector<nc::NcVar>& dimensions,
                                       const std::vector<nc::NcVar>& chunks,
                                       int compressionLevel) const final
      {
//        auto params = makeCreationParams(chunks, compressionLevel);
//        auto var = obsGroup.vars.createWithScales<T>(name, dimensions, params);
//        var.write(data_);
//        return var;
      };

      /// \brief Makes a new dimension scale using this data object as the source
      /// \param name The name of the dimension variable.
      /// \param dimIdx The idx of the data dimension to use.
      std::shared_ptr<DimensionDataBase> createDimensionFromData(const std::string& name,
                                                                 std::size_t dimIdx) const final
      {
        auto dimData = std::make_shared<DimensionData<T>>(name, getDims()[dimIdx]);

        if (data_.empty())
        {
          return dimData;
        }

        std::copy(data_.begin(),
                  data_.begin() + dimData->data.size(),
                  dimData->data.begin());

        // Validate this data object is a valid (has values that repeat for each frame
        for (size_t idx = 0; idx < data_.size(); idx += dimData->data.size())
        {
          if (!std::equal(data_.begin(),
                          data_.begin() + dimData->data.size(),
                          data_.begin() + idx,
                          data_.begin() + idx + dimData->data.size()))
          {
            std::stringstream errStr;
            errStr << "Dimension " << name << " has an invalid source field. ";
            errStr << "The values don't repeat in each sequence.";
            throw eckit::BadParameter(errStr.str());
          }
        }

        return dimData;
      }

      /// \brief Get the raw data associated with this data object.
      /// \return The raw data.
      std::vector<T> getRawData() const { return data_; }

      /// \brief Get the size of the data object.
      /// \return The size of the data object.
      size_t size() const final
      {
        return data_.size();
      }

      /// \brief Slice the data object according to a list of indices.
      /// \param rows The indices to slice the data object by.
      /// \return Sliced DataObject.
      std::shared_ptr<DataObjectBase> slice(const std::vector<std::size_t>& rows) const final
      {
        // Compute product of extra dimensions)
        std::size_t extraDims = 1;
        for (std::size_t i = 1; i < dims_.size(); ++i)
        {
          extraDims *= dims_[i];
        }

        // Make new DataObject with the rows we want
        std::vector<T> newData;
        newData.reserve(rows.size() * extraDims);
        for (std::size_t i = 0; i < rows.size(); ++i)
        {
          newData.insert(newData.end(),
                         data_.begin() + rows[i] * extraDims,
                         data_.begin() + (rows[i] + 1) * extraDims);
        }

        auto sliceDims = dims_;
        sliceDims[0] = rows.size();

        auto slicedDataObject = std::make_shared<DataObject<T>>();

        slicedDataObject->setData(newData);
        slicedDataObject->setFieldName(fieldName_);
        slicedDataObject->setGroupByFieldName(groupByFieldName_);
        slicedDataObject->setDims(sliceDims);
        slicedDataObject->setQuery(query_);
        slicedDataObject->setDimPaths(dimPaths_);

        return slicedDataObject;
      }

      friend class DataObjectBuilder;

    private:
      std::vector<T> data_;

//      /// \brief Make the variable creation parameters.
//      /// \param chunks The chunk sizes
//      /// \param compressionLevel The compression level
//      /// \return The variable creation patterns.
//      ioda::VariableCreationParameters makeCreationParams(
//        const std::vector<ioda::Dimensions_t>& chunks,
//        int compressionLevel) const
//      {
//        ioda::VariableCreationParameters params;
//        params.chunk = true;
//        params.chunks = chunks;
//        params.compressWithGZIP(compressionLevel);
//        params.setFillValue<T>(missingValue());
//
//        return params;
//      }
  };

  template<>
  class DataObject<std::string> : public DataObjectBase
  {
    public:
      DataObject() = default;

      static std::string missingValue()
      {
        return "";
      }

      /// \brief Print the data object to a output stream.
      void print(std::ostream& out) const final
      {
        out << "DataObjectImpl";
      }

      /// \brief Get the data at the location as an integer.
      /// \return Integer data.
      int getAsInt(const Location& loc) const final
      {
        throw eckit::BadParameter("Cannot convert string to int");
      };

      /// \brief Get the data at the location as an float.
      /// \return Float data.
      float getAsFloat(const Location& loc) const final
      {
        throw eckit::BadParameter("Cannot convert string to float");
      };

      /// \brief Get the data at the Location as an string.
      /// \return String data.
      virtual std::string getAsString(const Location& loc) const
      {
        return get(loc);
      }

      /// \brief Is the element at the location the missing value.
      /// \return bool data.
      bool isMissing(const Location& loc) const final
      {
        return get(loc) == "";
      }

      /// \brief Get the data at the index as an int.
      /// \return Int data.
      int getAsInt(size_t idx) const final
      {
        throw eckit::BadParameter("Cannot convert string to int");
      }

      /// \brief Get the data at the index as an float.
      /// \return Float data.
      float getAsFloat(size_t idx) const
      {
        throw eckit::BadParameter("Cannot convert string to float");
      }

      /// \brief Get the data at the index as an string.
      /// \return String data.
      std::string getAsString(size_t idx) const final
      {
        return data_[idx];
      }

      /// \brief Is the element at the index the missing value.
      /// \return bool data.
      bool isMissing(size_t idx) const final
      {
        return data_.at(idx) == "";
      }

      /// \brief Get data associated with a given location.
      /// \param location The location to get data for.
      /// \return The data at the given location.
      std::string get(const Location& loc) const
      {
        return data_[idxFromLoc(loc)];
      };

      /// \brief Multiply the stored values in this data object by a scalar (string version).
      /// \param val Scalar to multiply to the data.
      void multiplyBy(double val) final
      {
        throw eckit::BadParameter("Trying to multiply a string by a number");
      }

      /// \brief Add a scalar to the stored values in this data object (string version).
      /// \param val Scalar to add to the data.
      void offsetBy(double val) final
      {
        throw eckit::BadParameter("Trying to offset a string by a number");
      }

      /// \brief Set the data associated with this data object (string DataObject).
      /// \param data The raw data
      /// \param dataMissingValue The number that represents missing values within the raw data
      void setData( const Data& data) final
      {
        data_ = std::vector<std::string>();
        if (data.isLongStr())
        {
          data_ = data.value.strings;
        }
        else
        {
          auto charPtr = reinterpret_cast<const char *>(data.value.octets.data());
          for (size_t row_idx = 0; row_idx < data.size(); row_idx++)
          {
            if (!data.isMissing(row_idx))
            {
              std::string str = std::string(
                charPtr + row_idx * sizeof(double), sizeof(double));

              // trim trailing whitespace from str
              str.erase(std::find_if(str.rbegin(), str.rend(),
                                     [](char c) { return !std::isspace(c); }).base(),
                        str.end());

              data_.push_back(str);
            }
            else
            {
              data_.push_back("");
            }
          }
        }
      }

      /// \brief Set the data associated with this data object.
      /// \param data The raw data
      void setData(const std::vector<std::string>& data)
      {
        data_ = data;
      }

      /// \brief Makes an ioda::Variable and adds it to the given ioda::ObsGroup
      /// \param obsGroup Obsgroup were to add the variable
      /// \param name The name to associate with the variable (ex "MetaData/latitude")
      /// \param dimensions List of Variables to use as the dimensions for this new variable
      /// \param chunks List of integers specifying the chunking dimensions
      /// \param compressionLevel The GZip compression level to use, must be 0-9
      virtual nc::NcVar createVariable(nc::NcGroup& group,
                                       const std::string& name,
                                       const std::vector<nc::NcVar>& dimensions,
                                       const std::vector<nc::NcVar>& chunks,
                                       int compressionLevel) const
      {
//        auto params = makeCreationParams(chunks, compressionLevel);
//        auto var = obsGroup.vars.createWithScales<std::string>(name, dimensions, params);
//        var.write(data_);
//        return var;
      };

      /// \brief Makes a new dimension scale using this data object as the source
      /// \param name The name of the dimension variable.
      /// \param dimIdx The idx of the data dimension to use.
      std::shared_ptr<DimensionDataBase> createDimensionFromData(const std::string& name,
                                                                 std::size_t dimIdx) const final
      {
        auto dimData = std::make_shared<DimensionData<std::string>>(name, getDims()[dimIdx]);

        std::copy(data_.begin(),
                  data_.begin() + dimData->data.size(),
                  dimData->data.begin());

        // Validate this data object (has values that repeat for each frame
        for (size_t idx = 0; idx < data_.size(); idx += dimData->data.size())
        {
          if (!std::equal(data_.begin(),
                          data_.begin() + dimData->data.size(),
                          data_.begin() + idx,
                          data_.begin() + idx + dimData->data.size()))
          {
            std::stringstream errStr;
            errStr << "Dimension " << name << " has an invalid source field. ";
            errStr << "The values don't repeat in each sequence.";
            throw eckit::BadParameter(errStr.str());
          }
        }

        return dimData;
      }

      /// \brief Slice the data object according to a list of indices.
      /// \param rows The indices to slice the data object by.
      /// \return Sliced DataObject.
      std::shared_ptr<DataObjectBase> slice(const std::vector<std::size_t>& rows) const final
      {
        // Compute product of extra dimensions)
        std::size_t extraDims = 1;
        for (std::size_t i = 1; i < dims_.size(); ++i)
        {
          extraDims *= dims_[i];
        }

        // Make new DataObject with the rows we want
        std::vector<std::string> newData;
        newData.reserve(rows.size() * extraDims);
        for (std::size_t i = 0; i < rows.size(); ++i)
        {
          newData.insert(newData.end(),
                         data_.begin() + rows[i] * extraDims,
                         data_.begin() + (rows[i] + 1) * extraDims);
        }

        auto sliceDims = dims_;
        sliceDims[0] = rows.size();

        auto slicedDataObject = std::make_shared<DataObject<std::string>>();

        slicedDataObject->setData(newData);
        slicedDataObject->setFieldName(fieldName_);
        slicedDataObject->setGroupByFieldName(groupByFieldName_);
        slicedDataObject->setDims(sliceDims);
        slicedDataObject->setQuery(query_);
        slicedDataObject->setDimPaths(dimPaths_);

        return slicedDataObject;
      }

      /// \brief Get the raw data associated with this data object.
      /// \return The raw data.
      std::vector<std::string> getRawData() const { return data_; }

      /// \brief Get the size of the data object.
      /// \return The size of the data object.
      size_t size() const final
      {
        return data_.size();
      }

      friend class DataObjectBuilder;

    private:
      std::vector<std::string> data_;

//      /// \brief Make the variable creation parameters.
//      /// \param chunks The chunk sizes
//      /// \param compressionLevel The compression level
//      /// \return The variable creation patterns.
//      ioda::VariableCreationParameters makeCreationParams(
//        const std::vector<ioda::Dimensions_t>& chunks,
//        int compressionLevel) const
//      {
//        ioda::VariableCreationParameters params;
//        params.chunk = true;
//        params.chunks = chunks;
//        params.compressWithGZIP(compressionLevel);
//        params.setFillValue<std::string>("");
//
//        return params;
//      }
  };
}  // namespace bufr
