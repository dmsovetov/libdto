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

#ifndef __Dto_Json_H__
#define __Dto_Json_H__

DTO_BEGIN

	//! Consumes a sequence of DTO events and produces a compact JSON string.
	class JsonDtoWriter : public DtoWriter
	{
	public:

									//! Constructs a JSON data writer.
									JsonDtoWriter(byte* output, int32 capacity, cstring keyValueSeparator = "");

		//! Consumes an event an writes next entry to an output stream.
		virtual int32				consume(const DtoEvent& event);

	private:
		
		//! Removes a trailing comma symbol from an output.
		DtoTextOutput&				removeTrailingComma();

		//! Outputs an entry key based on a current node.
		DtoTextOutput&				key(const DtoStringView& value);

	protected:

		DtoTextOutput				m_output;				//!< An output data buffer.
		std::stack<DtoValueType>	m_stack;				//!< A DTO value type stack.
		cstring						m_keyValueSeparator;	//!< A separator between key and value.
	};

	//! Consumes a sequence of DTO events and produces a styled JSON string.
	class JsonStyledDtoWriter : public JsonDtoWriter
	{
	public:

									//! Constructs a JSON data writer.
									JsonStyledDtoWriter(byte* output, int32 capacity, cstring indent = "  ", cstring newLine = "\r\n");

		//! Consumes an event an writes next entry to an output stream.
		virtual int32				consume(const DtoEvent& event);

	private:

		//! Outputs an indentation characters.
		void						indentation(int32 level);

	private:

		cstring						m_indent;	//!< An indentation character.
		cstring						m_newLine;	//!< A new line string.
	};

    //! Parses a JSON string and produces a sequence of DTO events consumable by writer.
    class JsonDtoReader : public DtoReader
    {
    public:

                                    //! Constructs a JSON DTO reader instance.
                                    JsonDtoReader(const byte* input, int32 length);

        //! Parses a next event from an input stream.
        virtual DtoEvent            next();

        //! Returns a total number of consumed bytes.
		virtual int32		        consumed() const;

    protected:

		struct State
		{
			DtoEvent (*parser)();
		};

		//! Contains info about a JSON node being parsed.
		struct Node
		{
			//! A nested node type.
			enum Type
			{
				  Sequence
				, KeyValue
			};

			Type					type;		//!< A node value type.
			int32					children;	//!< A total number of nested items.
			bool					closed;		//!< Indicates that no child node are expected anymore.

									//! Constructs a Node instance.
									Node(Type type)
										: type(type), children(0), closed(false) {}
		};

		typedef DtoEvent (JsonDtoReader::*EventParser)();

		//! Parses a DtoEntry event from an input stream.
		DtoEvent					parsePrimitive(const DtoStringView& key);

		//! Parses a next event from an input stream.
		DtoEvent					parseKeyValue();

		//! Parses a next event from an input stream.
		DtoEvent					parseItem();

		//! Expects to parse a closing brace at the end of a stream.
		DtoEvent					expectBraceStreamEnd();

		//! Expects to parse a closing bracket at the end of a stream.
		DtoEvent					expectBracketStreamEnd();

		//! Expects to parse a closing brace at the end of a key-value node.
		DtoEvent					expectKeyValueEnd();

		//! Expects to parse a closing bracket at the end of a sequence node.
		DtoEvent					expectSequenceEnd();

		//! Continues parsing sequence if comma token encountered.
		DtoEvent					continueSequence();

		//! Continues parsing an object if comma token encountered.
		DtoEvent					continueKeyValue();

		//! A root event parser.
		DtoEvent					parseStream();

    protected:

        DtoTokenInput				m_input;    //!< An input token stream.
		std::stack<EventParser>		m_stack;	//!< A event parser stack.
		std::stack<int>				m_index;	//!< A sequence item index stack.
		char						m_text[64];	//!< An internal temporary string buffer.
    };

DTO_END

#endif	/*	#ifndef __Dto_Json_H__	*/