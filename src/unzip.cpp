//
// ZIP file support
// This is here to simplify interfacing to zlib, as zlib does NO zip file handling
//
// by James Hammons
// (C) 2012 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JLH  02/28/2010  Removed unnecessary cruft
// JLH  05/31/2012  Rewrote everything and removed all MAME code
//

#include "unzip.h"

#include <stdlib.h>
#include <string.h>
#include <zlib.h>
#include "log.h"


uint32_t GetLong(FILE * fp)
{
	uint32_t n = ((uint32_t)fgetc(fp));
	n |= ((uint32_t)fgetc(fp)) << 8;
	n |= ((uint32_t)fgetc(fp)) << 16;
	n |= ((uint32_t)fgetc(fp)) << 24;

	return n;
}


uint16_t GetWord(FILE * fp)
{
	uint16_t n = ((uint16_t)fgetc(fp));
	n |= ((uint16_t)fgetc(fp)) << 8;

	return n;
}


bool GetZIPHeader(FILE * fp, ZipFileEntry & ze)
{
	ze.signature = GetLong(fp);
	ze.version = GetWord(fp);
	ze.flags = GetWord(fp);
	ze.method = GetWord(fp);
	ze.modifiedTime = GetWord(fp);
	ze.modifiedDate = GetWord(fp);
	ze.crc32 = GetLong(fp);
	ze.compressedSize = GetLong(fp);
	ze.uncompressedSize = GetLong(fp);
	ze.filenameLength = GetWord(fp);
	ze.extraLength = GetWord(fp);

	// This handling is really ungraceful; but if someone is going to feed us
	// shit, then why eat it? :-)
	if (ze.filenameLength < 512)
	{
		fread(ze.filename, 1, ze.filenameLength, fp);
		ze.filename[ze.filenameLength] = 0;
	}
	else
	{
		fseek(fp, ze.filenameLength, SEEK_CUR);
		ze.filename[0] = 0;
	}

	// Skip the "extra" header
	fseek(fp, ze.extraLength, SEEK_CUR);

	return (ze.signature == 0x04034B50 ? true : false);
}


//
// Uncompress a file from a ZIP file filestream
// NOTE: The passed in buffer *must* be fully allocated before calling this!
//
#define CHUNKSIZE 16384
int UncompressFileFromZIP(FILE * fp, ZipFileEntry ze, uint8_t * buffer)
{
	z_stream zip;
	unsigned char inBuffer[CHUNKSIZE];
	uint32_t remaining = ze.compressedSize;

	// Set up z_stream for inflating
	zip.zalloc = Z_NULL;
	zip.zfree = Z_NULL;
	zip.opaque = Z_NULL;
	zip.avail_in = 0;
	zip.next_in = Z_NULL;

	int ret = inflateInit2(&zip, -MAX_WBITS);	// -MAX_WBITS tells it there's no header

	// Bail if can't initialize the z_stream...
	if (ret != Z_OK)
		return ret;

	zip.avail_out = ze.uncompressedSize;
	zip.next_out = buffer;

	// Decompress until deflate stream ends or we hit end of file
	do
	{
		zip.avail_in = fread(inBuffer, 1, (remaining < CHUNKSIZE ? remaining : CHUNKSIZE), fp);
		zip.next_in = inBuffer;
		remaining -= CHUNKSIZE;

		if (ferror(fp))
		{
			inflateEnd(&zip);
			return Z_ERRNO;
		}

		if (zip.avail_in == 0)
			break;

		ret = inflate(&zip, Z_NO_FLUSH);

		if ((ret == Z_NEED_DICT) || (ret == Z_DATA_ERROR) || (ret == Z_MEM_ERROR))
		{
			inflateEnd(&zip);
			return ret;
		}

	}
	while (ret != Z_STREAM_END);

	inflateEnd(&zip);

	return (ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR);
}
