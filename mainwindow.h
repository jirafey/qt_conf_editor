#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QShortcut>
#include "highlighter.h"
#include "macromanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // UI Action Triggers
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSave_as_triggered();
    void on_actionCut_triggered();
    void on_actionCopy_triggered();
    void on_actionPaste_triggered();
    void on_actionRedo_triggered();
    void on_actionUndo_triggered();
    void on_actionSearch_triggered();
    void on_actionEditMacros_triggered();
    void on_actionEditKeybinds_triggered();
    void on_actionThis_Text_Editor_triggered();
    void on_actionEdit_Colorscheme_triggered();
    void on_actionCommand_triggered();

    // Logic Slots for Shortcuts
    void copy_keybind();
    void findNextMatch();
    void findPrevMatch();
    void quitSearch();
    void toggleMacroRecord(); // Delegates to macroManager
    void playMacro();         // Delegates to macroManager

private:
    Ui::MainWindow *ui;
    Highlighter *highlighter;
    MacroManager *macroManager; // Pointer to our helper class
    QString file_path;

    // Search state
    QList<QTextEdit::ExtraSelection> currentSelections;
    int currentMatchIndex = -1;

    // Shortcut pointers
    QShortcut *enterShortcut, *shiftEnterShortcut, *quitSearchShortcut;
    QShortcut *shortcut_copy, *shortcut_copy_and_select_line;
    QShortcut *macroRecordShortcut, *macroPlayShortcut, *systemCommandShortcut;

    // Helpers
    void copy_and_select_line();
    void onChangeShortcutClicked();
    void saveShortcut(QString name, QKeySequence seq);
    QList<QTextEdit::ExtraSelection> highlightMatches(const QString &text);
    void updateSearchFocus();
    void runSystemCommand();
};
#endif
