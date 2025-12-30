#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QList>
#include <QShortcut>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    // void highlightMatches(const QString &text);    void on_actionNew_triggered();
    void on_actionNew_triggered();

    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_as_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();    
    void on_actionPaste_triggered();
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
    void on_actionThis_Text_Editor_triggered();

    void on_actionSearch_triggered();

    void on_actionSearch_Replace_triggered();

    void on_actionEditMacros_triggered();

    void on_actionEditKeybinds_triggered();

private:
    // kebinds
    void copy_keybind();
    void find_keybind();
    void copy_and_select_line();
    void quitSearch();

    QList<QTextEdit::ExtraSelection> highlightMatches(const QString &text);

    Ui::MainWindow *ui;
    QString file_path;
    QList<QTextEdit::ExtraSelection> currentSelections;
    int currentMatchIndex =-1;
    QShortcut *enterShortcut;
    QShortcut *shiftEnterShortcut;
    QShortcut *quitSearchShortcut;
    QShortcut *shortcut_copy;
    QShortcut *shortcut_copy_and_select_line;

    void updateSearchFocus();
    void onChangeShortcutClicked();
    void saveShortcut(QString name, QKeySequence seq);
    void findNextMatch();
    void findPrevMatch();
    QString macroToString();
    void loadMacroFromString(const QString &str);
    // macros
    struct MacroEvent {
        int key;
        Qt::KeyboardModifiers modifiers;
        QString text;
    };

    QList<MacroEvent> macroBuffer;
    bool isRecording =false;

    QShortcut *macroRecordShortcut;
    QShortcut *macroPlayShortcut;

    void toggleMacroRecord();
    void playMacro();
    void simulateKey(int key, Qt::KeyboardModifiers mods, const QString &text);
    bool eventFilter(QObject *obj, QEvent *event) override;

    // macros library
    QMap<QString, QList<MacroEvent>> macroLibrary;
    QMap<QString, QShortcut*> macroShortcuts;

    void loadMacroLibrary();
    void addMacroToLibrary(QString name, QKeySequence ks, QList<MacroEvent> buffer);
    void playNamedMacro(QString name);


};
#endif // MAINWINDOW_H
