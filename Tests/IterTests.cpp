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
#include <set>
#include <string>

static void construct(byte* document, int32 length)
{
	DtoEncoder(document, length)
		<< "a" << 1 << "b" << 2.0 << "c" << "hello"
		<< "sequence" << DtoEncoder::sequence
			<< 1 << 2 << 3 << DtoEncoder::end
		<< "mapping" << DtoEncoder::keyValue
			<< "aa" << 1 << "bb" << 2 << "cc" << 3 << DtoEncoder::end
		<< DtoEncoder::end;
}

TEST(Iter, IteratesOverItems)
{
	byte document[500];
	construct(document, sizeof(document));

	::Dto::Dto dto(document, sizeof(document));
	DtoIter iter = dto.iter();

	std::set<std::string> keys;
	keys.insert("a");
	keys.insert("b");
	keys.insert("c");
	keys.insert("sequence");
	keys.insert("mapping");

	while (iter.next())
	{
		char key[64];
		memset(key, 0, sizeof(key));
		strncpy_s(key, iter.key().value, iter.key().length);

		EXPECT_TRUE(keys.find(key) != keys.end());
		if (keys.count(key))
		{
			keys.erase(keys.find(key));
		}
	}

	EXPECT_TRUE(keys.empty());
}

TEST(Iter, IteratesOnlyTopLevelItems)
{
	byte document[500];
	construct(document, sizeof(document));

	::Dto::Dto dto(document, sizeof(document));
	DtoIter iter = dto.iter();

	std::set<std::string> keys;
	keys.insert("a");
	keys.insert("b");
	keys.insert("c");
	keys.insert("sequence");
	keys.insert("mapping");

	while (iter.next())
	{
		char key[64];
		memset(key, 0, sizeof(key));
		strncpy_s(key, iter.key().value, iter.key().length);

		EXPECT_TRUE(keys.find(key) != keys.end());
		keys.erase(keys.find(key));

		if (strcmp(key, "a") == 0)
		{
			EXPECT_EQ(iter.type(), DtoInt32);
			EXPECT_EQ(iter.toInt32(), 1);
		}
		else if (strcmp(key, "b") == 0)
		{
			EXPECT_EQ(iter.type(), DtoDouble);
			EXPECT_EQ(iter.toDouble(), 2.0);
		}
		else if (strcmp(key, "c") == 0)
		{
			EXPECT_EQ(iter.type(), DtoString);
			EXPECT_EQ(strncmp(iter.toString().value, "hello", iter.toString().length), 0);
		}
		else if (strcmp(key, "sequence") == 0)
		{
			EXPECT_EQ(iter.type(), DtoSequence);
		}
		else if (strcmp(key, "mapping") == 0)
		{
			EXPECT_EQ(iter.type(), DtoKeyValue);
		}
	}

	EXPECT_TRUE(keys.empty());
}

TEST(Iter, Find)
{
	byte document[5000];
	construct(document, sizeof(document));

	::Dto::Dto dto(document, sizeof(document));
	EXPECT_TRUE(dto.find("a"));
	EXPECT_TRUE(dto.find("b"));
	EXPECT_TRUE(dto.find("c"));
	EXPECT_TRUE(dto.find("sequence"));
	EXPECT_TRUE(dto.find("mapping"));

	EXPECT_FALSE(dto.find("aa"));
	EXPECT_FALSE(dto.find("bb"));
	EXPECT_FALSE(dto.find("cc"));
}

TEST(Iter, FindDescendant)
{
	byte document[5000];
	construct(document, sizeof(document));

	::Dto::Dto dto(document, sizeof(document));

	EXPECT_TRUE(dto.findDescendant("a"));
	EXPECT_TRUE(dto.findDescendant("b"));
	EXPECT_TRUE(dto.findDescendant("c"));

	EXPECT_TRUE(dto.findDescendant("sequence.0"));
	EXPECT_TRUE(dto.findDescendant("sequence.1"));
	EXPECT_TRUE(dto.findDescendant("sequence.2"));
	EXPECT_FALSE(dto.findDescendant("sequence.3"));

	EXPECT_TRUE(dto.findDescendant("mapping.aa"));
	EXPECT_TRUE(dto.findDescendant("mapping.bb"));
	EXPECT_TRUE(dto.findDescendant("mapping.cc"));
	EXPECT_FALSE(dto.findDescendant("mapping.dd"));
}