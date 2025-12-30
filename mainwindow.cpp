#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QShortcut>
#include <QTextCursor>
#include <QLineEdit>
#include <QInputDialog>
#include <QKeySequenceEdit>
#include <QSettings>
#include <QPushButton>
#include <QLabel>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QListWidget>

MainWindow::MainWindow(QWidget *parent) // constructor
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->installEventFilter(this);
    this->setCentralWidget(ui->textEdit);

    QSettings settings("user", "1");
    loadMacroLibrary();

    // Kopiowanie
    QString copyKey = settings.value("shortcuts/copy", "Esc, Y").toString();
    shortcut_copy = new QShortcut(QKeySequence(copyKey), this);
    connect(shortcut_copy, &QShortcut::activated, this, &MainWindow::copy_keybind);

    // Następne dopasowanie
    QString nextKey = settings.value("shortcuts/next", "Esc, F").toString();
    enterShortcut = new QShortcut(QKeySequence(nextKey), this);
    enterShortcut->setEnabled(false);
    connect(enterShortcut, &QShortcut::activated, this, &MainWindow::findNextMatch);

    // Poprzednie dopasowanie
    QString prevKey = settings.value("shortcuts/prev", "Esc, Shift+F").toString();
    shiftEnterShortcut = new QShortcut(QKeySequence(prevKey), this);
    shiftEnterShortcut->setEnabled(false);
    connect(shiftEnterShortcut, &QShortcut::activated, this, &MainWindow::findPrevMatch);

    // Wyjście z szukania
    QString quitKey = settings.value("shortcuts/quit", "Esc, Q").toString();
    quitSearchShortcut = new QShortcut(QKeySequence(quitKey), this);
    quitSearchShortcut->setEnabled(false);
    connect(quitSearchShortcut, &QShortcut::activated, this, &MainWindow::quitSearch);


    // Kopiowanie linii
    shortcut_copy_and_select_line = new QShortcut(QKeySequence("Esc, Y, Y"), this);
    connect(shortcut_copy_and_select_line, &QShortcut::activated, this, &MainWindow::copy_and_select_line);

    // Nagrywaj makro
    QString recKey =settings.value("shortcuts/macro_rec", "Esc, M").toString();
    macroRecordShortcut = new QShortcut(QKeySequence(recKey), this);
    connect(macroRecordShortcut, &QShortcut::activated, this, &MainWindow::toggleMacroRecord);

    // Puść makro
    QString playKey =settings.value("shortcuts/macro_play", "Esc, P").toString();
    macroPlayShortcut = new QShortcut(QKeySequence(playKey), this);
    connect(macroPlayShortcut, &QShortcut::activated, this, &MainWindow::playMacro);

}
void MainWindow::onChangeShortcutClicked(){
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Keybinds");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Tworzymy pola edycji dla każdego skrótu
    layout->addWidget(new QLabel("Copy (Esc+y):"));
    QKeySequenceEdit *copyEdit = new QKeySequenceEdit(shortcut_copy->key(), &dialog);
    layout->addWidget(copyEdit);

    layout->addWidget(new QLabel("Next Match (Esc+f):"));
    QKeySequenceEdit *nextEdit = new QKeySequenceEdit(enterShortcut->key(), &dialog);
    layout->addWidget(nextEdit);

    layout->addWidget(new QLabel("Prev Match (Esc+Shift+f):"));
    QKeySequenceEdit *prevEdit = new QKeySequenceEdit(shiftEnterShortcut->key(), &dialog);
    layout->addWidget(prevEdit);

    layout->addWidget(new QLabel("Quit Search (Esc+q):"));
    QKeySequenceEdit *quitEdit = new QKeySequenceEdit(quitSearchShortcut->key(), &dialog);
    layout->addWidget(quitEdit);

    QPushButton *btnSave = new QPushButton("Save All", &dialog);
    layout->addWidget(btnSave);

    connect(btnSave, &QPushButton::clicked, &dialog, &QDialog::accept);

    if (dialog.exec() == QDialog::Accepted){
        // Aktualizacja obiektów skrótów
        shortcut_copy->setKey(copyEdit->keySequence());
        enterShortcut->setKey(nextEdit->keySequence());
        shiftEnterShortcut->setKey(prevEdit->keySequence());
        quitSearchShortcut->setKey(quitEdit->keySequence());

        // Zapis do QSettings
        saveShortcut("copy", copyEdit->keySequence());
        saveShortcut("next", nextEdit->keySequence());
        saveShortcut("prev", prevEdit->keySequence());
        saveShortcut("quit", quitEdit->keySequence());

        QMessageBox::information(this, "Success", "Shortcuts updated!");
    }
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::saveShortcut(QString name, QKeySequence seq){
    QSettings settings("user","1");
    settings.setValue("shortcuts/" + name, seq.toString());
}

QList<QTextEdit::ExtraSelection> MainWindow::highlightMatches(const QString &text) {
    QList<QTextEdit::ExtraSelection> selections;

    QTextDocument *doc = ui->textEdit->document();
    QTextCursor cursor(doc);

    while (!(cursor = doc->find(text,cursor)).isNull()){
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(Qt::yellow);
        selection.cursor = cursor;
        selections.append(selection);
    }
    ui->textEdit->setExtraSelections(selections);
    return selections;
}

void MainWindow::copy_keybind(){
    qDebug() << "y clicked";
    ui->textEdit->copy();
}
void MainWindow::quitSearch(){
    currentSelections.clear();
    currentMatchIndex = -1;
    ui->textEdit->setExtraSelections(QList<QTextEdit::ExtraSelection>());

    enterShortcut->setEnabled(false);
    shiftEnterShortcut->setEnabled(false);
    quitSearchShortcut->setEnabled(false);
}



void MainWindow::copy_and_select_line(){
    qDebug() << "yy clicked";
    QTextCursor cursor = ui->textEdit->textCursor();
    cursor.movePosition(QTextCursor::StartOfLine);
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    ui->textEdit->setTextCursor(cursor);
    ui->textEdit->copy();
}


void MainWindow::on_actionNew_triggered()
{
    file_path = "";
    ui->textEdit->setText("");
}


void MainWindow::on_actionOpen_triggered()
{

    QString file_name = QFileDialog::getOpenFileName(this, "Open the file");
    QFile file(file_name);
    file_path = file_name;
    if(!file.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this, " ", "file not open");
        return;
    }
    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->setText(text);
    file.close();
}


