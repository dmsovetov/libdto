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

#include "Tests.h"

typedef DtoTokenInput::Token Token;

class Tokenizer : public ::testing::Test
{
public:

	void expect(DtoTokenInput& input, DtoTokenInput::TokenType type, int32 line, int32 column, cstring text = nullptr)
	{
		Token token = input.next();
		ASSERT_EQ(token, type);
		EXPECT_EQ(token.line, line);
		EXPECT_EQ(token.column, column);

		if (text)
		{
			EXPECT_EQ(token.text, text);
		}
	}
};

TEST_F(Tokenizer, EmptyString)
{
	DtoTokenInput input("");
	expect(input, DtoTokenInput::End, 1, 1);
}

TEST_F(Tokenizer, True)
{
	DtoTokenInput input("true");
	expect(input, DtoTokenInput::True, 1, 1);
	expect(input, DtoTokenInput::End, 1, 5);
}

TEST_F(Tokenizer, False)
{
	DtoTokenInput input("false");
	expect(input, DtoTokenInput::False, 1, 1);
	expect(input, DtoTokenInput::End, 1, 6);
}

TEST_F(Tokenizer, NewLine)
{
	DtoTokenInput input("\n\r\n");
	expect(input, DtoTokenInput::NewLine, 1, 1);
	expect(input, DtoTokenInput::NewLine, 2, 1);
	expect(input, DtoTokenInput::End, 3, 1);
}

TEST_F(Tokenizer, SpacesAndTabs)
{
	DtoTokenInput input(" \t ");
	expect(input, DtoTokenInput::Space, 1, 1);
	expect(input, DtoTokenInput::Tab, 1, 2);
	expect(input, DtoTokenInput::Space, 1, 3);
	expect(input, DtoTokenInput::End, 1, 4);
}

TEST_F(Tokenizer, Minus)
{
	DtoTokenInput input("-");
	expect(input, DtoTokenInput::Minus, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, BraceOpen)
{
	DtoTokenInput input("{");
	expect(input, DtoTokenInput::BraceOpen, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, BraceClose)
{
	DtoTokenInput input("}");
	expect(input, DtoTokenInput::BraceClose, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, BracketOpen)
{
	DtoTokenInput input("[");
	expect(input, DtoTokenInput::BracketOpen, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, BracketClose)
{
	DtoTokenInput input("]");
	expect(input, DtoTokenInput::BracketClose, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, Comma)
{
	DtoTokenInput input(",");
	expect(input, DtoTokenInput::Comma, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, Colon)
{
	DtoTokenInput input(":");
	expect(input, DtoTokenInput::Colon, 1, 1);
	expect(input, DtoTokenInput::End, 1, 2);
}

TEST_F(Tokenizer, Integer)
{
	DtoTokenInput input("1 123 434 4533545");
	expect(input, DtoTokenInput::Number, 1, 1);
	expect(input, DtoTokenInput::Space, 1, 2);
	expect(input, DtoTokenInput::Number, 1, 3);
	expect(input, DtoTokenInput::Space, 1, 6);
	expect(input, DtoTokenInput::Number, 1, 7);
	expect(input, DtoTokenInput::Space, 1, 10);
	expect(input, DtoTokenInput::Number, 1, 11);
	expect(input, DtoTokenInput::End, 1, 18);
}

TEST_F(Tokenizer, Decimal)
{
	DtoTokenInput input("1.3 1.23 43.4 4533.545");
	expect(input, DtoTokenInput::Number, 1, 1);
	expect(input, DtoTokenInput::Space, 1, 4);
	expect(input, DtoTokenInput::Number, 1, 5);
	expect(input, DtoTokenInput::Space, 1, 9);
	expect(input, DtoTokenInput::Number, 1, 10);
	expect(input, DtoTokenInput::Space, 1, 14);
	expect(input, DtoTokenInput::Number, 1, 15);
	expect(input, DtoTokenInput::End, 1, 23);
}

TEST_F(Tokenizer, SingleQuotedString)
{
	DtoTokenInput input("'hello world'");
	expect(input, DtoTokenInput::SingleQuotedString, 1, 1, "hello world");
	expect(input, DtoTokenInput::End, 1, 14);
}

TEST_F(Tokenizer, DoubleQuotedString)
{
	DtoTokenInput input("\"hello world\"");
	expect(input, DtoTokenInput::DoubleQuotedString, 1, 1, "hello world");
	expect(input, DtoTokenInput::End, 1, 14);
}

TEST_F(Tokenizer, Identifiers)
{
	DtoTokenInput input("hello world_2");
	expect(input, DtoTokenInput::Identifier, 1, 1, "hello");
	expect(input, DtoTokenInput::Space, 1, 6);
	expect(input, DtoTokenInput::Identifier, 1, 7, "world_2");
	expect(input, DtoTokenInput::End, 1, 14);
}

TEST_F(Tokenizer, IdentifierSeparators)
{
	DtoTokenInput input("hello:world_2");
	expect(input, DtoTokenInput::Identifier, 1, 1, "hello");
	expect(input, DtoTokenInput::Colon, 1, 6);
	expect(input, DtoTokenInput::Identifier, 1, 7, "world_2");
	expect(input, DtoTokenInput::End, 1, 14);
}