//
// filethread.cpp - File discovery thread
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/28/2010  Created this file
// JLH  02/16/2010  Moved RomIdentifier stuff to its own file
// JLH  03/02/2010  Added .ZIP file fishing
// JLH  06/28/2011  Cleanup in the file parsing/fishing code, to make it easier
//                  to follow the flow of the logic
//
// JPM  06/06/2016  Visual Studio support

#include "filethread.h"

#include "crc32.h"
#include "file.h"
#include "filedb.h"
//#include "memory.h"
#include "settings.h"

#define VERBOSE_LOGGING

FileThread::FileThread(QObject * parent/*= 0*/): QThread(parent), abort(false)
{
}

FileThread::~FileThread()
{
	mutex.lock();
	abort = true;
	condition.wakeOne();
	mutex.unlock();

	wait();
}

void FileThread::Go(bool allowUnknown/*= false*/)
{
	allowUnknownSoftware = allowUnknown;
	QMutexLocker locker(&mutex);
	start();
}

/*
Our strategy here is like so:
Look at the files in the directory pointed to by ROMPath.
For each file in the directory, take the CRC32 of it and compare it to the CRC
in the romList[]. If there's a match, put it in a list and note it's index value
in romList for future reference.

When constructing the list, use the index to pull up an image of the cart and
put that in the list. User picks from a graphical image of the cart.

Ideally, the label will go into the archive along with the ROM image, but that's
for the future...
Maybe box art, screenshots will go as well...
The future is NOW! :-)
*/

//
// Here's the thread's actual execution path...
//
void FileThread::run(void)
{
	QDir romDir(vjs.ROMPath);
	QFileInfoList list = romDir.entryInfoList();

	for(int i=0; i<list.size(); i++)
	{
		if (abort)
#ifdef VERBOSE_LOGGING
{
printf("FileThread: Aborting!!!\n");
#endif
			return;
#ifdef VERBOSE_LOGGING
}
#endif

		HandleFile(list.at(i));
	}
}

//
// This handles file identification and ZIP extraction.
//
void FileThread::HandleFile(QFileInfo fileInfo)
{
	// Really, need to come up with some kind of cacheing scheme here, so we don't
	// fish through these files every time we run VJ :-P
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to come up with some kind of cacheing scheme here !!!")
#else
#warning "!!! Need to come up with some kind of cacheing scheme here !!!"
#endif // _MSC_VER
	bool haveZIPFile = (fileInfo.suffix().compare("zip", Qt::CaseInsensitive) == 0
		? true : false);
	uint32_t fileSize = 0;
	uint8_t * buffer = NULL;

	if (haveZIPFile)
	{
		// ZIP files are special: They contain more than just the software now... ;-)
		// So now we fish around inside them to pull out the stuff we want.
		// Probably also need more stringent error checking as well... :-O
		fileSize = GetFileFromZIP(fileInfo.filePath().toUtf8(), FT_SOFTWARE, buffer);

		if (fileSize == 0)
			return;
	}
	else
	{
		QFile file(fileInfo.filePath());

		if (!file.open(QIODevice::ReadOnly))
			return;

		fileSize = fileInfo.size();

		if (fileSize == 0)
			return;

		buffer = new uint8_t[fileSize];
		file.read((char *)buffer, fileSize);
		file.close();
	}

	// Try to divine the file type by size & header
	int fileType = ParseFileType(buffer, fileSize);

	// Check for Alpine ROM w/Universal Header
	bool foundUniversalHeader = HasUniversalHeader(buffer, fileSize);
	uint32_t crc;

//printf("FileThread: About to calc checksum on file with size %u... (buffer=%08X)\n", size, buffer);
	if (foundUniversalHeader)
		crc = crc32_calcCheckSum(buffer + 8192, fileSize - 8192);
	else
		crc = crc32_calcCheckSum(buffer, fileSize);

	uint32_t index = FindCRCIndexInFileList(crc);
	delete[] buffer;

	// Here we filter out files that are *not* in the DB and of unknown type,
	// and BIOS files. If desired, this can be overriden with a config option.
	if ((index == 0xFFFFFFFF) && (fileType == JST_NONE))
	{
		// If we allow unknown software, we pass the (-1) index on, otherwise...
		if (!allowUnknownSoftware)
			return;								// CRC wasn't found, so bail...
	}
	else if ((index != 0xFFFFFFFF) && romList[index].flags & FF_BIOS)
		return;

//Here's a little problem. When we create the image here and pass it off to FilePicker,
//we can clobber this image before we have a chance to copy it out in the FilePicker function
//because we can be back here before FilePicker can respond.
// So now we create the image on the heap, problem solved. :-)
	QImage * img = NULL;

	// See if we can fish out a label. :-)
	if (haveZIPFile)
	{
		uint32_t size = GetFileFromZIP(fileInfo.filePath().toUtf8(), FT_LABEL, buffer);
//printf("FT: Label size = %u bytes.\n", size);

		if (size > 0)
		{
			QImage label;
			bool successful = label.loadFromData(buffer, size);
			img = new QImage;
			*img = label.scaled(365, 168, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
//printf("FT: Label %s: %ux%u.\n", (successful ? "succeeded" : "did not succeed"), img->width(), img->height());
			delete[] buffer;
		}
//printf("FileThread: Attempted to load image. Size: %u x %u...\n", img.width(), img.height());
	}

//	emit FoundAFile2(index, fileInfo.canonicalFilePath(), img, fileSize);
	emit FoundAFile3(index, fileInfo.canonicalFilePath(), img, fileSize, foundUniversalHeader, fileType, crc);
}

//
// Find a CRC in the ROM list (simple brute force algorithm).
// If it's there, return the index, otherwise return $FFFFFFFF
//
uint32_t FileThread::FindCRCIndexInFileList(uint32_t crc)
{
	// Instead of a simple brute-force search, we should probably do a binary
	// partition search instead, since the CRCs are sorted numerically.
#ifdef _MSC_VER
#pragma message("Warning: !!! Should do binary partition search here !!!")
#else
#warning "!!! Should do binary partition search here !!!"
#endif // _MSC_VER
	for(int i=0; romList[i].crc32!=0xFFFFFFFF; i++)
	{
		if (romList[i].crc32 == crc)
			return i;
	}

	return 0xFFFFFFFF;
}
