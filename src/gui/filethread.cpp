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
// JLH  06/28/2011  Cleanup in the file parsing/fishing code, to make it easier to follow the flow of the logic
// JPM  06/06/2016  Visual Studio support
// JPM  09/25/2024  Recursive sub-directories search in the software's path
//

#include "filethread.h"
#include "crc32.h"
#include "file.h"
#include "filedb.h"
#include "settings.h"


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


// Looking for files from the software's path
void FileThread::run(void)
{
	// get list of files/dir found in the path
	QDir romDir(vjs.ROMPath);
	QFileInfoList list = romDir.entryInfoList();

	// loop on the list
	for (int i = 0; (i < list.size()) && !abort; i++)
	{
		HandleFile(list.at(i));
	}
}


// Handles file identification and ZIP extraction
void FileThread::HandleFile(QFileInfo fileInfo)
{
	// check dir
	if (fileInfo.isDir())
	{
		// avoid . and .. diretory names
		if (strcmp(fileInfo.fileName().toStdString().c_str(), ".") && strcmp(fileInfo.fileName().toStdString().c_str(), ".."))
		{
			// recursive loop on the directory content
			QDir subDir(fileInfo.filePath().toStdString().c_str());
			QFileInfoList list = subDir.entryInfoList();
			for (int i = 0; (i < list.size()) && !abort; i++)
			{
				HandleFile(list.at(i));
			}
		}
	}
	else
	{
		// check presence zip file
		bool haveZIPFile = (fileInfo.suffix().compare("zip", Qt::CaseInsensitive) == 0 ? true : false);
		uint32_t fileSize = 0;
		uint8_t * buffer = NULL;

		// get file size, either from a zip or a unique file
		if (haveZIPFile)
		{
			fileSize = GetFileFromZIP(fileInfo.filePath().toUtf8(), FT_SOFTWARE, buffer);
		}
		else
		{
			fileSize = fileInfo.size();
			if (fileSize)
			{
				QFile file(fileInfo.filePath());
				if (file.open(QIODevice::ReadOnly))
				{
					buffer = new uint8_t[fileSize];
					file.read((char *)buffer, fileSize);
					file.close();
				}
			}
		}

		if (fileSize)
		{
			// tentative to divine the file type by size & header, or if none has been found, from the extension file
			int fileType = ParseFileType(buffer, fileSize);
			(fileType == JST_NONE) ? (fileType = ParseFileExt(fileInfo.suffix().toLocal8Bit().constData())) : false;

			// check for Alpine ROM w/Universal Header
			bool foundUniversalHeader = HasUniversalHeader(buffer, fileSize);

			// get the title's database index based on the crc checksum
			uint32_t crc;
			if (foundUniversalHeader)
			{
				crc = crc32_calcCheckSum(buffer + 8192, fileSize - 8192);
			}
			else
			{
				crc = crc32_calcCheckSum(buffer, fileSize);
			}
			uint32_t index = FindCRCIndexInFileList(crc);

			// BIOS filtering
			bool bios = (index != 0xFFFFFFFF) && (romList[index].flags & FF_BIOS);
			// filter the non 'official' rom, without valid type but still allowed by the emulation's setting
			bool allow = !((index == 0xFFFFFFFF) && (fileType == JST_NONE));

			// file can be on the list
			if (!bios && (allow || allowUnknownSoftware))
			{
				// no rom label image
				QImage * img = NULL;

				// get the rom label's image, if any, fron a zip file
				if (haveZIPFile)
				{
					uint8_t * buff = NULL;
					uint32_t size = GetFileFromZIP(fileInfo.filePath().toUtf8(), FT_LABEL, buff);
					if (size > 0)
					{
						QImage label;
						label.loadFromData(buff, size);
						img = new QImage;
						*img = label.scaled(365, 168, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
					}
					delete[] buff;
				}

				// add the file in the list
				emit FoundAFile3(index, fileInfo.canonicalFilePath(), img, fileSize, foundUniversalHeader, fileType, crc);
			}
		}

		delete[] buffer;
	}
}


// Find a CRC in the ROM list (simple brute force algorithm)
//
// Return the index, otherwise return $FFFFFFFF
uint32_t FileThread::FindCRCIndexInFileList(uint32_t crc)
{
	for (int i = 0; (romList[i].crc32 != 0xFFFFFFFF); i++)
	{
		if (romList[i].crc32 == crc)
		{
			return i;
		}
	}

	return 0xFFFFFFFF;
}
