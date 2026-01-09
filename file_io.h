#ifndef FILE_IO_H
#define FILE_IO_H

#include <QObject>
#include <QString>
#include <QTextEdit>
#include <QMainWindow>


class FileIO : public QObject {
    Q_OBJECT
public:
    // for managing mainwindow and textedit
    explicit FileIO(QTextEdit *editor, QMainWindow *parent);

    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();

    QString currentPath() const {return m_filePath; }

private:
    QTextEdit *m_editor;
    QMainWindow *m_parent;
    QString m_filePath;

    void updateTitle();
};

#endif
