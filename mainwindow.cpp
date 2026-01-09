#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "highlighter.h"
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
#include <QProcess>
#include <QColorDialog>
#include <QPalette>

MainWindow::MainWindow(QWidget *parent) // constructor
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->textEdit->installEventFilter(this);
    this->setCentralWidget(ui->textEdit);

    QSettings settings("user", "1");
    macroManager = new MacroManager(ui->textEdit, this);
    macroManager->loadMacroLibrary();

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

    // Skrót Systemowy
    QString sysCommandKey = settings.value("shortcuts/sysCommand", "Esc, !").toString();
    systemCommandShortcut = new QShortcut(QKeySequence(sysCommandKey), this);
    connect(systemCommandShortcut, &QShortcut::activated, this, &MainWindow::runSystemCommand);

    // Inicjalizacja Highlightera
    highlighter = new Highlighter(ui->textEdit->document());

    // Kolory (Usunięto ponowną definicję settings)
    QString bgName = settings.value("colors/bg", "#ffffff").toString();
    QString textName = settings.value("colors/text", "#000000").toString();

    QPalette p = ui->textEdit->palette();
    p.setColor(QPalette::Base, QColor(bgName));
    p.setColor(QPalette::Text, QColor(textName));
    ui->textEdit->setPalette(p);

    fileIO = new FileIO(ui->textEdit, this);
}
void MainWindow::onChangeShortcutClicked(){
    QDialog dialog(this);
    dialog.setWindowTitle("Edit Keybinds");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Tworzymy pola edycji dla każdego skrótu
    layout->addWidget(new QLabel("Copy:"));
    QKeySequenceEdit *copyEdit = new QKeySequenceEdit(&dialog);
    copyEdit->setKeySequence(shortcut_copy->key());
    layout->addWidget(copyEdit);

    layout->addWidget(new QLabel("Next Match:"));
    QKeySequenceEdit *nextEdit = new QKeySequenceEdit(&dialog);
    nextEdit->setKeySequence(enterShortcut->key());
    layout->addWidget(nextEdit);

    layout->addWidget(new QLabel("Prev Match:"));
    QKeySequenceEdit *prevEdit = new QKeySequenceEdit(&dialog);
    prevEdit->setKeySequence(shiftEnterShortcut->key());
    layout->addWidget(prevEdit);

    layout->addWidget(new QLabel("Quit Search:"));
    QKeySequenceEdit *quitEdit = new QKeySequenceEdit(&dialog);
    quitEdit->setKeySequence(quitSearchShortcut->key());
    layout->addWidget(quitEdit);

    layout->addWidget(new QLabel("Run System Command:"));
    QKeySequenceEdit *runSystemCommandEdit = new QKeySequenceEdit(&dialog);
    runSystemCommandEdit->setKeySequence(systemCommandShortcut->key());
    layout->addWidget(runSystemCommandEdit);

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
    fileIO->newFile();
}


void MainWindow::on_actionOpen_triggered()
{

    fileIO->openFile();
}


void MainWindow::on_actionSave_triggered()
{
    fileIO->saveFile();
}


void MainWindow::on_actionSave_as_triggered()
{
    fileIO->saveFileAs();
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


void MainWindow::on_actionEditKeybinds_triggered()
{
    onChangeShortcutClicked();
}

void MainWindow::on_actionEditMacros_triggered() {
    QDialog dialog(this);
    dialog.setWindowTitle("Manage Macro Library");
    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QListWidget *listWidget = new QListWidget(&dialog);
    // Access via macroManager pointer:
    for (auto it = macroManager->macroLibrary.begin(); it != macroManager->macroLibrary.end(); ++it) {
        QString name = it.key();
        QString keybind = "No Key";
        if (macroManager->macroShortcuts.contains(name) && macroManager->macroShortcuts[name]){
            keybind = macroManager->macroShortcuts[name]->key().toString();
        }
        listWidget->addItem(it.key() + " (" + keybind + ")");
    }
    layout->addWidget(listWidget);

    QPushButton *btnDelete = new QPushButton("Delete Selected", &dialog);
    layout->addWidget(btnDelete);

    connect(btnDelete, &QPushButton::clicked, this, [this, listWidget]() {
        QListWidgetItem *item = listWidget->currentItem();
        if (item) {
            QString name = item->text().split(" (").first();
            auto reply = QMessageBox::question(this, "Confirm", "Delete macro: " + name + "?",
                                               QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // Delete from the manager's maps
                macroManager->macroLibrary.remove(name);
                if (macroManager->macroShortcuts.contains(name)) {
                    delete macroManager->macroShortcuts[name];
                    macroManager->macroShortcuts.remove(name);
                }

                QSettings settings("user", "1");
                QStringList names = settings.value("macros/library_names").toStringList();
                names.removeAll(name);
                settings.setValue("macros/library_names", names);
                settings.remove("macros/seq_" + name);
                settings.remove("macros/key_" + name);

                delete item;
            }
        }
    });
    dialog.exec();
}

void MainWindow::on_actionEdit_Colorscheme_triggered()
{
    QColor bgColor = QColorDialog::getColor(Qt::white, this, "Wybierz kolor tła");
    if (!bgColor.isValid()) return;

    QColor textColor = QColorDialog::getColor(Qt::black, this, "Wybierz kolor tekstu");
    QPalette p = ui->textEdit->palette();
    p.setColor(QPalette::Base, bgColor);
    p.setColor(QPalette::Text, textColor);
    ui->textEdit->setPalette(p);

    highlighter->setColors(Qt::blue, Qt::darkGreen);

    QSettings settings("user", "1");
    settings.setValue("colors/bg", bgColor.name());
    settings.setValue("colors/text", textColor.name());
}

void MainWindow::runSystemCommand() {
    bool ok;
    QString command = QInputDialog::getText(this, "System Call", "!", QLineEdit::Normal, "", &ok);

    if (ok && !command.isEmpty()) {
        QProcess process;
#ifdef Q_OS_WIN
        process.start("cmd", QStringList() << "/c" << command);
#else
        process.start("sh", QStringList() << "-c" << command);
#endif

        if (process.waitForFinished(5000)) {
            QString output = process.readAllStandardOutput();
            if (!output.isEmpty()) QMessageBox::information(this, "Output", output);
            else ui->statusbar->showMessage("Command executed.", 3000);
        }
    }
}

void MainWindow::on_actionCommand_triggered()
{
    runSystemCommand();
}

// delegate
void MainWindow::toggleMacroRecord() {
    macroManager->toggleRecord();
}

// delegate
void MainWindow::playMacro() {
    macroManager->playMacro();
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event) {
    if (obj == ui->textEdit && event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (macroManager->isRecording()){
            macroManager->recordEvent(keyEvent);
        }
    }
    return QMainWindow::eventFilter(obj, event);
}
