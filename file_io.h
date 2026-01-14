#ifndef FILE_IO_H
#define FILE_IO_H

#include <QObject>
#include <QString>
#include <QTextEdit>
#include <QMainWindow>
#include <QFile>
#include <QTimer>
#include <QList>

class FileIO : public QObject {
    Q_OBJECT
public:
    explicit FileIO(QTextEdit *editor, QMainWindow *parent);

    void newFile();
    void openFile();
    void openFileBufferedNewDS(QString path = "");
    void openFileLazy(QString path = "");
    void saveFile();
    void saveFileAs();

    QString currentPath() const { return m_filePath; }

private slots:
    void loadNextChunk();

private:
    QTextEdit *m_editor;
    QMainWindow *m_parent;
    QString m_filePath;
    QList<QString> m_internalBuffer;

    // For Lazy Loading
    QFile m_currentFile;
    QTimer *m_loadTimer;
    const int CHUNK_SIZE = 1024 * 256; // 256KB

    void updateTitle();
    QString getFilePathHelper(const QString& path);
};

#endif
