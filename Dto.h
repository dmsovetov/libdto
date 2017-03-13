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

#ifndef __Dto_H__
#define __Dto_H__

#ifndef DTO_NAMESPACE
	#define DTO_NAMESPACE Dto
#endif	//	#ifndef DTO_NAMESPACE

#define DTO_BEGIN	namespace DTO_NAMESPACE {
#define DTO_END		}

DTO_BEGIN

	typedef unsigned char		byte;
	typedef unsigned short		uint16;
	typedef int					int32;
	typedef long long			int64;
	typedef unsigned long long	uint64;
	typedef byte				decimal128[16];
	typedef const char*			cstring;

	//! Supported DTO formats.
	enum DtoFormat // Is it better to replace this enum with a set of classes that support format configuration?
	{
		  DtoBson	//!< A default internal DTO format.
		, DtoJson	//!< A JavaScript object notation file format.
		, DtoYaml	//!< A Yaml file format.
		, DtoLson	//!< A Lua script object notation file format.
	};

	//! Enumeration of all available value types.
	enum DtoValueType
	{
		  DtoEnd		= 0x0	//!< Used internally as document, key-value or sequence end indicator.
		, DtoDouble		= 0x1	//!< A 64-bit decimal floating point value.
		, DtoString		= 0x2	//!< A UTF-8 string value.
		, DtoKeyValue	= 0x3	//!< A set of values stored in a key-value manner.
		, DtoSequence	= 0x4	//!< A sequence of DTO values.
		, DtoBinary		= 0x5	//!< A binary blob value.
		, DtoUUID		= 0x7	//!< A 128-bit universally unique id.
		, DtoBool		= 0x8	//!< A boolean value holds true or false.
		, DtoDate		= 0x9	//!< A 64-bit unsigned integer value that represent UTC milliseconds since the Unix epoch.
		, DtoNull		= 0xA	//!< A special value type that indicates that no data is associated with some key.
		, DtoRegEx		= 0xB	//!< A Perl-compatible regular expression value.
		, DtoInt32		= 0x10	//!< A 32-bit signed integer.
		, DtoTimestamp	= 0x11	//!< A 64-bit unsigned integer suitable for storing a Unix timestamp value.
		, DtoInt64		= 0x12	//!< A 64-bit signed integer.
		, DtoDecimal	= 0x13	//!< A 128-bit decimal floating point value.
	};

	//! Enumeration of events emitted by a reader and consumable by a writer.
	enum DtoEventType
	{
		  DtoStreamStart	//!< This event is emitted at the very beginning of a DTO parsing process. 
		, DtoStreamEnd		//!< This event is emitted at the end of a DTO parsing process.
		, DtoKeyValueStart	//!< This event is emitted when a key-value data block encountered.
		, DtoKeyValueEnd	//!< This event is emitted when a key-value data block was fully parsed. 
		, DtoSequenceStart	//!< This event is emitted when a sequence of values encountered.
		, DtoSequenceEnd	//!< This event is emitted when a sequence of values was fully parsed.
		, DtoEntry			//!< This event is emitted when a value was encountered.
	};

	//! A structure to store parsed string value or a key as a string pointer along with string length.
	struct DtoStringView
	{
		cstring			value;		//!< A pointer to a first charecter of a string (this string is NOT zero-terminated).
		int32			length;		//!< A total number of characters in a string.
	};

	//! A structure to store a binary blob value.
	struct DtoBinaryBlob
	{
		const byte*		data;		//!< A pointer to an array of bytes that represent binary blob.
		byte			subtype;	//!< A binary data type.
		int32			length;		//!< A total number of bytes comprising the binary blob.
	};

	//! A structure to store value associated with DTO key.
	struct DtoValue
	{
		DtoValueType		type;		//!< Value data type.

		union
		{
			bool			boolean;
			double			number;
			DtoStringView	string;
			int32			int32;
			int64			int64;
			uint64			uint64;
			DtoBinaryBlob	binary;
		};
	};

	//! An iterator is used to traverse entries stored inside a DTO.
	class DtoIter
	{
	friend class Dto;
	public:

		//! Switches to a next value.
		bool					next();

		//! Returns iterator value type.
		DtoValueType			type() const;

		//! Returns current iterator key.
		const DtoStringView&	key() const;

		//! Returns boolean iterator value.
		bool					toBool() const;

		//! Returns string iterator value.
		const DtoStringView&	toString() const;

		//! Returns integer iterator value.
		int						toInt32() const;

		//! Returns double iterator value.
		double					toDouble() const;

	private:

								//!< Constructs a DtoIter instance.
								DtoIter(const byte* input, int32 length);

	private:

		const byte*				m_input;	//!< An input DTO data.
		int32					m_length;	//!< An input DTO length.
		DtoStringView			m_key;		//!< Entry key this iterator points to.
		DtoValueType			m_type;		//!< Entry value type this iterator points to.
		DtoValue				m_value;	//!< Entry value this iterator points to.
	};

	//! Actual data container that stores it in a key-value manner.
	class Dto
	{
	public:

								//! Constructs an empty DTO instance.
								Dto();

								//! Constructs a DTO instance from an array of bytes.
								Dto(byte* data, int32 capacity);

								//! Returns true if this DTO instance is valid (has a non-zero length and valid data pointer).
								operator bool() const;

		//! Returns a DTO data buffer capacity.
		int32					capacity() const;

		//! Returns a DTO payload length.
		int32					length() const;

		//! Returns a DTO data buffer.
		const byte*				data() const;
		byte*					data();

		//! Returns a DTO iterator instance.
		DtoIter					iter() const;

	private:

		byte*					m_data;		//!< An array of bytes with encoded data.
		int32					m_capacity;	//!< Data buffer capacity.
	};

	/*!
	  A POD structure that describes a single DTO event.
	  DTO event is emitted by a reader and can be consumed by a writer.
	*/
	struct DtoEvent
	{
		DtoEventType		type;		//!< An event type.
		DtoStringView		key;		//!< A key value associated with this event (may be null);
		DtoValue			data;		//!< An associated event data.
	};

	//! An abstract DTO reader interface.
	class DtoReader
	{
	public:

		virtual				~DtoReader() {}

		//! Reads a next event from an input stream.
		virtual DtoEvent	next() = 0;

		//! Returns a total number of consumed bytes.
		virtual int32		consumed() const = 0;
	};

	//! An abstract DTO writer interface.
	class DtoWriter
	{
	public:

		virtual				~DtoWriter() {}

		//! Consumes a single event produced by a reader and returns a total number of bytes written to an output.
		virtual int32		consume(const DtoEvent& event) = 0;
	};

DTO_END

#include "ByteBuffer.h"
#include "Bson.h"
#include "Json.h"
#include "Yaml.h"

#endif	/*	#ifndef __Dto_H__	*/