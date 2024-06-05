// (C) Copyright 2022 NOAA/NWS/NCEP/EMC

#pragma once

#include <algorithm>
#include <string>


namespace bufr {
    struct SubsetVariant
    {
        std::string subset;
        size_t variantId;
        bool otherVariantsExist = false;

        SubsetVariant() = default;
        SubsetVariant(const std::string& subset, size_t varientId, bool otherVariantsExist = false):
            subset(subset),
            variantId(varientId),
            otherVariantsExist(otherVariantsExist)
        {
        }

        std::string str() const
        {
            if (otherVariantsExist)
            {
                return subset + "[" + std::to_string(variantId) + "]";
            }
            else
            {
                return subset;
            }
        }

        bool operator< (const SubsetVariant& right) const
        {
            return str() < right.str();
        }

        bool operator== (const SubsetVariant& right) const
        {
            return (subset == right.subset) && (variantId == right.variantId);
        }
    };
}  // namespace bufr

// Implement a version of the hash function for ths Subset Variant class so that we may use it as a
// unordered map key.
namespace std
{
    template<>
    struct hash<bufr::SubsetVariant>
    {
        std::size_t operator()(const bufr::SubsetVariant &k) const
        {
            return std::hash<string>()(k.subset) ^ (std::hash<size_t>()(k.variantId) << 1);
        }
    };
}  // namespace std
