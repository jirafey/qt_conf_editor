#include "macromanager.h"
#include <QCoreApplication>
#include <QSettings>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeySequenceEdit>
#include <QPushButton>
#include <QDialog>

MacroManager::MacroManager(QTextEdit *editor, QObject *parent)
    : QObject(parent), m_editor(editor) {
    loadMacroLibrary();
}

void MacroManager::simulateKey(int key, Qt::KeyboardModifiers mods, const QString &text) {
    QKeyEvent press(QEvent::KeyPress, key, mods, text);
    QCoreApplication::sendEvent(m_editor->viewport(), &press);
    QKeyEvent release(QEvent::KeyRelease, key, mods, text);
    QCoreApplication::sendEvent(m_editor->viewport(), &release);
}

QString MacroManager::macroToString(const QList<MacroEvent> &buffer) {
    QStringList parts;
    for(const auto &e : buffer) {
        parts << QString("%1|%2|%3").arg(e.key).arg((int)e.modifiers).arg(e.text);
    }
    return parts.join(";");
}

void MacroManager::toggleRecord() {
    if (!recording) {
        macroBuffer.clear();
        recording = true;
    } else {
        recording = false;
        bool ok;
        QString name = QInputDialog::getText(m_editor->window(), "Save Macro", "Macro Name:", QLineEdit::Normal, "", &ok);

        if (ok && !name.isEmpty()) {
            QDialog keyDialog(m_editor->window());
            keyDialog.setWindowTitle("Assign Key");
            QVBoxLayout *lay = new QVBoxLayout(&keyDialog);
            QKeySequenceEdit *keyEdit = new QKeySequenceEdit(&keyDialog);
            lay->addWidget(new QLabel("Press shortcut:"));
            lay->addWidget(keyEdit);
            QPushButton *btn = new QPushButton("Save", &keyDialog);
            lay->addWidget(btn);
            connect(btn, &QPushButton::clicked, &keyDialog, &QDialog::accept);

            if (keyDialog.exec() == QDialog::Accepted) {
                QKeySequence ks = keyEdit->keySequence();
                macroLibrary[name] = macroBuffer;

                QShortcut *s = new QShortcut(ks, m_editor->window());
                connect(s, &QShortcut::activated, this, [this, name](){ playNamedMacro(name); });
                macroShortcuts[name] = s;

                QSettings settings("user", "1");
                QStringList names = settings.value("macros/library_names").toStringList();
                if (!names.contains(name)) names.append(name);
                settings.setValue("macros/library_names", names);
                settings.setValue("macros/seq_" + name, macroToString(macroBuffer));
                settings.setValue("macros/key_" + name, ks.toString());
            }
        }
    }
}

void MacroManager::playMacro() {
    if (recording || macroBuffer.isEmpty()) return;
    for (const auto &e : macroBuffer) simulateKey(e.key, e.modifiers, e.text);
}

void MacroManager::recordEvent(QKeyEvent *event) {
    if (!recording) return;
    macroBuffer.append({event->key(), event->modifiers(), event->text()});
}

void MacroManager::loadMacroLibrary() {
    QSettings settings("user", "1");
    QStringList names = settings.value("macros/library_names").toStringList();
    for (const QString &name : names) {
        QString seq = settings.value("macros/seq_" + name).toString();
        QList<MacroEvent> buffer;
        for (const QString &item : seq.split(";", Qt::SkipEmptyParts)) {
            QStringList d = item.split("|");
            if (d.size() >= 3) buffer.append({d[0].toInt(), (Qt::KeyboardModifiers)d[1].toInt(), d[2]});
        }
        macroLibrary[name] = buffer;
        QShortcut *s = new QShortcut(QKeySequence(settings.value("macros/key_"+name).toString()), m_editor->window());
        connect(s, &QShortcut::activated, this, [this, name](){ playNamedMacro(name); });
    }
}

void MacroManager::playNamedMacro(const QString &name) {
    if (!recording && macroLibrary.contains(name)) {
        for (const auto &e : macroLibrary[name]) simulateKey(e.key, e.modifiers, e.text);
    }
}
