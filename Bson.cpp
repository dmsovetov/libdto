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
#include "Bson.h"

#include <assert.h>

DTO_BEGIN

BinaryDtoWriter::BinaryDtoWriter(byte* output, int32 capacity)
	: m_output(output, capacity)
{

}

// ** BinaryDtoWriter::consume
int32 BinaryDtoWriter::consume(const DtoEvent& event)
{
	// Save current length
	int32 length = m_output.length();

	// Process an event
	switch (event.type)
	{
	case DtoStreamStart:
		start(event.key, DtoEnd);
		break;

	case DtoSequenceStart:
		start(event.key, DtoSequence);
		break;

	case DtoKeyValueStart:
		start(event.key, DtoKeyValue);
		break;

	case DtoEntry:
		encode(m_output, event.key, event.data);
		break;

	case DtoKeyValueEnd:
	case DtoSequenceEnd:
	case DtoStreamEnd:
		finish();
		break;

	default:
		assert(0);
	}

	// Calculate a total number of bytes written to an output.
	int32 count = m_output.length() - length;

	return count;
}

// ** BinaryDtoWriter::start
void BinaryDtoWriter::start(const DtoStringView& key, DtoValueType type)
{
	if (type != DtoEnd)
	{
		m_output << type << key << DtoEnd;
	}
	m_stack.push(m_output.ptr());
	m_output << static_cast<int32>(0);
}

// ** BinaryDtoWriter::finish
void BinaryDtoWriter::finish()
{
	assert(!m_stack.empty());
	m_output << DtoEnd;
	*reinterpret_cast<int32*>(m_stack.top()) = m_output.ptr() - m_stack.top();
	m_stack.pop();
}

// ** BinaryDtoWriter::encode
int32 BinaryDtoWriter::encode(DtoByteArrayOutput& output, const DtoStringView& key, const DtoValue& value)
{
	// Save current stream length
	int32 length = output.length();

	// First, write the value type and it's key
	output << value.type << key << DtoEnd;

	// Then write an actual value
	switch (value.type)
	{
	case DtoString:
		output << value.string.length + 1 << value.string.value << DtoEnd;
		break;

	case DtoBool:
		output << value.boolean;
		break;

	case DtoInt32:
		output << value.int32;
		break;

	case DtoInt64:
		output << value.int64;
		break;

	case DtoTimestamp:
		output << value.uint64;
		break;

	case DtoDouble:
		output << value.number;
		break;

	case DtoBinary:
		output << value.binary.length << value.binary.subtype << DtoByteArrayOutput::size(value.binary.length) << value.binary.data;
		break;

	default:
		assert(0);
	}

	// Calculate a total number of bytes written to an output
	int32 count = output.length() - length;

	return count;
}

//! Constructs a DTO value from a C-string.
DtoValue constructValue(cstring value)
{
	DtoValue v;
	v.type = DtoString;
	v.string.value = value;
	v.string.length = strlen(value);
	return v;
}

//! Constructs a DTO value from a binary blob.
DtoValue constructValue(const DtoBinaryBlob& value)
{
	DtoValue v;
	v.type = DtoBinary;
	v.binary = value;
	return v;
}

//! Constructs a DTO value from a boolean value.
DtoValue constructValue(bool value)
{
	DtoValue v;
	v.type = DtoBool;
	v.boolean = value;
	return v;
}

//! Constructs a DTO value from a 64-bit decimal floating point value.
DtoValue constructValue(double value)
{
	DtoValue v;
	v.type = DtoDouble;
	v.number = value;
	return v;
}

//! Constructs a DTO value from a 32-bit signed integer.
DtoValue constructValue(int32 value)
{
	DtoValue v;
	v.type = DtoInt32;
	v.int32 = value;
	return v;
}

//! Constructs a DTO value from a 64-bit signed integer.
DtoValue constructValue(int64 value)
{
	DtoValue v;
	v.type = DtoInt64;
	v.int64 = value;
	return v;
}

//! Constructs a DTO value from a 64-bit unsigned integer.
DtoValue constructValue(uint64 value)
{
	DtoValue v;
	v.type = DtoTimestamp;
	v.uint64 = value;
	return v;
}

// ----------------------------------------------------------- DtoEncoder ------------------------------------------------------------ //

// ** DtoEncoder::DtoEncoder
DtoEncoder::DtoEncoder(byte* output, int32 capacity)
	: m_output(output, capacity)
	, m_key(0)
	//, m_allocated(0)
{
	// Save a document root pointer
	m_stack.push(Nested(m_output.ptr()));

	// Initialize document length
	m_output << static_cast<int32>(0);
}

// DtoEncoder::DtoEncoder
//DtoEncoder::DtoEncoder(int32 capacity)
//	: m_output(new byte[capacity], capacity)
//	, m_key(0)
//{
//	// Save a document root pointer
//	m_stack.push(Nested(m_output.ptr()));
//
//	// Initialize document length
//	m_output << static_cast<int32>(0);
//
//	// Allocate a temporary buffer
//	m_allocated = m_output.buffer();
//}

