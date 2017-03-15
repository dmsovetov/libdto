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

#ifndef __Dto_Bson_H__
#define __Dto_Bson_H__

#include <stack>

DTO_BEGIN

	// A helper class to encode a DTO with C++ stream operators.
	class DtoEncoder
	{
	public:

		//! A proxy type that is used byte writer to finalize DTO encoding.
		enum marker { end, keyValue, sequence };

							//! Constructs a DtoEncoder instance.
							DtoEncoder(byte* output, int32 capacity);

							~DtoEncoder();

		//! Appends a boolean value to an output.
		DtoEncoder&			operator << (bool value);

		//! Appends a double value to an output.
		DtoEncoder&			operator << (double value);

		//! Appends a 32-bit signed integer value to an output.
		DtoEncoder&			operator << (int32 value);

		//! Appends a 64-bit signed integer value to an output.
		DtoEncoder&			operator << (int64 value);

		//! Appends a 64-bit unsigned integer value to an output.
		DtoEncoder&			operator << (uint64 value);

		//! Appends a UTF-8 string or acts as a entry key specification.
		DtoEncoder&			operator << (cstring value);

		//! Appends a binary data to an output buffer
		DtoEncoder&			operator << (const DtoBinaryBlob& value);

		//! Appends a nested key-value node to an output buffer.
		DtoEncoder&			operator << (const DtoEncoder& value);

		//! Finalizes a DTO encoding.
		DtoEncoder&			operator << (marker value);

		//! Returns a total number of bytes that was written by this encoder.
		int32				length() const;

		//! Returns a destination byte buffer.
		const byte*			data() const;

		//! Returns true if an encoded DTO is complete.
		bool				complete() const;

	private:

		//! Returns an entry key and resets it after a use.
		DtoStringView		entryKey();

		//! Returns true if a key for the next entry was set.
		bool				hasKey() const;

	protected:

		//! A nested DTO info.
		struct Nested
		{
			byte*			ptr;			//!< A document root pointer.
			int32			index;			//!< Next entry index used by sequence encoder.

							//! Constructs a Nested instance.
							Nested(byte* ptr = NULL, int32 index = -1)
								: ptr(ptr), index(index) {}
		};

		DtoByteArrayOutput	m_output;		//!< An output byte array.
		cstring				m_key;			//!< An active key value.
		std::stack<Nested>	m_stack;		//!< A stack of pointers to track nested DTOs.
		char				m_index[6];		//!< A termporary buffer used for sequence item index formatting.
	};

	//! Consumes a sequence of DTO events and produces a DTO binary representation.
	class BinaryDtoWriter : public DtoWriter
	{
	public:

							//! Constructs a BSON data writer.
							BinaryDtoWriter(byte* output, int32 capacity);

		//! Consumes an event an writes next entry to an output stream.
		virtual int32		consume(const DtoEvent& event);

		//! Encodes a DTO entry to an output stream and returns a total number of bytes that was written.
		static int32		encode(DtoByteArrayOutput& output, const DtoStringView& key, const DtoValue& value);

	private:

		//! Starts writing a nested DTO of specified type.
		void				start(const DtoStringView& key, DtoValueType type);

		//! Finalizes a topmost DTO on stack by writing a zero-terminator and calculating the final DTO length.
		void				finish();

	private:

		DtoByteArrayOutput	m_output;	//!< An output data buffer.
		std::stack<byte*>	m_stack;	//!< An object stack to track nesting.
	};

	//! A BSON compatible DTO reader.
	class BinaryDtoReader : public DtoReader
	{
	public:

							BinaryDtoReader(const byte* input, int32 length);

		//! Decodes next event from an input stream.
		virtual DtoEvent	next();

		//! Returns a total number of consumed bytes.
		virtual int32		consumed() const;

		//! Decodes a single entry from an input stream and returns a total number of consumed bytes.
		static int32		decode(DtoByteBufferInput& input, DtoStringView& key, DtoValue& value);

	private:

		//! Pushes a new document to the stack.
		DtoEventType		push(DtoValueType type, int32 length);

		//! Pops a document from the stack.
		DtoEventType		pop();

	private:

		//! A structure to hold nested DTO info.
		struct Nested
		{
			int32			length;
			byte			type;

							//! Constructs a Nested instance.
							Nested(int32 length = 0, byte type = 0)
								: length(length), type(type) {}
		};

		DtoByteBufferInput	m_input;	//!< An input byte buffer stream.
		std::stack<Nested>	m_stack;	//!< An object stack to track nesting.
	};

DTO_END

#endif	/*	#ifndef __Dto_Bson_H__	*/