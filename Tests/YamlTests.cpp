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

static byte document[16536];
typedef ::Dto::Dto DtoType;

TEST(Yaml, ParsesEmptyString)
{
	cstring yaml = "";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), 5);
	EXPECT_EQ(dto.entryCount(), 0);
}

TEST(Yaml, ParsesTrue)
{
	cstring yaml = "a:true";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_TRUE(i.toBool());
}

TEST(Yaml, ParsesFalse)
{
	cstring yaml = "a:false";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_FALSE(i.toBool());
}

TEST(Yaml, ParsesIntegers)
{
	cstring yaml = "a:123";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toInt32(), 123);
}

TEST(Yaml, ParsesNegativeIntegers)
{
	cstring yaml = "a:-123";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toInt32(), -123);
}

TEST(Yaml, ParsesNegativeIntegersInsideArrays)
{
	cstring yaml = "a:\n"
				   "  - -1\n"
				   "  - -3\n"
				   "  - -5\n"
				   ;
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
}

TEST(Yaml, ParsesDecimals)
{
	cstring yaml = "a:12.23";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toDouble(), 12.23);
}

TEST(Yaml, ParsesNegativeDecimals)
{
	cstring yaml = "a:-12.23";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toDouble(), -12.23);
}

TEST(Yaml, ParsesNegativeDecimalsInsideArrays)
{
	cstring yaml =	"a:\n"
					"  - -12.23\n"
					"  - -1.2\n"
					;
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
}

TEST(Yaml, ParsesStrings)
{
	cstring yaml = "a:hello world";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_TRUE(i.toString() == "hello world");
}

TEST(Yaml, ParsesComplexObjects)
{
	cstring yaml = "a:12.23\nb:1\nc:true";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
}

TEST(Yaml, HandlesWhitespaceChars)
{
	cstring yaml = "a :   12.23\n\nb :1\r\n\n\r\nc: \ttrue";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
}

TEST(Yaml, ParsesNestedEmptyObjects)
{
	cstring yaml = "a:{}";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoKeyValue);
}

TEST(Yaml, ParsesNestedEmptyArrays)
{
	cstring yaml = "a:[]";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoSequence);
}

TEST(Yaml, ParsesNestedObjects)
{
	cstring yaml =	"a: \n"
					"  b:0\n";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoKeyValue);
}

TEST(Yaml, ParsesNestedComplexObjects)
{
	cstring yaml =	"a:\n"
					"  b:0\n"
					"  c:1\n"
					"  d:2\n";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoKeyValue);
}

TEST(Yaml, ParsesNestedComplexArrays)
{
	cstring yaml = "a:\n"
					"  - 0\n"
					"  - 1\n"
					"  - 2\n";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_TRUE(dto);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoSequence);
}

TEST(Yaml, WontParseRootArray)
{
	cstring yaml = "[]";
	DtoType dto = dtoParse<YamlDtoReader>(yaml, document, sizeof(document));
	ASSERT_FALSE(dto);
}