//
// filelistmodel.h: Class definition
//
// by James Hammons
// (C) 2010 Underground Software
//

#ifndef __FILELISTMODEL_H__
#define __FILELISTMODEL_H__

#include <QtWidgets>
#include <vector>
#include <stdint.h>

struct FileListData
{
//	FileListData(unsigned long ul=0, QString str="", QImage img=QImage()): dbIndex(ul), filename(str), label(img) {}
//	FileListData(unsigned long ul=0, QString str, QImage img): dbIndex(ul), filename(str), label(img) {}

	unsigned long dbIndex;
	unsigned long fileSize;
	QString filename;
	QImage label;
	bool hasUniversalHeader;
	uint32_t fileType;
	uint32_t crc;
};

//hm.
#define FLM_INDEX			(Qt::UserRole + 1)
#define FLM_FILESIZE		(Qt::UserRole + 2)
#define FLM_FILENAME		(Qt::UserRole + 3)
#define FLM_LABEL			(Qt::UserRole + 4)
#define FLM_UNIVERSALHDR	(Qt::UserRole + 5)
#define FLM_FILETYPE		(Qt::UserRole + 6)
#define FLM_CRC				(Qt::UserRole + 7)

class FileListModel: public QAbstractListModel
{
	public:
		FileListModel(QObject * parent = 0);

		int rowCount(const QModelIndex & parent = QModelIndex()) const;
		QVariant data(const QModelIndex & index, int role) const;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

//		void AddData(QIcon pix);
//		void AddData(unsigned long);
		void AddData(unsigned long, QString, QImage, unsigned long);
		void AddData(unsigned long, QString, QImage, unsigned long, bool, uint32_t, uint32_t);
		void ClearData(void);

//		FileListData GetData(const QModelIndex & index) const;

	private:
		std::vector<FileListData> list;
};

#endif	// __FILELISTMODEL_H__
