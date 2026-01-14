#include "file_io.h"
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QCoreApplication>
#include <QTimer>

FileIO::FileIO(QTextEdit *editor, QMainWindow *parent)
    : QObject(parent), m_editor(editor), m_parent(parent) {
    m_filePath = "";
    m_loadTimer = new QTimer(this);
    connect(m_loadTimer, &QTimer::timeout, this, &FileIO::loadNextChunk);
}

QString FileIO::getFilePathHelper(const QString& path) {
    if (!path.isEmpty()) return path;
    return QFileDialog::getOpenFileName(m_parent, "Open Large File");
}

void FileIO::updateTitle() {
    QString name = m_filePath.isEmpty() ? "Untitled" : m_filePath;
    m_parent->setWindowTitle(name + " - EzVim");
}

void FileIO::openFileLazy(QString path) {
    path = getFilePathHelper(path);
    if (path.isEmpty()) return;

    if (m_currentFile.isOpen()) m_currentFile.close();

    m_currentFile.setFileName(path);
    if (!m_currentFile.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(m_parent, "Error", "Cannot open file");
        return;
    }

    m_filePath = path;

    m_editor->clear();
    m_editor->setReadOnly(true);

    m_editor->setUpdatesEnabled(false);

    m_editor->blockSignals(true);

    m_editor->setPlaceholderText("Loading optimized buffer...");
    // ---------------------------------

    m_loadTimer->start(1); // Start background loading
}

void FileIO::loadNextChunk() {
    if (m_currentFile.atEnd()) {
        m_loadTimer->stop();
        m_currentFile.close();

        m_editor->setUpdatesEnabled(true); // Re-enable drawing
        m_editor->blockSignals(false);     // Re-enable signals

        m_editor->setReadOnly(false);
        m_editor->setPlaceholderText("");

        m_editor->repaint();

        updateTitle();
        return;
    }

    QByteArray rawData = m_currentFile.read(CHUNK_SIZE);
    QString chunk = QString::fromUtf8(rawData);

    // Append text at the end
    m_editor->moveCursor(QTextCursor::End);
    m_editor->insertPlainText(chunk);

    // Update progress in the title bar
    qint64 size = m_currentFile.size();
    if (size > 0) {
        int progress = (m_currentFile.pos() * 100) / size;
        m_parent->setWindowTitle(QString("[%1%] Loading %2...").arg(progress).arg(m_filePath));
    }
}
void FileIO::newFile(){
    m_filePath = "";
    m_editor->setText("");
    updateTitle();
}

void FileIO::saveFile(){
    if (m_filePath.isEmpty()){
        saveFileAs();
        return;
    }

    QFile file(m_filePath);
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(m_parent, "Error", "cannot save file");
        return;
    }
    QTextStream out(&file);
    QString text = m_editor->toPlainText();
    out << text;
    file.flush();
    file.close();

    updateTitle();
}
void FileIO::saveFileAs(){
    QString m_filePath = QFileDialog::getSaveFileName(m_parent, "Save File As");
    QFile file(m_filePath);
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(m_parent, " ", "file not open");
        return;
    }
    QTextStream out(&file);
    QString text = m_editor->toPlainText();
    out << text;
    file.flush();
    file.close();

    updateTitle();
}
