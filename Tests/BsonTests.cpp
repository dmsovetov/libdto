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

void construct(byte* document, int32 capacity)
{
	DtoEncoder(document, capacity)
		<< "a" << 1
		<< "b" << 2.32
		<< "c" << "hello world"
		<< "d" << true
		<< "e" << static_cast<uint64>(1234)

		<< "sequence" << DtoEncoder::sequence
			<< 1 << 2 << 3 << DtoEncoder::end

		<< "mapping" << DtoEncoder::keyValue
			<< "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
		
		<< "sequenceOfSequences" << DtoEncoder::sequence
			<< DtoEncoder::sequence << 5 << 6 << 7 << DtoEncoder::end
			<< DtoEncoder::sequence << 5 << 6 << 7 << DtoEncoder::end
			<< DtoEncoder::sequence << 5 << 6 << 7 << DtoEncoder::end
			<< DtoEncoder::end
		
		<< "sequenceOfMappings" << DtoEncoder::sequence
			<< DtoEncoder::keyValue << "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
			<< DtoEncoder::keyValue << "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
			<< DtoEncoder::keyValue << "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
			<< DtoEncoder::end
		
		<< "mappingOfMappings" << DtoEncoder::keyValue
			<< "one"   << DtoEncoder::keyValue << "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
			<< "two"   << DtoEncoder::keyValue << "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
			<< "three" << DtoEncoder::keyValue << "a" << "hello" << "b" << "world" << "c" << true << DtoEncoder::end
			<< DtoEncoder::end
		
		<< "mappingOfSequences" << DtoEncoder::keyValue
			<< "one"   << DtoEncoder::sequence << 5 << 6 << 7 << DtoEncoder::end
			<< "two"   << DtoEncoder::sequence << 5 << 6 << 7 << DtoEncoder::end
			<< "three" << DtoEncoder::sequence << 5 << 6 << 7 << DtoEncoder::end
			<< DtoEncoder::end

		<< DtoEncoder::end;
}

static cstring kJson = "{\"a\":1,\"b\":2.32,\"c\":\"hello world\",\"d\":true,\"e\":1234,\"sequence\":[1,2,3],\"mapping\":{\"a\":\"hello\",\"b\":\"world\",\"c\":true},\"sequenceOfSequences\":[[5,6,7],[5,6,7],[5,6,7]],\"sequenceOfMappings\":[{\"a\":\"hello\",\"b\":\"world\",\"c\":true},{\"a\":\"hello\",\"b\":\"world\",\"c\":true},{\"a\":\"hello\",\"b\":\"world\",\"c\":true}],\"mappingOfMappings\":{\"one\":{\"a\":\"hello\",\"b\":\"world\",\"c\":true},\"two\":{\"a\":\"hello\",\"b\":\"world\",\"c\":true},\"three\":{\"a\":\"hello\",\"b\":\"world\",\"c\":true}},\"mappingOfSequences\":{\"one\":[5,6,7],\"two\":[5,6,7],\"three\":[5,6,7]}}";

static cstring kYaml =
"a: 1\n"
"b: 2.32\n"
"c: hello world\n"
"d: true\n"
"e: 1234\n"
"sequence: \n"
"  - 1\n"
"  - 2\n"
"  - 3\n"
"mapping: \n"
"  a: hello\n"
"  b: world\n"
"  c: true\n"
"sequenceOfSequences: \n"
"  - \n"
"    - 5\n"
"    - 6\n"
"    - 7\n"
"  - \n"
"    - 5\n"
"    - 6\n"
"    - 7\n"
"  - \n"
"    - 5\n"
"    - 6\n"
"    - 7\n"
"sequenceOfMappings: \n"
"  - \n"
"    a: hello\n"
"    b: world\n"
"    c: true\n"
"  - \n"
"    a: hello\n"
"    b: world\n"
"    c: true\n"
"  - \n"
"    a: hello\n"
"    b: world\n"
"    c: true\n"
"mappingOfMappings: \n"
"  one: \n"
"    a: hello\n"
"    b: world\n"
"    c: true\n"
"  two: \n"
"    a: hello\n"
"    b: world\n"
"    c: true\n"
"  three: \n"
"    a: hello\n"
"    b: world\n"
"    c: true\n"
"mappingOfSequences: \n"
"  one: \n"
"    - 5\n"
"    - 6\n"
"    - 7\n"
"  two: \n"
"    - 5\n"
"    - 6\n"
"    - 7\n"
"  three: \n"
"    - 5\n"
"    - 6\n"
"    - 7\n"
;

