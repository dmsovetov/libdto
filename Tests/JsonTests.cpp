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

TEST(Json, WontParseEmptyString)
{
	cstring json = "";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_FALSE(dto);
}

TEST(Json, ParsesEmptyObject)
{
	cstring json = "{}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), 5);
	EXPECT_EQ(dto.entryCount(), 0);
}

TEST(Json, ParsesTrue)
{
	cstring json = "{\"a\":true}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);
	
	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_TRUE(i.toBool());
}

TEST(Json, ParsesFalse)
{
	cstring json = "{\"a\":false}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_FALSE(i.toBool());
}

TEST(Json, ParsesIntegers)
{
	cstring json = "{\"a\":123}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toInt32(), 123);
}

TEST(Json, ParsesNegativeIntegers)
{
	cstring json = "{\"a\":-123}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toInt32(), -123);
}

TEST(Json, ParsesNegativeIntegersInsideArrays)
{
	cstring json = "{\"a\":[-123, -1, -2]}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	ASSERT_TRUE(i == DtoSequence);

	DtoIter _0 = dto.findDescendant("a.0");
	ASSERT_TRUE(_0);
	EXPECT_EQ(_0.toInt32(), -123);

	DtoIter _1 = dto.findDescendant("a.1");
	ASSERT_TRUE(_1);
	EXPECT_EQ(_1.toInt32(), -1);

	DtoIter _2 = dto.findDescendant("a.2");
	ASSERT_TRUE(_2);
	EXPECT_EQ(_2.toInt32(), -2);
}

TEST(Json, ParsesDecimals)
{
	cstring json = "{\"a\":12.23}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toDouble(), 12.23);
}

TEST(Json, ParsesNegativeDecimals)
{
	cstring json = "{\"a\":-12.23}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_EQ(i.toDouble(), -12.23);
}

TEST(Json, ParsesNegativeDecimalsInsideArrays)
{
	cstring json = "{\"a\":[-12.23, -1.2]}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	ASSERT_TRUE(i == DtoSequence);

	DtoIter _0 = dto.findDescendant("a.0");
	ASSERT_TRUE(_0);
	EXPECT_EQ(_0.toDouble(), -12.23);

	DtoIter _1 = dto.findDescendant("a.1");
	ASSERT_TRUE(_1);
	EXPECT_EQ(_1.toDouble(), -1.2);
}

TEST(Json, ParsesStrings)
{
	cstring json = "{\"a\":\"hello world\"}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);

	EXPECT_TRUE(i.toString() == "hello world");
}

TEST(Json, ParsesComplexObjects)
{
	cstring json = "{\"a\":12.23,\"b\":1,\"c\":true}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);

	DtoIter _a = dto.find("a");
	ASSERT_TRUE(_a);
	EXPECT_EQ(_a.toDouble(), 12.23);

	DtoIter _b = dto.find("b");
	ASSERT_TRUE(_b);
	EXPECT_EQ(_b.toInt32(), 1);

	DtoIter _c = dto.find("c");
	ASSERT_TRUE(_c);
	EXPECT_TRUE(_c.toBool());
}

TEST(Json, HandlesWhitespaceChars)
{
	cstring json = "{\"a\" :   12.23,\"b\":1,\r\n\n\r\n\"c\": \ttrue}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);

	DtoIter _a = dto.find("a");
	ASSERT_TRUE(_a);
	EXPECT_EQ(_a.toDouble(), 12.23);

	DtoIter _b = dto.find("b");
	ASSERT_TRUE(_b);
	EXPECT_EQ(_b.toInt32(), 1);

	DtoIter _c = dto.find("c");
	ASSERT_TRUE(_c);
	EXPECT_TRUE(_c.toBool());
}

TEST(Json, ParsesNestedEmptyObjects)
{
	cstring json = "{\"a\":{}}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoKeyValue);
	EXPECT_EQ(i.toDto().entryCount(), 0);
}

TEST(Json, ParsesNestedEmptyArrays)
{
	cstring json = "{\"a\":[]}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoSequence);
}

TEST(Json, ParsesNestedObjects)
{
	cstring json = "{\"a\":{\"b\":0}}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoKeyValue);
}

TEST(Json, ParsesNestedComplexObjects)
{
	cstring json = "{\"a\":{\"b\":0,\"c\":1,\"d\":2}}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.entryCount(), 1);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoKeyValue);
}

TEST(Json, ParsesNestedComplexArrays)
{
	cstring json = "{\"a\":[0,1,2,3]}";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);

	DtoIter i = dto.find("a");
	ASSERT_TRUE(i);
	EXPECT_EQ(i.type(), DtoSequence);
}

TEST(Json, ParsesRootEmptyArray)
{
	cstring json = "[]";
	DtoType dto = dtoParse<JsonDtoReader>(json, document, sizeof(document));
	ASSERT_TRUE(dto);
}