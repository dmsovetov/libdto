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
#include "ByteBuffer.h"

#include <string.h>
#include <assert.h>
#include <stdio.h>

#ifdef _WINDOWS
	#define snprintf _snprintf_s
#endif	//	#ifdef _WINDOWS

DTO_BEGIN

// ------------------------------------------------------- DtoByteArrayOutput ------------------------------------------------------- //

// ** DtoByteArrayOutput::DtoByteArrayOutput
DtoByteArrayOutput::DtoByteArrayOutput(byte* output, int32 capacity)
	: m_output(output)
	, m_ptr(output)
	, m_capacity(capacity)
	, m_size(0)
{
	assert(m_capacity > 0);
}

// ** DtoByteArrayOutput::DtoByteArrayOutput
DtoByteArrayOutput::DtoByteArrayOutput(DtoByteArrayOutput& parent)
	: m_output(parent.ptr())
	, m_ptr(parent.ptr())
	, m_capacity(parent.available())
	, m_size(0)
{
	assert(m_capacity > 0);
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (bool value)
{
	assert(sizeof(value) == 1);
	*reinterpret_cast<bool*>(advance(1)) = value;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (DtoValueType value)
{
	return (*this << static_cast<byte>(value));
}

// **  DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (byte value)
{
	assert(sizeof(value) == 1);
	*reinterpret_cast<byte*>(advance(1)) = value;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (int32 value)
{
	assert(sizeof(value) == 4);
	*reinterpret_cast<int32*>(advance(4)) = value;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (int64 value)
{
	assert(sizeof(value) == 8);
	*reinterpret_cast<int64*>(advance(8)) = value;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (uint64 value)
{
	assert(sizeof(value) == 8);
	*reinterpret_cast<uint64*>(advance(8)) = value;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (double value)
{
	assert(sizeof(value) == 8);
	*reinterpret_cast<double*>(advance(8)) = value;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (cstring value)
{
	int32 length = static_cast<int32>(strlen(value));
	memcpy(advance(length), value, length);
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (const DtoStringView& value)
{
	memcpy(advance(value.length), value.value, value.length);
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (const size& size)
{
	assert(m_size.value == 0);
	assert(size.value > 0);
	m_size = size;
	return *this;
}

// ** DtoByteArrayOutput::operator <<
DtoByteArrayOutput& DtoByteArrayOutput::operator << (const byte* bytes)
{
	assert(m_size.value > 0);
	memcpy(advance(m_size.value), bytes, m_size.value);
	m_size.value = 0;
	return *this;
}

// ** DtoByteArrayOutput::buffer
const byte* DtoByteArrayOutput::buffer() const
{
	return m_output;
}

// ** DtoByteArrayOutput::buffer
byte* DtoByteArrayOutput::buffer()
{
	return m_output;
}

// ** DtoByteArrayOutput::ptr
const byte* DtoByteArrayOutput::ptr() const
{
	return m_ptr;
}

// ** DtoByteArrayOutput::ptr
byte* DtoByteArrayOutput::ptr()
{
	return m_ptr;
}

// ** DtoByteArrayOutput::advance
byte* DtoByteArrayOutput::advance(int32 count)
{
	assert(count <= available());

	byte* result = m_ptr;
	m_ptr += count;
	return result;
}

// ** DtoByteArrayOutput::length
int32 DtoByteArrayOutput::length() const
{
	return m_ptr - m_output;
}

// ** DtoByteArrayOutput::capacity
int32 DtoByteArrayOutput::capacity() const
{
	return m_capacity;
}

// ** DtoByteArrayOutput::available
int32 DtoByteArrayOutput::available() const
{
	return capacity() - length();
}

// --------------------------------------------------------- DtoTextOutput --------------------------------------------------------- //

// ** DtoTextOutput::DtoTextOutput
DtoTextOutput::DtoTextOutput(byte* output, int32 capacity)
	: DtoByteArrayOutput(output, capacity)
	, m_isQuotedString(false)
{
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (bool value)
{
	if (value)
	{
		memcpy(advance(4), "true", 4);
	}
	else
	{
		memcpy(advance(5), "false", 5);
	}

	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (byte value)
{
	char buffer[5];
	int32 length = snprintf(buffer, sizeof(buffer), "%d", value);
	memcpy(advance(length), buffer, length);
	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (int32 value)
{
	char buffer[12];
	int32 length = snprintf(buffer, sizeof(buffer), "%d", value);
	memcpy(advance(length), buffer, length);
	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (int64 value)
{
	char buffer[21];
	int32 length = snprintf(buffer, sizeof(buffer), "%lld", value);
	memcpy(advance(length), buffer, length);
	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (uint64 value)
{
	char buffer[21];
	int32 length = snprintf(buffer, sizeof(buffer), "%llu", value);
	memcpy(advance(length), buffer, length);
	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (double value)
{
	char buffer[16];
	int32 length = snprintf(buffer, sizeof(buffer), "%g", value);
	memcpy(advance(length), buffer, length);
	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (cstring value)
{
	int32 length = static_cast<int32>(strlen(value));

	if (m_isQuotedString)
	{
		memcpy(advance(1), "\"", 1);
	}

	memcpy(advance(length), value, length);

	if (m_isQuotedString)
	{
		memcpy(advance(1), "\"", 1);
		m_isQuotedString = false;
	}

	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (const DtoStringView& value)
{
	if (m_isQuotedString)
	{
		memcpy(advance(1), "\"", 1);
	}

	memcpy(advance(value.length), value.value, value.length);

	if (m_isQuotedString)
	{
		memcpy(advance(1), "\"", 1);
		m_isQuotedString = false;
	}

	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (const DtoValue& value)
{
	switch (value.type)
	{
	case DtoBool:
		operator << (value.boolean);
		break;

	case DtoString:
		operator << (value.string.value);
		break;

	case DtoInt32:
		operator << (value.int32);
		break;

	case DtoTimestamp:
		operator << (value.uint64);
		break;

	case DtoDate:
	case DtoInt64:
		operator << (value.int64);
		break;

	case DtoDouble:
		operator << (value.number);
		break;

	case DtoBinary:
		operator << ("<binary>");
		break;

	default:
		assert(0);
	}

	m_isQuotedString = false;

	return *this;
}

// ** DtoTextOutput::operator <<
DtoTextOutput& DtoTextOutput::operator << (marker value)
{
	switch (value)
	{
	case quotedString:
		m_isQuotedString = true;
		break;
		
	default:
		assert(0);
	}
	return *this;
}

// ** DtoTextOutput::rewind
void DtoTextOutput::rewind(int32 count)
{
	m_ptr -= count;
}

// ** DtoTextOutput::text
cstring DtoTextOutput::text() const
{
	return reinterpret_cast<cstring>(ptr());
}

// ------------------------------------------------------- DtoByteArrayInput ------------------------------------------------------- //

// ** DtoByteBufferInput::DtoByteBufferInput
DtoByteBufferInput::DtoByteBufferInput(const byte* input, int32 capacity)
	: m_input(input)
	, m_ptr(input)
	, m_capacity(capacity)
{

}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (bool& value)
{
	assert(sizeof(bool) == 1);
	value = *reinterpret_cast<const bool*>(advance(1));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (DtoValueType& value)
{
	value = static_cast<DtoValueType>(*reinterpret_cast<const byte*>(advance(1)));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (byte& value)
{
	assert(sizeof(byte) == 1);
	value = *reinterpret_cast<const byte*>(advance(1));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (int32& value)
{
	assert(sizeof(int32) == 4);
	value = *reinterpret_cast<const int32*>(advance(4));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (int64& value)
{
	assert(sizeof(int64) == 8);
	value = *reinterpret_cast<const int64*>(advance(8));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (uint64& value)
{
	assert(sizeof(uint64) == 8);
	value = *reinterpret_cast<const uint64*>(advance(8));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (double& value)
{
	assert(sizeof(double) == 8);
	value = *reinterpret_cast<const double*>(advance(8));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (cstring& value)
{
	value = reinterpret_cast<cstring>(m_ptr);
	int32 length = static_cast<int32>(strlen(value));
	advance(length);
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (DtoStringView& value)
{
	*this >> value.value;
	value.length = static_cast<int32>(strlen(value.value));
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (const byte*& value)
{
	value = m_ptr;
	return *this;
}

// ** DtoByteBufferInput::operator >>
DtoByteBufferInput& DtoByteBufferInput::operator >> (const skip& count)
{
	assert(count.value > 0);
	advance(count.value);
	return *this;
}

// ** DtoByteBufferInput::advance
const byte* DtoByteBufferInput::advance(int32 count)
{
	assert(count <= available());
	assert(count >= 0);

	const byte* result = m_ptr;
	m_ptr += count;
	return result;
}

// ** DtoByteBufferInput::capacity
int32 DtoByteBufferInput::capacity() const
{
	return m_capacity;
}

// ** DtoByteBufferInput::consumed
int32 DtoByteBufferInput::consumed() const
{
	return m_ptr - m_input;
}

// ** DtoByteBufferInput::available
int32 DtoByteBufferInput::available() const
{
	return capacity() - consumed();
}

// ** DtoByteBufferInput::ptr
const byte* DtoByteBufferInput::ptr() const
{
	return m_ptr;
}

// ** DtoByteBufferInput::setPtr
void DtoByteBufferInput::setPtr(const byte* value)
{
	assert(value >= m_input && value <= (m_input + m_capacity));
	m_ptr = value;
}

DTO_END