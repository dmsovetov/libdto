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

static cstring keyA = "a";
static cstring regex = "(\\w+)+";

enum ItemSize
{
	  Length			= 4
	, End				= 1
	, ValueType			= 1
	, AKey				= 2
	, Bool				= 1
	, Int32				= 4
	, Int64				= 8
	, Double			= 8
	, BString			= Length + 2
};

typedef ::Dto::Dto DtoType;

TEST(Encoder, SizeOf_EncodedEmptyDocument)
{
	byte document[500];
	DtoEncoder encoder(document, sizeof(document));
	encoder << DtoEncoder::end;
	
	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + End);
}

TEST(Encoder, SizeOf_EncodedDouble)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << 1.0 << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + Double + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedString)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << "b" << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + BString + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedEmptyKeyValue)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << DtoEncoder::keyValue << DtoEncoder::end << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + (ValueType + (Length + End)) + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedEmptySequence)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << DtoEncoder::sequence << DtoEncoder::end << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + (ValueType + (Length + End)) + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedBinaryBlob)
{
	byte document[500];
	byte binary[4];
	DtoBinaryBlob blob = { binary, 0, sizeof(binary) };

	DtoEncoder(document, sizeof(document))
		<< keyA << blob << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + (Length + ValueType + sizeof(binary)) + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedUUID)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << DtoUuid::null() << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + (16) + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedBool)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << true << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + Bool + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedDate)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << static_cast<int64>(0) << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + Int64 + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedNull)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << DtoEncoder::null << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedRegEx)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << DtoRegularExpression::construct(regex) << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + (strlen(regex) + 1) + (strlen("") + 1) + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedInt32)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << 0 << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + Int32 + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedTimestamp)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << static_cast<uint64>(0) << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + Int64 + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

TEST(Encoder, SizeOf_EncodedInt64)
{
	byte document[500];
	DtoEncoder(document, sizeof(document))
		<< keyA << static_cast<int64>(0) << DtoEncoder::end;

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_EQ(dto.length(), Length + AKey + ValueType + Int64 + End);

	DtoIter i = dto.find(keyA);
	EXPECT_TRUE(i.key() == keyA);
}

/*TEST(Encoder, SizeOf_EncodedDecimal)
{
	byte document[500];
	//DtoEncoder encoder(document, sizeof(document));

	DtoType dto(document, sizeof(document));
	ASSERT_TRUE(dto);
	EXPECT_TRUE(false);
}*/