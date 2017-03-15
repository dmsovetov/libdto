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

#include <stdio.h>
#include <assert.h>
#include "Dto.h"

using namespace DTO_NAMESPACE;

int main(int argc, char** argv)
{
	printf("byte size is %d bytes\n", sizeof(byte));
	printf("int32 size is %d bytes\n", sizeof(int32));
	printf("int64 size is %d bytes\n", sizeof(int64));
	printf("uint64 size is %d bytes\n", sizeof(uint64));
	printf("double size is %d bytes\n", sizeof(double));
	printf("decimal128 size is %d bytes\n", sizeof(decimal128));

	byte document[16536];
	byte copy[16536];

	memset(document, ~0, sizeof(document));
	memset(copy, ~0, sizeof(copy));

	// Encode document
	byte bytes[] = { 1, 2, 3, 4 };

	{
		DtoEncoder encoder(document, sizeof(document));
		DtoBinaryBlob blob = { bytes, 0, sizeof(bytes) };

		encoder
			// Scalar entries
			<< "cstring" << "helloworld"
			<< "binary" << blob
			<< "yes" << true
			<< "no" << false
			<< "double" << 1.12
			<< "int64" << static_cast<int64>(123)
			<< "uint64" << static_cast<uint64>(22)
			// Sequence of numbers
			<< "numbers" << DtoEncoder::sequence
				<< 1 << 2 << 3
				<< DtoEncoder::end

			// Key-value object
			<< "nested" << DtoEncoder::keyValue
				<< "lol" << "troll"
				<< "healh" << 22
				<< DtoEncoder::end

			// Sequence of key-value objects
			<< "players" << DtoEncoder::sequence
				// First key-value object
				<< DtoEncoder::keyValue
					<< "name" << "vasya pupkin"
					<< "rank" << 20
					<< DtoEncoder::end

				// Second key-value object
				<< DtoEncoder::keyValue
					<< "name" << "someplayer"
					<< "rank" << 10
					<< DtoEncoder::end

				<< DtoEncoder::end

			// Sequence of sequences
			<< "sequenceOfSequences" << DtoEncoder::sequence
				// First sequence
				<< DtoEncoder::sequence 
					<< 1 << 2 << 3 << DtoEncoder::end

				// Second sequnce
				<< DtoEncoder::sequence
					<< "a" << "b" << "c" << DtoEncoder::end

				<< DtoEncoder::end

			// Mapping of mappings
			<< "mappingOfMappings" << DtoEncoder::keyValue
				// First mapping
				<< "first" << DtoEncoder::keyValue
					<< "a" << "letter"
					<< "b" << 23
					<< DtoEncoder::end

				// Second mapping
				<< "second" << DtoEncoder::keyValue
					<< "a" << "letter"
					<< "b" << 23
					<< DtoEncoder::end

				<< DtoEncoder::end

			<< DtoEncoder::end;
	}


	// Iterate
	::Dto::Dto dto(document, sizeof(document));
	DtoIter iter = dto.iter();

	printf("encoded document length: %d\n", dto.length());

	printf("contains following fields:\n");
	while (iter.next())
	{
		printf("\t%s\n", iter.key().value);
	}

	// Write using a writer
	{
		BinaryDtoReader reader(document, sizeof(document));
		BinaryDtoWriter writer(copy, sizeof(copy));
		DtoEvent event;

		do
		{
			event = reader.next();
			writer.consume(event);
		} while (event.type != DtoStreamEnd);

		::Dto::Dto first(document, sizeof(document)), second(copy, sizeof(copy));
		int32 a = first.length();
		int32 b = second.length();
		assert(first.length() == second.length());

		int32 length = first.length();
		for (int32 i = 0; i < length; i++)
		{
			assert(first.data()[i] == second.data()[i]);
		}

		assert(memcmp(document, copy, length) == 0);
	}

	byte json[16536];
	memset(json, 0, sizeof(json));

	// Format to a JSON string
	{
		BinaryDtoReader reader(copy, sizeof(copy));
		BinaryDtoWriter binaryWriter(document, sizeof(document));
		JsonDtoWriter jsonWriter(json, sizeof(json));
		//JsonStyledDtoWriter writer(json, sizeof(json));
		//YamlDtoWriter writer(json, sizeof(json));
		DtoEvent event;

		do
		{
			event = reader.next();
			jsonWriter.consume(event);
		} while (event.type != DtoStreamEnd);

		/*JsonDtoReader jsonReader(json, sizeof(json));
		do
		{
			event = jsonReader.next();
			binaryWriter.consume(event);
		} while (event.type != DtoStreamEnd);*/

		printf("\n%s\n", json);
	}
 
	cstring indent[8] =
	{
		  ""
		, "  "
		, "    "
		, "      "
		, "        "
		, "          "
		, "            "
		, "              "
	};

	int32 level = 0;

	JsonDtoReader jsonReader(json, sizeof(json));
	DtoEvent event;
	do
	{
		event = jsonReader.next();

		switch (event.type)
		{
		case DtoError:
			printf("parse error\n");
			break;
		case DtoStreamStart:
			printf("{\n");
			level++;
			break;
		case DtoStreamEnd:
			printf("}\n");
			level--;
			break;
		case DtoKeyValueStart:
			printf("%s%.*s = {\n", indent[level++], event.key.length, event.key.value);
			break;
		case DtoKeyValueEnd:
			printf("%s}\n", indent[--level]);
			break;
		case DtoSequenceStart:
			printf("%s%.*s = [\n", indent[level++], event.key.length, event.key.value);
			break;
		case DtoSequenceEnd:
			printf("%s]\n", indent[--level]);
			break;
		case DtoEntry:
			switch (event.data.type)
			{
			case DtoString:
				printf("%s%.*s = %.*s\n", indent[level], event.key.length, event.key.value, event.data.string.length, event.data.string.value);
				break;
			case DtoBinary:
				printf("%s%.*s = <blob:%d>\n", indent[level], event.key.length, event.key.value, event.data.binary.length);
				break;
			case DtoBool:
				printf("%s%.*s = %s\n", indent[level], event.key.length, event.key.value, event.data.boolean ? "true" : "false");
				break;
			case DtoInt32:
				printf("%s%.*s = %d\n", indent[level], event.key.length, event.key.value, event.data.int32);
				break;
			case DtoInt64:
				printf("%s%.*s = %lld\n", indent[level], event.key.length, event.key.value, event.data.int64);
				break;
			case DtoTimestamp:
				printf("%s%.*s = %lld\n", indent[level], event.key.length, event.key.value, event.data.uint64);
				break;
			case DtoDouble:
				printf("%s%.*s = %g\n", indent[level], event.key.length, event.key.value, event.data.number);
				break;
			}
			break;
		}
	} while (event.type != DtoStreamEnd);

	return 0;
}