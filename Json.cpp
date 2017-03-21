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
#include <stdio.h>
#include <cctype>
#include <algorithm>

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
		removeTrailingComma() << "}" << DtoTextOutput::zero;
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

// -------------------------------------------------------- JsonDtoReader -------------------------------------------------------- //

// ** JsonDtoReader::JsonDtoReader
JsonDtoReader::JsonDtoReader(const byte* input, int32 length)
	: m_input(input, length)
{

}

// ** JsonDtoReader::next
DtoEvent JsonDtoReader::next()
{
	// A node stack is empty, this means that we have just started JSON parsing 
	if (m_stack.empty())
	{
		m_stack.push(&JsonDtoReader::parseStream);
	}

	// Pop a topmost parser from a stack
	EventParser parser = m_stack.top();
	m_stack.pop();

	// Now parse a next event from a stream.
	DtoEvent event = (this->*parser)();

	return event;
}

// ** JsonDtoReader::parseStream
DtoEvent JsonDtoReader::parseStream()
{
	const DtoTokenInput::Token& token = m_input.nextNonSpace();

	switch (token.type)
	{
	case DtoTokenInput::BraceOpen:
		m_input.nextNonSpace();
		m_stack.push(&JsonDtoReader::expectBraceStreamEnd);
		if (!m_input.check(DtoTokenInput::BraceClose))
		{
			m_stack.push(&JsonDtoReader::parseKeyValue);
		}
		return DtoStreamStart;

	case DtoTokenInput::BracketOpen:
		m_input.nextNonSpace();
		m_stack.push(&JsonDtoReader::expectBracketStreamEnd);
		m_index.push(0);
		if (!m_input.check(DtoTokenInput::BracketClose))
		{
			m_stack.push(&JsonDtoReader::parseItem);
		}
		return DtoStreamStart;

	default:
		m_input.emitUnexpectedToken();
	}

	return DtoError;
}

// ** JsonDtoReader::parseKeyValue
DtoEvent JsonDtoReader::parseKeyValue()
{
	const DtoTokenInput::Token& token = m_input.currentToken();
	DtoEvent event;
	DtoStringView key;

	m_stack.push(&JsonDtoReader::continueKeyValue);

	switch (token.type)
	{
	case DtoTokenInput::DoubleQuotedString:
		key = m_input.consumeString(true).string;

		if (!m_input.expect(DtoTokenInput::Colon, true))
		{
			return DtoError;
		}

		event = parsePrimitive(key);
		break;

	default:
		m_input.emitUnexpectedToken();
		return DtoError;
	}

	return event;
}

// ** JsonDtoReader::expectBraceStreamEnd
DtoEvent JsonDtoReader::expectBraceStreamEnd()
{
	if (m_input.expect(DtoTokenInput::BraceClose, true))
	{
		return DtoStreamEnd;
	}

	return DtoError;
}

// ** JsonDtoReader::expectBracketStreamEnd
DtoEvent JsonDtoReader::expectBracketStreamEnd()
{
	if (m_input.expect(DtoTokenInput::BracketClose, true))
	{
		m_index.pop();
		return DtoStreamEnd;
	}

	return DtoError;
}

// ** JsonDtoReader::expectKeyValueEnd
DtoEvent JsonDtoReader::expectKeyValueEnd()
{
	if (m_input.expect(DtoTokenInput::BraceClose, true))
	{
		return DtoKeyValueEnd;
	}

	return DtoError;
}

// ** JsonDtoReader::expectSequenceEnd
DtoEvent JsonDtoReader::expectSequenceEnd()
{
	if (m_input.expect(DtoTokenInput::BracketClose, true))
	{
		m_index.pop();
		return DtoSequenceEnd;
	}

	return DtoError;
}

// ** JsonDtoReader::continueSequence
DtoEvent JsonDtoReader::continueSequence()
{
	if (m_input.consume(DtoTokenInput::Comma, true))
	{
		return parseItem();
	}

	return next();
}

// ** JsonDtoReader::continueKeyValue
DtoEvent JsonDtoReader::continueKeyValue()
{
	if (m_input.consume(DtoTokenInput::Comma, true))
	{
		return parseKeyValue();
	}

	return next();
}

// ** JsonDtoReader::parseItem
DtoEvent JsonDtoReader::parseItem()
{
	DtoStringView key;
	key.value = m_text;
	key.length = sprintf_s(m_text, "%d", m_index.top()++);

	m_stack.push(&JsonDtoReader::continueSequence);

	DtoEvent event = parsePrimitive(key);

	return event;
}

// ** JsonDtoReader::parsePrimitive
DtoEvent JsonDtoReader::parsePrimitive(const DtoStringView& key)
{
	const DtoTokenInput::Token& next = m_input.currentToken();

	switch (next.type)
	{
	case DtoTokenInput::BracketOpen:
		m_input.nextNonSpace();
		m_stack.push(&JsonDtoReader::expectSequenceEnd);
		m_index.push(0);
		if (!m_input.check(DtoTokenInput::BracketClose))
		{
			m_stack.push(&JsonDtoReader::parseItem);
		}
		return DtoEvent(DtoSequenceStart, key);

	case DtoTokenInput::BraceOpen:
		m_input.nextNonSpace();
		m_stack.push(&JsonDtoReader::expectKeyValueEnd);
		if (!m_input.check(DtoTokenInput::BraceClose))
		{
			m_stack.push(&JsonDtoReader::parseKeyValue);
		}
		return DtoEvent(DtoKeyValueStart, key);

	case DtoTokenInput::DoubleQuotedString:
		return DtoEvent(key, m_input.consumeString(true));

	case DtoTokenInput::Number:
		return DtoEvent(key, m_input.consumeNumber(true));

	case DtoTokenInput::Minus:
		m_input.nextNonSpace();
		if (m_input.check(DtoTokenInput::Number))
		{
			return DtoEvent(key, m_input.consumeNumber(-1, true));
		}
		break;

	case DtoTokenInput::True:
	case DtoTokenInput::False:
		return DtoEvent(key, m_input.consumeBoolean(true));
	}

	return DtoError;
}

// ** JsonDtoReader::consumed
int32 JsonDtoReader::consumed() const
{
	return m_input.consumed();
}

DTO_END
