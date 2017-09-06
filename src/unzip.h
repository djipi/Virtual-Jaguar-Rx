#ifndef __UNZIP_H__
#define __UNZIP_H__

#include <stdio.h>
#include <stdint.h>

struct ZipFileEntry
{
	uint32_t signature;
	uint16_t version;
	uint16_t flags;
	uint16_t method;
	uint16_t modifiedTime;
	uint16_t modifiedDate;
	uint32_t crc32;
	uint32_t compressedSize;
	uint32_t uncompressedSize;
	uint16_t filenameLength;
	uint16_t extraLength;
	uint8_t filename[512];
};

bool GetZIPHeader(FILE *, ZipFileEntry &);
int UncompressFileFromZIP(FILE *, ZipFileEntry, uint8_t *);

#endif	// __UNZIP_H__
