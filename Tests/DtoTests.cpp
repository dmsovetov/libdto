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

TEST(Dto, IsInvalid_AfterDefaultConstructor)
{
	::Dto::Dto dto;
	EXPECT_FALSE(dto);
}

TEST(Dto, HasZeroLength_AfterDefaultConstructor)
{
	::Dto::Dto dto;
	EXPECT_EQ(0, dto.length());
}

TEST(Dto, HasZeroCapacity_AfterDefaultConstructor)
{
	::Dto::Dto dto;
	EXPECT_EQ(0, dto.capacity());
}

TEST(Dto, HasNullData_AfterDefaultConstructor)
{
	::Dto::Dto dto;
	EXPECT_EQ(0, dto.data());
}

TEST(Dto, ReturnsInvalidIter_AfterDefaultConstructor)
{
	::Dto::Dto dto;
	EXPECT_FALSE(dto.iter());
}

TEST(Dto, HasData_AfterConstruction)
{
	byte document[50];
	::Dto::Dto dto(document, sizeof(document));
	EXPECT_TRUE(dto.data() != NULL);
}

TEST(Dto, HasValidCapacity_AfterConstruction)
{
	byte document[50];
	int32 capacity = sizeof(document);
	::Dto::Dto dto(document, capacity);
	EXPECT_EQ(capacity, dto.capacity());
}