void MainWindow::on_actionSave_triggered()
{
    QFile file(file_path);
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, " ", "file not open");
        return;
    }
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();
}


void MainWindow::on_actionSave_as_triggered()
{
    QString file_name = QFileDialog::getSaveFileName(this, "Open the file");
    QFile file(file_name);
    file_path = file_name;
    if(!file.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, " ", "file not open");
        return;
    }
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    file.flush();
    file.close();
}


void MainWindow::on_actionCut_triggered()
{
    ui->textEdit->cut();
}


void MainWindow::on_actionCopy_triggered()
{
    copy_keybind();
}

void MainWindow::on_actionPaste_triggered()
{
    ui->textEdit->paste();
}


void MainWindow::on_actionRedo_triggered()
{
    ui->textEdit->redo();
}


void MainWindow::on_actionUndo_triggered()
{
    ui->textEdit->undo();
}


void MainWindow::on_actionThis_Text_Editor_triggered()
{
    QString about_text = "Author: Grzegorz Kmita";
    QMessageBox::about(this, "About ezvim", about_text);
}


void MainWindow::on_actionSearch_triggered()
{
    bool string_ok;
    QString input = QInputDialog::getText(this, tr("Search"),tr("Input the search query:"), QLineEdit::Normal, "", &string_ok);

    if (string_ok && !input.isEmpty()){
        // ui->textEdit->moveCursor(QTextCursor::Start);
        // ui->textEdit->find(input);
        currentSelections = highlightMatches(input);

        if (!currentSelections.isEmpty()){
            currentMatchIndex = 0;
            enterShortcut->setEnabled(true);
            shiftEnterShortcut->setEnabled(true);
            quitSearchShortcut->setEnabled(true);
            updateSearchFocus();
        }
        else{
            currentMatchIndex = -1;
            enterShortcut->setEnabled(false);
            shiftEnterShortcut->setEnabled(false);

            QMessageBox::information(this, "Search", "No matches found.");
        }
    }

    // bool found = ui->textEdit->find();
    // if (!found) {
    //     ui->textEdit->moveCursor(QTextCursor::Start);
    //     found = ui->textEdit->find("targetWord");
    // }
}

void MainWindow::updateSearchFocus(){
    for (int i=0; i < currentSelections.size(); i++){
        if (i == currentMatchIndex){
            currentSelections[i].format.setBackground(Qt::cyan);
            ui->textEdit->setTextCursor(currentSelections[i].cursor);
        } else {
            currentSelections[i].format.setBackground(Qt::yellow);
        }
    }
    ui->textEdit->setExtraSelections(currentSelections);
}



