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
#include <cassert>
#include <memory.h>

DTO_BEGIN

// ------------------------------------------------------ Dto ------------------------------------------------------ //

// ** Dto::Dto
Dto::Dto()
	: m_data(0)
	, m_capacity(0)
{
}

// ** Dto::Dto
Dto::Dto(const byte* data, int32 capacity)
	: m_data(data)
	, m_capacity(capacity)
{
	assert(capacity >= 0);
	assert(data != 0);
}

// ** Dto::operator bool
Dto::operator bool() const
{
	return m_capacity > 0 && m_data != 0;
}

// ** Dto::capacity
int32 Dto::capacity() const
{
	return m_capacity;
}

// ** Dto::length
int32 Dto::length() const
{
	if (m_data)
	{
		return *reinterpret_cast<const int32*>(m_data);
	}

	return 0;
}

// ** Dto::data
const byte* Dto::data() const
{
	return m_data;
}

// ** Dto::iter
DtoIter Dto::iter() const
{
	return DtoIter(m_data + sizeof(int32), m_capacity);
}

// ** Dto::find
DtoIter Dto::find(cstring key) const
{
	assert(key);
	DtoStringView view;
	view.value = key;
	view.length = static_cast<int32>(strlen(key));
	
	return find(view);
}

// ** Dto::find
DtoIter Dto::find(const DtoStringView& key) const
{
	assert(key);

	DtoIter i = iter();

	while (i.next())
	{
		if (i.key() == key)
		{
			return i;
		}
	}

	return i;
}

// ** Dto::findDescendant
DtoIter Dto::findDescendant(cstring key) const
{
	DtoIter i = iter();
	Dto dto = *this;
	
	while (*key)
	{
		if (*key == '.')
		{
			key++;
			continue;
		}

		DtoStringView nextKey;
		nextKey.value = key;

		while (*key && *key != '.')
		{
			key++;
		}
		nextKey.length = key - nextKey.value;

		i = dto.find(nextKey);

		if (i && *key == 0)
		{
			return i;
		}

		if (!i || (i != DtoSequence && i != DtoKeyValue))
		{
			return DtoIter(NULL, 0);
		}

		dto = i.toDto();
	}

	return i;
}

// ** Dto::entryCount
int32 Dto::entryCount() const
{
	DtoIter i = iter();
	int32 count = 0;

	while (i.next())
	{
		count++;
	}

	return count;
}

// ---------------------------------------------------- DtoIter ---------------------------------------------------- //

// ** DtoIter::DtoIter
DtoIter::DtoIter(const byte* input, int32 length)
	: m_input(input)
	, m_length(length)
{
	memset(&m_key, 0, sizeof(m_key));
	memset(&m_value, 0, sizeof(m_value));
}

// ** DtoIter::operator bool
DtoIter::operator bool() const
{
	return m_value.type != DtoEnd;
}

// ** DtoIter::operator ==
bool DtoIter::operator == (DtoValueType type) const
{
	return m_value.type == type;
}

// ** DtoIter::operator !=
bool DtoIter::operator != (DtoValueType type) const
{
	return m_value.type != type;
}

// ** DtoIter::next
bool DtoIter::next()
{
	// Decode next entry from an input stream.
	DtoByteBufferInput input(m_input, m_length);
	m_input += BinaryDtoReader::decode(input, m_key, m_value);

	// Skip a nested DTO body
	if (m_value.type == DtoKeyValue || m_value.type == DtoSequence)
	{
		m_input += m_value.binary.length;
	}

	bool isValid = m_value.type != DtoEnd;

	return isValid;
}

// ** DtoIter::type
DtoValueType DtoIter::type() const
{
	return m_value.type;
}

// ** DtoIter::key
const DtoStringView& DtoIter::key() const
{
	return m_key;
}

// ** DtoIter::toBool
bool DtoIter::toBool() const
{
	assert(m_value.type == DtoBool);
	return m_value.boolean;
}

// ** DtoIter::toString
const DtoStringView& DtoIter::toString() const
{
	assert(m_value.type == DtoString);
	return m_value.string;
}

// ** DtoIter::toInt32
int32 DtoIter::toInt32() const
{
	switch (m_value.type)
	{
	case DtoInt32:
		return m_value.int32;
	case DtoDouble:
		return static_cast<int32>(m_value.number);
	}

	assert(m_value.type == DtoInt32);
	return m_value.int32;
}

// ** DtoIter::toDouble
double DtoIter::toDouble() const
{
	assert(m_value.type == DtoDouble);
	return m_value.number;
}

// ** DtoIter::toDto
Dto DtoIter::toDto() const
{
	assert(m_value.type == DtoSequence || m_value.type == DtoKeyValue);
	return Dto(m_value.binary.data, m_value.binary.length);
}

// ---------------------------------------------------- DtoStringView ---------------------------------------------------- //

// ** DtoStringView::operator bool
DtoStringView::operator bool() const
{
	return value != 0 && length > 0;
}

// ** DtoStringView::operator ==
bool DtoStringView::operator == (cstring other) const
{
	return strncmp(value, other, length) == 0;
}

// ** DtoStringView::operator ==
bool DtoStringView::operator == (const DtoStringView& other) const
{
	if (length != other.length)
	{
		return false;
	}

	bool equal = strncmp(value, other.value, length) == 0;
	return equal;
}

// ** DtoStringView::construct
DtoStringView DtoStringView::construct(cstring value)
{
	assert(value);
	DtoStringView result;
	result.value = value;
	result.length = static_cast<int32>(strlen(value));
	return result;
}

// ------------------------------------------------ DtoRegularExpression ------------------------------------------------ //

// ** DtoRegularExpression::construct
DtoRegularExpression DtoRegularExpression::construct(cstring value, cstring options)
{
	assert(value);
	assert(options);

	DtoRegularExpression regex;
	regex.value = DtoStringView::construct(value);
	regex.options = DtoStringView::construct(options);

	return regex;
}

// -------------------------------------------------------- DtoUuid ------------------------------------------------------ //

// ** DtoUuid::null
DtoUuid DtoUuid::null()
{
	DtoUuid result;
	memset(&result, 0, sizeof(result));
	return result;
}

// ** DtoUuid::null
DtoUuid DtoUuid::generate()
{
	assert(0);
	return DtoUuid();
}

DTO_END