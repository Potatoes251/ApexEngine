#include "SerializationParser.h"

using namespace Apex::Serialization;

SerialParser::SerialParser(const std::string& text)
    : m_text(text), m_index(0) {}

void SerialParser::SkipWhitespace()
{
    while (m_index < m_text.size() && std::isspace(m_text[m_index]))
    {
        m_index++;
    }
}

void SerialParser::Expect(char c)
{
    SkipWhitespace();

    if (m_text[m_index] != c)
        throw std::runtime_error("Unexpected character");

    m_index++;
}

bool SerialParser::Peek(char c)
{
    SkipWhitespace();
    return m_text[m_index] == c;
}

std::string SerialParser::ParseString()
{
    SkipWhitespace();

    if (m_text[m_index] != '"')
        throw std::runtime_error("Expected string");

    m_index++;

    size_t start = m_index;

    while (m_text[m_index] != '"')
        m_index++;

    std::string result = m_text.substr(start, m_index - start);

    m_index++;

    return result;
}

float SerialParser::ParseFloat()
{
    SkipWhitespace();

    size_t start = m_index;

    while (m_index < m_text.size() &&
        (isdigit(m_text[m_index]) ||
            m_text[m_index] == '.' ||
            m_text[m_index] == '-' ||
            m_text[m_index] == 'e'))
    {
        m_index++;
    }

    return std::stof(m_text.substr(start, m_index - start));
}

int SerialParser::ParseInt()
{
    return static_cast<int>(ParseFloat());
}

bool SerialParser::ParseBool()
{
    SkipWhitespace();

    if (Match("true"))
        return true;

    if (Match("false"))
        return false;

    throw std::runtime_error("Invalid bool");
}

bool SerialParser::Match(const std::string& str)
{
    SkipWhitespace();

    if (m_text.substr(m_index, str.size()) == str)
    {
        m_index += str.size();
        return true;
    }

    return false;
}

LibMath::Vector3 SerialParser::ParseVector3()
{
    Expect('[');

    float x = ParseFloat();
    Expect(',');

    float y = ParseFloat();
    Expect(',');

    float z = ParseFloat();

    Expect(']');

    return { x,y,z };
}

LibMath::Vector4 SerialParser::ParseVector4()
{
    Expect('[');

    float x = ParseFloat();
    Expect(',');

    float y = ParseFloat();
    Expect(',');

    float z = ParseFloat();
    Expect(',');

    float w = ParseFloat();

    Expect(']');

    return { x,y,z,w };
}

LibMath::Quaternion SerialParser::ParseQuaternion()
{
    Expect('[');

    float x = ParseFloat();
    Expect(',');

    float y = ParseFloat();
    Expect(',');

    float z = ParseFloat();
    Expect(',');

    float w = ParseFloat();

    Expect(']');

    return { x,y,z,w };
}

char SerialParser::PeekChar()
{
    SkipWhitespace();
    return m_text[m_index];
}