void MainWindow::findNextMatch() {
    if (currentSelections.isEmpty()) return;
    currentMatchIndex = (currentMatchIndex + 1) % currentSelections.size();
    updateSearchFocus();
}
void MainWindow::findPrevMatch() {
    if (currentSelections.isEmpty()) return;
    currentMatchIndex = (currentMatchIndex - 1 + currentSelections.size()) % currentSelections.size();
    updateSearchFocus();
}

void MainWindow::on_actionSearch_Replace_triggered()
{

}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->textEdit && event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        qDebug() << "Key:" << keyEvent->key()
                 << "Text:" << keyEvent->text()
                 << "Mods:" << keyEvent->modifiers();

        if (isRecording){
            MacroEvent e; // initialize the struct
            e.key = keyEvent->key();
            e.modifiers = keyEvent->modifiers();
            e.text = keyEvent->text();
            macroBuffer.append(e);
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

void MainWindow::toggleMacroRecord() {
    if (!isRecording) {
        macroBuffer.clear();
        isRecording = true;
        ui->statusbar->showMessage("Recording Macro...");
    } else {
        isRecording = false;
        ui->statusbar->showMessage("Recording Stopped", 2000);

        bool ok;
        QString name = QInputDialog::getText(this, "Save Macro", "Macro Name:", QLineEdit::Normal, "", &ok);

        if (ok && !name.isEmpty()){
            QDialog keyDialog(this);
            keyDialog.setWindowTitle("Assign Playback Key");
            QVBoxLayout *lay = new QVBoxLayout(&keyDialog);
            lay->addWidget(new QLabel("Press the playback shortcut:"));
            QKeySequenceEdit *keyEdit = new QKeySequenceEdit(&keyDialog);
            lay->addWidget(keyEdit);
            QPushButton *btn = new QPushButton("Save", &keyDialog);
            lay->addWidget(btn);
            connect(btn, &QPushButton::clicked, &keyDialog, &QDialog::accept);

            if (keyDialog.exec() == QDialog::Accepted) {
                QKeySequence ks = keyEdit->keySequence();

                macroLibrary[name] = macroBuffer;

                if (macroShortcuts.contains(name)) delete macroShortcuts[name];
                QShortcut *s = new QShortcut(ks, this);
                connect(s, &QShortcut::activated, this, [this, name](){ playNamedMacro(name); });
                macroShortcuts[name] = s;

                QSettings settings("user", "1");
                QStringList names = settings.value("macros/library_names").toStringList();
                if (!names.contains(name)) names.append(name);

                settings.setValue("macros/library_names", names);
                settings.setValue("macros/seq_" + name, macroToString());
                settings.setValue("macros/key_" + name, ks.toString());
            }
        }
    }
}

void MainWindow::playMacro(){
    if (isRecording || macroBuffer.isEmpty()) return; // dont play when recording

    ui->textEdit->removeEventFilter(this); // dont record the playback

    for (const MacroEvent &e : std::as_const(macroBuffer)) {
        simulateKey(e.key, e.modifiers, e.text);
    }
    ui->textEdit->installEventFilter(this);
}

void MainWindow::simulateKey(int key, Qt::KeyboardModifiers mods, const QString &text){
    QKeyEvent press(QEvent::KeyPress,key, mods, text);
    QCoreApplication::sendEvent(ui->textEdit->viewport(), &press);

    QKeyEvent release(QEvent::KeyRelease, key, mods, text);
    QCoreApplication::sendEvent(ui->textEdit->viewport(), &release);
}



// QDialog dialog(this);
// dialog.setWindowTitle("Edit Macros");
// QVBoxLayout *layout = new QVBoxLayout(&dialog);

// layout->addWidget(new QLabel("Record macro (Esc+m)"));
// QKeySequenceEdit *recEdit = new QKeySequenceEdit(macroRecordShortcut->key(), &dialog);
// layout->addWidget(recEdit);

// layout->addWidget(new QLabel("Play macro (Esc+p)"));
// QKeySequenceEdit *playEdit = new QKeySequenceEdit(macroPlayShortcut->key(), &dialog);
// layout->addWidget(playEdit);

// layout->addWidget(new QLabel("MacroRecord (Esc+m)"));
// QKeySequenceEdit *recEdit = new QKeySequenceEdit(macroRecordShortcut->key(), &dialog);
// layout->addWidget(recEdit);

// layout->addWidget(new QLabel("MacroRecord (Esc+p)"));
// QKeySequenceEdit *playEdit = new QKeySequenceEdit(macroPlayShortcut->key(), &dialog);
// layout->addWidget(playEdit);


QString MainWindow::macroToString(){
    QStringList parts;
    for(const auto &e : std::as_const(macroBuffer)){
        parts<<QString("%1|%2|%3").arg(e.key).arg((int)e.modifiers).arg(e.text);
    }
    return parts.join(";");
}

void MainWindow::loadMacroFromString(const QString &str) {
    macroBuffer.clear();
    if (str.isEmpty()) return;
    QStringList items = str.split(";");
    for (const QString &item : std::as_const(items)){
        QStringList data = item.split("|");
        if (data.size()>=3) {
            MacroEvent e;
            e.key = data[0].toInt();
            e.modifiers = (Qt::KeyboardModifiers)data[1].toInt();
            e.text = data[2];
            macroBuffer.append(e);
        }
    }
}
void MainWindow::on_actionEditKeybinds_triggered()
{
    onChangeShortcutClicked();
}

void MainWindow::on_actionEditMacros_triggered() {
    QDialog dialog(this);
    dialog.setWindowTitle("Manage Macro Library");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    layout->addWidget(new QLabel("<b>Saved Macros:</b>"));

    QListWidget *listWidget = new QListWidget(&dialog);
    for (auto it = macroLibrary.begin(); it != macroLibrary.end(); ++it) {
        QString keybind = macroShortcuts[it.key()]->key().toString();
        listWidget->addItem(it.key() + " (" + keybind + ")");
    }
    layout->addWidget(listWidget);

    QHBoxLayout *btnLayout = new QHBoxLayout();
    QPushButton *btnDelete = new QPushButton("Delete Selected", &dialog);
    QPushButton *btnClose = new QPushButton("Close", &dialog);
    btnLayout->addWidget(btnDelete);
    btnLayout->addWidget(btnClose);
    layout->addLayout(btnLayout);

    connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::accept);

    connect(btnDelete, &QPushButton::clicked, this, [this, listWidget]() {
        QListWidgetItem *item = listWidget->currentItem();
        if (item) {
            QString name = item->text().split(" (").first();
            auto reply = QMessageBox::question(this, "Confirm Delete",
                                               "Are you sure you want to delete: " + name + "?",
                                               QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes){
                macroLibrary.remove(name);
                if (macroShortcuts.contains(name)) {
                    delete macroShortcuts[name];
                    macroShortcuts.remove(name);
                }

                QSettings settings("user", "1");
                QStringList names = settings.value("macros/library_names").toStringList();
                names.removeAll(name);
                settings.setValue("macros/library_names", names);
                settings.remove("macros/seq_" + name);
                settings.remove("macros/key_" + name);

                delete item;
                QMessageBox::information(this, "Deleted", "Macro removed.");
            }
        } else {
            QMessageBox::warning(this, "Selection", "Please select a macro to delete.");
        }
    });

    dialog.exec();
}
void MainWindow::loadMacroLibrary(){
    QSettings settings("user", "1");
    QStringList names = settings.value("macros/library_names").toStringList();

    for (const QString &name: std::as_const(names)){
        QString sequenceStr = settings.value("macros/seq_"+name).toString();
        QString keyStr = settings.value("macros/key_"+name).toString();

        QList<MacroEvent> buffer;
        QStringList items= sequenceStr.split(";", Qt::SkipEmptyParts);

        for (const QString &item : std::as_const(items)){
            QStringList data= item.split("|");
            if (data.size()>=3){
                MacroEvent e;
                e.key=data[0].toInt();
                e.modifiers = (Qt::KeyboardModifiers)data[1].toInt();
                e.text = data[2];
                buffer.append(e);
            }
        }
        macroLibrary[name]=buffer;
        QShortcut *s = new QShortcut(QKeySequence(keyStr), this);
        connect(s, &QShortcut::activated, this, [this, name](){ playNamedMacro(name); });
        macroShortcuts[name] = s;
    }
}

void MainWindow::playNamedMacro(QString name){
    if (isRecording) return;

    QList<MacroEvent> buffer = macroLibrary[name];
    ui->textEdit->removeEventFilter(this);

    for (const auto &e: std::as_const(buffer)){
        simulateKey(e.key, e.modifiers, e.text);
    }
    ui->textEdit->installEventFilter(this);
}

