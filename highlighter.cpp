#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent) {
    m_keywordColor = QColor(249, 38, 114); // Pink
    m_commentColor = QColor(117, 113, 94); // Grey-Olive
    m_stringColor = QColor(230, 219, 116);  // Yellow
    m_typeColor = QColor(102, 217, 239);    // Cyan

    setupRules();
}

void Highlighter::setupRules() {
    rules.clear();
    HighlightRule rule;

    QTextCharFormat keywordFormat;
    keywordFormat.setForeground(m_keywordColor);
    keywordFormat.setFontWeight(QFont::Bold);
    for (const QString &word : keywords) {
        rule.pattern = QRegularExpression("\\b" + word + "\\b");
        rule.format = keywordFormat;
        rules.append(rule);
    }

    // Types/Classes
    QTextCharFormat typeFormat;
    typeFormat.setForeground(m_typeColor);
    typeFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("\\b[A-Z][A-Za-z0-9_]*\\b");
    rule.format = typeFormat;
    rules.append(rule);

    QTextCharFormat functionFormat;
    functionFormat.setForeground(QColor(166, 226, 46)); // Neon Green
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\s*\\()");
    rule.format = functionFormat;
    rules.append(rule);

    QTextCharFormat numberFormat;
    numberFormat.setForeground(QColor(174, 129, 255)); // Purple
    rule.pattern = QRegularExpression("\\b\\d+\\b");
    rule.format = numberFormat;
    rules.append(rule);

    QTextCharFormat stringFormat;
    stringFormat.setForeground(m_stringColor);
    rule.pattern = QRegularExpression("\".*?\"");
    rule.format = stringFormat;
    rules.append(rule);

    QTextCharFormat commentFormat;
    commentFormat.setForeground(m_commentColor);
    commentFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("//[^\n]*");
    rule.format = commentFormat;
    rules.append(rule);
}

void Highlighter::setColors(QColor keywordColor, QColor commentColor, QColor stringColor, QColor typeColor) {
    m_keywordColor = keywordColor;
    m_commentColor = commentColor;
    m_stringColor = stringColor;
    m_typeColor = typeColor;

    setupRules();
    rehighlight();
}

void Highlighter::highlightBlock(const QString &text) {
    for (const auto &rule : std::as_const(rules)) {
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
