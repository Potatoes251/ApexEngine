#ifndef SERIALIZATION_PARSER
#define SERIALIZATION_PARSER

#include "LibMath/Vector.h"
#include "LibMath/Quaternion.h"

#include <optional>

#include <fstream>

namespace Apex::Serialization
{
    class SerialParser
    {
    public:

        SerialParser(const std::string& text);

        void SkipWhitespace();
        void Expect(char c);

        std::string ParseString();
        float ParseFloat();
        int ParseInt();
        bool ParseBool();

        LibMath::Vector3 ParseVector3();
        LibMath::Vector4 ParseVector4();
        LibMath::Quaternion ParseQuaternion();
        char PeekChar();

        bool Match(const std::string& str);
        bool Peek(char c);

    private:
        const std::string& m_text;
        size_t m_index;
    };
}

#endif // !SERIALIZATION_PARSER

