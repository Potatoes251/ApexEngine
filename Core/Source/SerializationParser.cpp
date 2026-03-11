#include "SerializationParser.h"

Apex::Serialization::SerialParser::SerialParser(const std::string& text)
{
	std::ifstream file(path);

	std::stringstream buffer;
	buffer << file.rdbuf();

	std::string text = buffer.str();
	size_t i = 0;
}
