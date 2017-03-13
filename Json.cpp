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
#include "Json.h"

#include <assert.h>

DTO_BEGIN

// -------------------------------------------------------- JsonDtoWriter -------------------------------------------------------- //

// ** JsonDtoWriter::JsonDtoWriter
JsonDtoWriter::JsonDtoWriter(byte* output, int32 capacity, cstring keyValueSeparator)
	: m_output(output, capacity)
	, m_keyValueSeparator(keyValueSeparator)
{

}

// ** JsonDtoWriter::consume
int32 JsonDtoWriter::consume(const DtoEvent& event)
{
	switch (event.type)
	{
	case DtoStreamStart:
		m_output << "{";
		m_stack.push(DtoKeyValue);
		break;

	case DtoStreamEnd:
		m_stack.pop();
		removeTrailingComma() << "}";
		break;

	case DtoSequenceStart:
		key(event.key) << "[";
		m_stack.push(DtoSequence);
		break;

	case DtoSequenceEnd:
		removeTrailingComma() << "]" << ",";
		m_stack.pop();
		break;

	case DtoKeyValueStart:
		key(event.key) << "{";
		m_stack.push(DtoKeyValue);
		break;

	case DtoKeyValueEnd:
		removeTrailingComma() << "}" << ",";
		m_stack.pop();
		break;

	case DtoEntry:
		key(event.key) << DtoTextOutput::quotedString << event.data << ",";
		break;

	default:
		assert(0);
	}
	return 0;
}

// ** JsonDtoWriter::indentation
DtoTextOutput& JsonDtoWriter::key(const DtoStringView& value)
{
	// Output keys only for key-value objects
	if (m_stack.top() == DtoKeyValue)
	{
		m_output << DtoTextOutput::quotedString << value << ":" << m_keyValueSeparator;
	}

	return m_output;
}

// ** JsonDtoWriter::removeTrailingComma
DtoTextOutput& JsonDtoWriter::removeTrailingComma()
{
	cstring text = m_output.text() - 1;

	if (*text == ',')
	{
		m_output.rewind(1);
	}

	return m_output;
}

// -------------------------------------------------------- JsonStyledDtoWriter -------------------------------------------------------- //

// ** JsonStyledDtoWriter::JsonStyledDtoWriter
JsonStyledDtoWriter::JsonStyledDtoWriter(byte* output, int32 capacity, cstring indent, cstring newLine)
	: JsonDtoWriter(output, capacity, " ")
	, m_indent(indent)
	, m_newLine(newLine)
{
}

// ** JsonStyledDtoWriter::JsonStyledDtoWriter
int32 JsonStyledDtoWriter::consume(const DtoEvent& event)
{
	int32 result = 0;

	switch (event.type)
	{
	case DtoStreamStart:
		result = JsonDtoWriter::consume(event);
		m_output << m_newLine;
		break;

	case DtoStreamEnd:
		result = JsonDtoWriter::consume(event);
		break;

	case DtoSequenceStart:
	case DtoKeyValueStart:
		indentation(m_stack.size());
		result = JsonDtoWriter::consume(event);
		m_output << m_newLine;
		break;

	case DtoKeyValueEnd:
	case DtoSequenceEnd:
		indentation(m_stack.size() - 1);
		result = JsonDtoWriter::consume(event);
		m_output << m_newLine;
		break;

	case DtoEntry:
		indentation(m_stack.size());
		result = JsonDtoWriter::consume(event);
		m_output << m_newLine;
		break;
	}

	return result;
}

// ** JsonStyledDtoWriter::indentation
void JsonStyledDtoWriter::indentation(int32 level)
{
	for (int32 i = 0; i < level; i++)
	{
		m_output << m_indent;
	}
}

DTO_END