// ** DtoEncoder::~DtoEncoder
DtoEncoder::~DtoEncoder()
{
	assert(m_stack.empty());
//	delete[]m_allocated;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (bool value)
{
	assert(!complete());
	BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (double value)
{
	assert(!complete());
	BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (int32 value)
{
	assert(!complete());
	BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (int64 value)
{
	assert(!complete());
	BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (uint64 value)
{
	assert(!complete());
	BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (cstring value)
{
	assert(!complete());

	if (hasKey())
	{
		BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	}
	else
	{
		m_key = value;
	}

	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (const DtoBinaryBlob& value)
{
	assert(!complete());
	BinaryDtoWriter::encode(m_output, entryKey(), constructValue(value));
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (const DtoEncoder& value)
{
	assert(value.complete());
	m_output << DtoKeyValue << entryKey() << DtoEnd << DtoByteArrayOutput::size(value.length()) << value.data();
	return *this;
}

// ** DtoEncoder::operator <<
DtoEncoder& DtoEncoder::operator << (marker value)
{
	assert(!complete());

	switch (value)
	{
	case keyValue:
		m_output << DtoKeyValue << entryKey() << DtoEnd;
		m_stack.push(Nested(m_output.ptr()));
		m_output << static_cast<int32>(0);
		break;

	case sequence:
		m_output << DtoSequence << entryKey() << DtoEnd;
		m_stack.push(Nested(m_output.ptr(), 0));
		m_output << static_cast<int32>(0);
		break;

	case end:
		m_output << DtoEnd;
		*reinterpret_cast<int32*>(m_stack.top().ptr) = m_output.ptr() - m_stack.top().ptr;
		m_stack.pop();
		break;
	}

	return *this;
}

// ** DtoEncoder::entryKey
DtoStringView DtoEncoder::entryKey()
{
	Nested& topmost = m_stack.top();
	cstring key = 0;

	if (topmost.index >= 0)
	{
		key = itoa(topmost.index++, m_index, 10);
	}
	else
	{
		key = m_key;
		m_key = 0;
	}

	assert(key);

	DtoStringView result = { key, static_cast<int32>(strlen(key)) };
	return result;
}

// ** DtoEncoder::hasKey
bool DtoEncoder::hasKey() const
{
	assert(!complete());
	const Nested& topmost = m_stack.top();

	if (topmost.index >= 0)
	{
		return true;
	}

	return m_key != 0;
}

// ** DtoEncoder::length
int32 DtoEncoder::length() const
{
	return m_output.length();
}

// ** DtoEncoder::data
const byte* DtoEncoder::data() const
{
	return m_output.buffer();
}

// ** DtoEncoder::complete
bool DtoEncoder::complete() const
{
	return m_stack.empty();
}

// --------------------------------------------------------- BinaryDtoReader --------------------------------------------------------- //

// ** BinaryDtoReader::BinaryDtoReader
BinaryDtoReader::BinaryDtoReader(const byte* input, int32 length)
	: m_input(input, length)
{

}

// ** BinaryDtoReader::consumed
int32 BinaryDtoReader::consumed() const
{
	return m_input.consumed();
}

// ** BinaryDtoReader::decode
int32 BinaryDtoReader::decode(DtoByteBufferInput& input, DtoStringView& key, DtoValue& value)
{
	// Save current amount of consumed bytes
	int32 count = input.consumed();

	// Read tag
	input >> value.type;

	// If this is an end tag - just return
	if (value.type == DtoEnd)
	{
		return 1; // Just a single byte was consumed.
	}

	// This is an entry, so read it's key
	input >> key.value >> DtoByteBufferInput::skip(1);
	key.length = static_cast<int32>(strlen(key.value));

	// Read value data according to it's type
	switch (value.type)
	{
	case DtoString:
		input >> value.string.length >> value.string.value >> DtoByteBufferInput::skip(1);
		value.string.length--; // Decrease the string length as it counts the zero terminator.
		break;

	case DtoBinary:
		input >> value.binary.length >> value.binary.subtype >> value.binary.data;
		input >> DtoByteBufferInput::skip(value.binary.length);
		break;

	case DtoBool:
		input >> value.boolean;
		break;

	case DtoInt32:
		input >> value.int32;
		break;

	case DtoDate:
	case DtoInt64:
		input >> value.int64;
		break;

	case DtoTimestamp:
		input >> value.uint64;
		break;

	case DtoDouble:
		input >> value.number;
		break;

	case DtoSequence:
	case DtoKeyValue:
		input >> value.binary.length >> value.binary.data;
		break;

	default:
		assert(0);
	}

	// Calculate a total number of bytes consumed from an input stream
	int32 bytesConsumed = input.consumed() - count;

	return bytesConsumed;
}

// ** BinaryDtoReader::next
DtoEvent BinaryDtoReader::next()
{
	DtoEvent event;

	if (m_stack.empty())
	{
		int32 length;
		m_input >> length;
		event.type = push(DtoKeyValue, length);
	}
	else
	{
		// Decode next entry from an input stream.
		decode(m_input, event.key, event.data);

		// Set an event type as DtoEntry by default
		event.type = DtoEntry;

		// Process an entry value type
		switch (event.data.type)
		{
		case DtoEnd:
			event.type = pop();
			break;

		case DtoSequence:
		case DtoKeyValue:
			event.type = push(event.data.type, event.data.binary.length);
			break;
		}
	}

	return event;
}

// ** BinaryDtoReader::push
DtoEventType BinaryDtoReader::push(DtoValueType type, int32 length)
{
	m_stack.push(Nested(length, type));

	if (m_stack.size() == 1)
	{
		return DtoStreamStart;
	}

	return type == DtoSequence ? DtoSequenceStart : DtoKeyValueStart;
}

// ** BinaryDtoReader::pop
DtoEventType BinaryDtoReader::pop()
{
	assert(m_stack.size());
	DtoValueType type = static_cast<DtoValueType>(m_stack.top().type);
	m_stack.pop();

	if (m_stack.empty())
	{
		return DtoStreamEnd;
	}

	return type == DtoSequence ? DtoSequenceEnd : DtoKeyValueEnd;
}

DTO_END