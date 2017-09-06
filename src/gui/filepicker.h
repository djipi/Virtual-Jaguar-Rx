//
// filepicker.h - A ROM chooser
//

#include <QtWidgets>

// Forward declarations
class QListWidget;
class FileThread;
class FileListModel;
class QListView;

class FilePickerWindow: public QWidget
{
	// Once we have signals/slots, we need this...
	Q_OBJECT

	public:
		FilePickerWindow(QWidget * parent = 0);
		QString GetSelectedPrettyName(void);
		void ScanSoftwareFolder(bool allow = false);

	public slots:
		void AddFileToList(unsigned long index);
		void AddFileToList2(unsigned long index, QString, QImage *, unsigned long size);
		void AddFileToList3(unsigned long index, QString, QImage *, unsigned long size, bool, unsigned long, unsigned long);
		void UpdateSelection(const QModelIndex &, const QModelIndex &);
		void LoadButtonPressed(void);
		void CatchDoubleClick(const QModelIndex &);

	signals:
		void RequestLoad(QString);
		void FilePickerHiding(void);

	protected:
		void keyPressEvent(QKeyEvent *);
//		void PopulateList(void);

	private:
		QString currentFile;
		QString prettyFilename;
		QListWidget * fileList2;
		FileThread * fileThread;
		FileListModel * model;
		QListView * fileList;
		QLabel * cartImage;
		QLabel * title;
		QLabel * data;
		QPushButton * insertCart;
};
