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
		m_stack.push(Node::Root);
	}

	// Read next token from an input stream
	DtoStringView text;
	Token token = readToken(text);

	// Process next token according to a topmost node type.
	switch (topmost().type)
	{
	case Node::Root:
		return eventRoot(topmost(), token, text);

	case Node::KeyValue:
		return eventKeyValue(topmost(), token, text);

	case Node::Sequence:
		return eventSequence(topmost(), token, text);
	}

	return DtoError;
}

// ** JsonDtoReader::eventRoot
DtoEvent JsonDtoReader::eventRoot(Node& node, Token token, const DtoStringView& text)
{
	DtoEvent event = DtoError;

	switch (token)
	{
	case TokenKeyValueStart:
		return DtoStreamStart;

	case TokenKeyValueEnd:
		return DtoStreamEnd;

	case TokenString:
		event = eventEntry(text);
		node.children++;
		break;

	default:
		return DtoError;
	}

	// Process a trailing comma
	node.closed = parseToken(TokenComma) == false;

	return event;
}

// ** JsonDtoReader::eventKeyValue
DtoEvent JsonDtoReader::eventKeyValue(Node& node, Token token, const DtoStringView& text)
{
	DtoEvent event = DtoError;

	switch (token)
	{
	case TokenString:
		event = eventEntry(text);
		node.children++;
		break;

	case TokenKeyValueEnd:
		m_stack.pop();
		event = DtoKeyValueEnd;
		break;

	default:
		return DtoError;
	}

	// Process a trailing comma
	node.closed = parseToken(TokenComma) == false;

	return event;
}

// ** JsonDtoReader::eventSequence
DtoEvent JsonDtoReader::eventSequence(Node& node, Token token, const DtoStringView& text)
{
	DtoEvent event = DtoError;
	DtoStringView key;
	key.value = m_text;
	key.length = sprintf_s(m_text, "%d", node.children);

	switch (token)
	{
	case TokenString:
		event = DtoEvent(key, stringFromToken(text));
		node.children++;
		break;

	case TokenNumber:
		event = DtoEvent(key, numberFromToken(text));
		node.children++;
		break;

	case TokenSequenceStart:
		m_stack.push(Node::Sequence);
		node.children++;
		return DtoEvent(DtoSequenceStart, key);

	case TokenKeyValueStart:
		m_stack.push(Node::KeyValue);
		node.children++;
		return DtoEvent(DtoKeyValueStart, key);

	case TokenKeyValueEnd:
		m_stack.pop();
		event = DtoKeyValueEnd;
		break;

	case TokenSequenceEnd:
		m_stack.pop();
		event = DtoSequenceEnd;
		break;

	default:
		return DtoError;
	}

	// Process trailing comma
	node.closed = parseToken(TokenComma) == false;

	return event;
}

// ** JsonDtoReader::eventEntry
DtoEvent JsonDtoReader::eventEntry(const DtoStringView& key)
{
	if (!expectToken(TokenColon))
	{
		return DtoError;
	}

	DtoStringView text;
	DtoValue value;
	Token token = readToken(text);

	switch (token)
	{
	case TokenSequenceStart:
		m_stack.push(Node::Sequence);
		return DtoEvent(DtoSequenceStart, key);

	case TokenKeyValueStart:
		m_stack.push(Node::KeyValue);
		return DtoEvent(DtoKeyValueStart, key);

	case TokenString:
		return DtoEvent(key, stringFromToken(text));

	case TokenNumber:
		return DtoEvent(key, numberFromToken(text));

	case TokenTrue:
		value.type = DtoBool;
		value.boolean = true;
		return DtoEvent(key, value);

	case TokenFalse:
		value.type = DtoBool;
		value.boolean = false;
		return DtoEvent(key, value);
	}

	return DtoError;
}

// ** JsonDtoReader::stringFromToken
DtoValue JsonDtoReader::stringFromToken(const DtoStringView& value)
{
	DtoValue result;
	result.type = DtoString;
	result.string = value;
	return result;
}

// ** JsonDtoReader::numberFromToken
DtoValue JsonDtoReader::numberFromToken(const DtoStringView& value)
{
	char buffer[24];
	strncpy(buffer, value.value, std::min<int32>(sizeof(buffer), value.length));

	DtoValue result;
	result.type = DtoDouble;
	result.number = atof(buffer);
	return result;
}

// ** JsonDtoReader::consumed
int32 JsonDtoReader::consumed() const
{
	return m_input.consumed();
}

// ** JsonDtoReader::topmost
JsonDtoReader::Node& JsonDtoReader::topmost()
{
	assert(m_stack.size());
	return m_stack.top();
}

// ** JsonDtoReader::readToken
JsonDtoReader::Token JsonDtoReader::readToken(DtoStringView& text)
{
	text.value = reinterpret_cast<cstring>(m_input.advance(1));
	text.length = 1;

	char symbol = *text.value;

	switch (symbol)
	{
		case 0:
			return TokenEOF;

		case '{':
			return TokenKeyValueStart;

		case '}':
			return TokenKeyValueEnd;

		case '[':
			return TokenSequenceStart;

		case ']':
			return TokenSequenceEnd;

		case ':':
			return TokenColon;

		case ',':
			return TokenComma;

		case 't':
			if (strncmp(reinterpret_cast<cstring>(m_input.ptr()), "rue", 3) == 0)
			{
				m_input.advance(3);
				return TokenTrue;
			}
			break;

		case 'f':
			if (strncmp(reinterpret_cast<cstring>(m_input.ptr()), "alse", 4) == 0)
			{
				m_input.advance(4);
				return TokenFalse;
			}
			break;

		case '"':
			consumeStringToken(text);
			text.length = reinterpret_cast<cstring>(m_input.ptr()) - text.value;
			text.value++;
			text.length -= 2;
			return TokenString;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			consumeNumberToken(text);
			text.length = reinterpret_cast<cstring>(m_input.ptr()) - text.value;
			return TokenNumber;
	}

	return TokenNonTerminal;
}

// ** JsonDtoReader::expectToken
bool JsonDtoReader::expectToken(Token token)
{
	DtoStringView text;
	
	if (readToken(text) == token)
	{
		return true;
	}

	return false;
}

// ** JsonDtoReader::parseToken
bool JsonDtoReader::parseToken(Token token)
{
	const byte* ptr = m_input.ptr();
	DtoStringView text;
	Token next = readToken(text);

	if (next == token)
	{
		return true;
	}

	m_input.setPtr(ptr);
	return false;
}

// ** JsonDtoReader::consumeStringToken
void JsonDtoReader::consumeStringToken(DtoStringView& text)
{
	while (*m_input.advance(1) != '"')
	{
	}
}

// ** JsonDtoReader::consumeNumberToken
void JsonDtoReader::consumeNumberToken(DtoStringView& text)
{
	while (isdigit(m_input.ptr()[0]))
	{
		m_input.advance(1);
	}

	if (m_input.ptr()[0] == '.')
	{
		m_input.advance(1);

		while (isdigit(m_input.ptr()[0]))
		{
			m_input.advance(1);
		}
	}
}

DTO_END
