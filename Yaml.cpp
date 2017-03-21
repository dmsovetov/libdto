/**************************************************************************

The MIT License (MIT)

Copyright (c) 2017 Dmitry Sovetov

https://github.com/dmsovetov

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

**************************************************************************/

#include "Dto.h"
#include "Yaml.h"

#include <assert.h>
#include <cctype>

DTO_BEGIN

// -------------------------------------------------------- YamlDtoWriter -------------------------------------------------------- //

// ** YamlDtoWriter::YamlDtoWriter
YamlDtoWriter::YamlDtoWriter(byte* output, int32 capacity, cstring newLine)
	: m_output(output, capacity)
	, m_newLine(newLine)
{

}

// ** YamlDtoWriter::JsonStyledDtoWriter
int32 YamlDtoWriter::consume(const DtoEvent& event)
{
	int32 result = 0;

	switch (event.type)
	{
	case DtoStreamStart:
		m_stack.push(DtoKeyValue);
		break;
	case DtoStreamEnd:
		m_stack.pop();
		m_output << DtoTextOutput::zero;
		break;

	case DtoSequenceStart:
		key(event.key) << m_newLine;
		m_stack.push(DtoSequence);
		break;

	case DtoKeyValueStart:
		key(event.key) << m_newLine;
		m_stack.push(DtoKeyValue);
		break;

	case DtoKeyValueEnd:
	case DtoSequenceEnd:
		m_stack.pop();
		break;

	case DtoEntry:
		key(event.key) << event.data << m_newLine;
		break;

	default:
		assert(0);
	}

	return result;
}

// ** YamlDtoWriter::indentation
void YamlDtoWriter::indentation(int32 level)
{
	for (int32 i = 0; i < level; i++)
	{
		m_output << "  ";
	}
}

// ** YamlDtoWriter::indentation
DtoTextOutput& YamlDtoWriter::key(const DtoStringView& value)
{
	// Output an indentation level
	indentation(m_stack.size() - 1);

	// Skip a key for sequences, instead output a '-' symbol
	if (m_stack.top().type == DtoSequence)
	{
		m_output << "- ";
	}
	else
	{
		m_output << value << ": ";
	}

	return m_output;
}

// -------------------------------------------------------- YamlDtoReader -------------------------------------------------------- //

// ** YamlDtoReader::YamlDtoReader
YamlDtoReader::YamlDtoReader(const byte* input, int32 length)
	: m_input(input, length)
{

}

// ** YamlDtoReader::consumed
int32 YamlDtoReader::consumed() const
{
	return m_input.consumed();
}

// ** YamlDtoReader::next
DtoEvent YamlDtoReader::next()
{
	// A node stack is empty, this means that we have just started JSON parsing 
	if (m_stack.empty())
	{
		m_stack.push(&YamlDtoReader::parseStream);
	}

	// Pop a topmost parser from a stack
	EventParser parser = m_stack.top();
	m_stack.pop();

	// Now parse a next event from a stream.
	DtoEvent event = (this->*parser)();

	return event;
}

// ** YamlDtoReader::parseStream
DtoEvent YamlDtoReader::parseStream()
{
	// Consume an indentation level
	int32 indentation = consumeIndentation();

	if (m_stack.empty())
	{
		m_indentation.push(indentation);
	}

	if (indentation > m_indentation.top())
	{
		return DtoKeyValueStart;
	}

	const DtoTokenInput::Token& token = m_input.next();

	switch (token.type)
	{
	case DtoTokenInput::Identifier:
		assert(0);
		return DtoStreamStart;

	case DtoTokenInput::Minus:
		assert(0);
		return DtoStreamStart;

	default:
		m_input.emitUnexpectedToken();
	}

	return DtoError;
}

// ** YamlDtoReader::consumeIndentation
int32 YamlDtoReader::consumeIndentation()
{
	int32 count = 0;

	while (m_input.consume(DtoTokenInput::Space))
	{
		count++;
	}

	return count;
}

DTO_END
