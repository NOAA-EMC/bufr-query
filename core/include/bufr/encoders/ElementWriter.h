//
// Created by Ronald McLaren on 3/20/24.
//

#pragma once

#include <string>

namespace bufr {
namespace encoders{

    class ElementWriter
    {
     public:
        ElementWriter() = default;
        virtual ~ElementWriter() = 0;

        void write(void* data)
        {
            _write(data);
        }

     private:
        virtual void _write(void* data) = 0;
    };

    template<typename T>
    class ElementWriterImpl : public ElementWriter
    {
     public:
        ElementWriterImpl() = default;
        ~ElementWriterImpl() = default;

        void _write(const std::vector<T>& data)
        {
            // Write the data
        }
    };

}  // namespace encoders
}  // namespace bufr
