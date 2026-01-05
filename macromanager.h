#ifndef MACROMANAGER_H
#define MACROMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QTextEdit>
#include <QKeyEvent>
#include <QShortcut>

struct MacroEvent {
    int key;
    Qt::KeyboardModifiers modifiers;
    QString text;
};

class MacroManager : public QObject {
    Q_OBJECT
public:
    explicit MacroManager(QTextEdit *editor, QObject *parent = nullptr);

    void toggleRecord();
    void playMacro();
    void recordEvent(QKeyEvent *event);
    void loadMacroLibrary();
    bool isRecording() const { return recording; }

    // Moved to public so MainWindow can access them for the Management Dialog
    QMap<QString, QList<MacroEvent>> macroLibrary;
    QMap<QString, QShortcut*> macroShortcuts;

private:
    void playNamedMacro(const QString &name);
    void simulateKey(int key, Qt::KeyboardModifiers mods, const QString &text);
    QString macroToString(const QList<MacroEvent> &buffer);

    QTextEdit *m_editor;
    bool recording = false;
    QList<MacroEvent> macroBuffer;
};

#endif
