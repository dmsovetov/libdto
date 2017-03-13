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

#ifndef __Dto_Yaml_H__
#define __Dto_Yaml_H__

DTO_BEGIN

	//! Consumes a sequence of DTO events and produces a Yaml string.
	class YamlDtoWriter : public DtoWriter
	{
	public:

								//! Constructs a Yaml data writer.
								YamlDtoWriter(byte* output, int32 capacity, cstring newLine = "\r\n");

		//! Consumes an event an writes next entry to an output stream.
		virtual int32			consume(const DtoEvent& event);

	private:

		//! Outputs indentation.
		void					indentation(int32 level);

		//! Outputs an entry key based on a current node.
		DtoTextOutput&			key(const DtoStringView& value);

	protected:

		//! A nested JSON node info
		struct Nested
		{
			DtoValueType		type;		//!< A node type.
			bool				isEmpty;	//!< Indicates that at lease one entry was output to a stream.

								//! Constructs a Nested instance.
								Nested(DtoValueType type)
									: type(type), isEmpty(true) {}
		};

		DtoTextOutput			m_output;	//!< An output data buffer.
		std::stack<Nested>		m_stack;	//!< A DTO value type stack.
		cstring					m_newLine;	//!< A new line symbol.
	};

DTO_END

#endif	/*	#ifndef __Dto_Yaml_H__	*/