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
	: JsonDtoReader(input, length)
{

}

// ** YamlDtoReader::next
DtoEvent YamlDtoReader::next()
{
	// A node stack is empty, this means that we have just started JSON parsing 
	if (m_stack.empty())
	{
		m_stack.push(Node::Root);
		return DtoStreamStart;
	}

	// Read next token from an input stream
	byte token = readToken();

	switch (topmost().type)
	{
	case Node::Root:
		return eventRoot(topmost());

	case Node::KeyValue:
		return eventKeyValue(topmost());

	case Node::Sequence:
		return eventSequence(topmost());
	}

	return DtoError;
}

// ** YamlDtoReader::eventRoot
DtoEvent YamlDtoReader::eventRoot(Node& node)
{
	DtoEvent event = DtoError;

	switch (currentToken())
	{
	case TokenEOF:
		return DtoStreamEnd;

	case TokenIdentifier:
	case TokenString:
		return eventEntry(currentText());

	default:
		return DtoError;
	}

	return event;
}

// ** YamlDtoReader::eventEntry
DtoEvent YamlDtoReader::eventEntry(const DtoStringView& key)
{
	if (!expectToken(TokenColon))
	{
		return DtoError;
	}

	DtoValue value;
	byte token = readToken();

	switch (currentToken())
	{
	case TokenSequenceStart:
		m_stack.push(Node::Sequence);
		return DtoEvent(DtoSequenceStart, key);

	case TokenKeyValueStart:
		m_stack.push(Node::KeyValue);
		return DtoEvent(DtoKeyValueStart, key);

	case TokenString:
		return DtoEvent(key, stringFromToken(currentText()));

	case TokenNumber:
		return DtoEvent(key, numberFromToken(currentText()));

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

// ** YamlDtoReader::eventKeyValue
DtoEvent YamlDtoReader::eventKeyValue(Node& node)
{
	DtoEvent event = DtoError;

	switch (currentToken())
	{
	case TokenString:
		event = eventEntry(currentText());
		node.children++;
		break;

	case TokenKeyValueEnd:
		m_stack.pop();
		event = DtoKeyValueEnd;
		break;

	default:
		return DtoError;
	}

	return event;
}

// ** YamlDtoReader::eventSequence
DtoEvent YamlDtoReader::eventSequence(Node& node)
{
	DtoEvent event = DtoError;
	DtoStringView key;
	key.value = m_text;
	key.length = sprintf_s(m_text, "%d", node.children);

	switch (currentToken())
	{
	case TokenString:
		event = DtoEvent(key, stringFromToken(currentText()));
		node.children++;
		break;

	case TokenNumber:
		event = DtoEvent(key, numberFromToken(currentText()));
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

	return event;
}

// ** YamlDtoReader::readToken
byte YamlDtoReader::readToken()
{
	// First try to read a JSON token
	byte token = JsonDtoReader::readToken();

	if (token != TokenNonTerminal)
	{
		return token;
	}

	DtoStringView& text = m_token.text;

	// Not a JSON token, so try to read a Yaml token
	if (consumeSymbol('-'))
	{
		return TokenEntry;
	}

	if (consumeIdentifierToken(text))
	{
		text.length = reinterpret_cast<cstring>(m_input.ptr()) - text.value;
		return setToken(TokenIdentifier, text);
	}

	return DtoError;
}

// ** YamlDtoReader::consumeIdentifierToken
bool YamlDtoReader::consumeIdentifierToken(DtoStringView& text)
{
	char c = currentSymbol();

	if (!isalpha(c))
	{
		return false;
	}

	do
	{
		m_input.advance(1);
		c = currentSymbol();
	} while (isalnum(c) || c == ' ' || c == '\t');

	return true;
}

// ** YamlDtoReader::consumeIndentation
int32 YamlDtoReader::consumeIndentation()
{
	assert(0);
	return 0;
}

// ** YamlDtoReader::consumeIndentation
void YamlDtoReader::consumeWhiteSpaces()
{
	
}

DTO_END
