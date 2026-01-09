#include "file_io.h"

#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>

FileIO::FileIO(QTextEdit *editor, QMainWindow *parent)
    : QObject(parent), m_editor(editor), m_parent(parent){
    m_filePath="";
}

void FileIO::updateTitle() {
    QString name = m_filePath.isEmpty() ? "Untitled" : m_filePath;
    m_parent->setWindowTitle(name + " - EzVim");
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

void FileIO::openFile(){
    QString file_name = QFileDialog::getOpenFileName(m_parent, "Open the file");
    if (file_name.isEmpty()) return; // user cancelled

    QFile file(file_name);
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(m_parent, " ", "file not open");
        return;
    }
    m_filePath = file_name;
    QTextStream in(&file);
    QString text = in.readAll();
    m_editor->setText(text);

    file.close();
    updateTitle();
}
