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

    private:

        //! Enumeration of JSON tokens.
        enum Token
        {
			  TokenNonTerminal		//!< A non terminal symbol encountered.
            , TokenEOF              //!< The end of an input stream encountered.
            , TokenString           //!< A sequence of symbols surrounded by a '"' symbol.
            , TokenNumber           //!< An integer or a floating point number.
            , TokenSequenceStart    //!< The start of a sequence.
            , TokenSequenceEnd      //!< The end of a sequence.
            , TokenKeyValueStart    //!< The start of a key-value node.
            , TokenKeyValueEnd      //!< The end of a key-value node.
            , TokenComma            //!< A comma separator.
            , TokenColon            //!< A colon symbol.
			, TokenTrue				//!< A boolean value token.
			, TokenFalse			//!< A boolean value token.
        };

		//! Contains info about a JSON node being parsed.
		struct Node
		{
			//! A nested node type.
			enum Type
			{
				  Root
				, Sequence
				, KeyValue
			};

			Type					type;		//!< A node value type.
			int32					children;	//!< A total number of nested items.
			bool					closed;		//!< Indicates that no child node are expected anymore.

									//! Constructs a Node instance.
									Node(Type type)
										: type(type), children(0), closed(false) {}
		};

        //! Reads a next token from an input stream.
        Token                       readToken(DtoStringView& text);

		//! Reads a string token from an input stream.
		void						consumeStringToken(DtoStringView& text);

		//! Reads a number token from an input stream.
		void						consumeNumberToken(DtoStringView& text);

		//! Consumes a DtoEntry event from an input stream.
		DtoEvent					eventEntry(const DtoStringView& key);

		//! Consumes a next event from an input stream.
		DtoEvent					eventRoot(Node& node, Token token, const DtoStringView& text);

		//! Consumes a next event from an input stream.
		DtoEvent					eventKeyValue(Node& node, Token token, const DtoStringView& text);

		//! Consumes a next event from an input stream.
		DtoEvent					eventSequence(Node& node, Token token, const DtoStringView& text);

		//! Returns true if an expected token was read from an input stream.
		bool						expectToken(Token token);

		//! Returns true if a next token is a specified one, if so consumes it.
		bool						parseToken(Token token);

		//! Constructs a string value from a token.
		DtoValue					stringFromToken(const DtoStringView& value);

		//! Constructs a number value from a token.
		DtoValue					numberFromToken(const DtoStringView& value);

		//! Returns a topmost JSON node.
		Node&						topmost();

    private:

        DtoByteBufferInput          m_input;    //!< An input byte buffer.
		std::stack<Node>			m_stack;	//!< A node stack.
		char						m_text[64];	//!< An internal temporary string buffer.
    };

DTO_END

#endif	/*	#ifndef __Dto_Json_H__	*/