static byte document[16536];

TEST(Bson, ReadWriteCompare)
{
	byte document[4096];
	construct(document, sizeof(document));
	byte copy[4096];

	dtoConvert<BinaryDtoReader, BinaryDtoWriter>(document, sizeof(document), copy, sizeof(copy));

	::Dto::Dto first(document, sizeof(document));
	::Dto::Dto second(copy, sizeof(copy));

	EXPECT_EQ(first.length(), second.length());
	EXPECT_EQ(memcmp(document, copy, first.length()), 0);
}

TEST(Bson, ToJson)
{
	byte document[4096];
	construct(document, sizeof(document));
	byte json[4000];

	dtoConvert<BinaryDtoReader, JsonDtoWriter>(document, sizeof(document), json, sizeof(json));
	EXPECT_STREQ(reinterpret_cast<cstring>(json), kJson);
}

TEST(Bson, FromJson)
{
	byte document[16000], duplicate[16000];
	construct(document, sizeof(document));
	byte json[16000];
	byte copy[16000];

	// First convert a BSON to JSON
	ASSERT_TRUE((dtoConvert<BinaryDtoReader, JsonDtoWriter>(document, sizeof(document), json, sizeof(json))));
	EXPECT_STREQ(reinterpret_cast<cstring>(json), kJson);

	// Now parse BSON from JSON
	ASSERT_TRUE(dtoParse<JsonDtoReader>(json, document, sizeof(document)));

	// And convert it back to JSON
	ASSERT_TRUE((dtoConvert<BinaryDtoReader, JsonDtoWriter>(document, sizeof(document), copy, sizeof(copy))));
	EXPECT_STREQ(reinterpret_cast<cstring>(json), reinterpret_cast<cstring>(copy));

	// Finally parse a JSON...
	ASSERT_TRUE(dtoParse<JsonDtoReader>(copy, duplicate, sizeof(duplicate)));

	// ...and compare two BSON documents
	::Dto::Dto first(document, sizeof(document));
	::Dto::Dto second(duplicate, sizeof(duplicate));

	EXPECT_EQ(first.length(), second.length());
	EXPECT_EQ(memcmp(document, duplicate, first.length()), 0);
}

TEST(Bson, ToYaml)
{
	byte document[4096];
	construct(document, sizeof(document));
	byte yaml[4000];

	ASSERT_TRUE((dtoConvert<BinaryDtoReader, YamlDtoWriter>(document, sizeof(document), yaml, sizeof(yaml))));
	EXPECT_STREQ(reinterpret_cast<cstring>(yaml), kYaml);
}

/*TEST(Bson, FromYaml)
{
	byte document[16000], duplicate[16000];
	construct(document, sizeof(document));
	byte yaml[16000];
	byte copy[16000];

	// First convert a BSON to Yaml
	dtoConvert<BinaryDtoReader, YamlDtoWriter>(document, sizeof(document), yaml, sizeof(yaml));
	EXPECT_STREQ(reinterpret_cast<cstring>(yaml), kYaml);

	// Now parse BSON from Yaml
	dtoParse<YamlDtoReader>(yaml, document, sizeof(document));

	// And convert it back to Yaml
	dtoConvert<BinaryDtoReader, YamlDtoWriter>(document, sizeof(document), copy, sizeof(copy));
	EXPECT_STREQ(reinterpret_cast<cstring>(yaml), reinterpret_cast<cstring>(copy));

	// Finally parse a Yaml...
	dtoParse<YamlDtoReader>(copy, duplicate, sizeof(duplicate));

	// ...and compare two BSON documents
	::Dto::Dto first(document, sizeof(document));
	::Dto::Dto second(duplicate, sizeof(duplicate));

	EXPECT_EQ(first.length(), second.length());
	EXPECT_EQ(memcmp(document, duplicate, first.length()), 0);
}*/