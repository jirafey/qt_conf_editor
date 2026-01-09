#include "highlighter.h"

Highlighter::Highlighter(QTextDocument *parent) : QSyntaxHighlighter(parent){
    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(Qt::blue);

    for (const QString &word : std::as_const(keywords)){
        HighlightRule rule;
        rule.pattern = QRegularExpression("\\b" + word + "\\b");
        rule.format = keywordFormat;
        rules.append(rule);
    }

    commentFormat.setFontItalic(true);
    commentFormat.setForeground(Qt::darkGreen);

    HighlightRule commentRule;
    commentRule.pattern = QRegularExpression("//[^\n]*");
    commentRule.format = commentFormat;
    rules.append(commentRule);
}

void Highlighter::setColors(QColor keywordColor, QColor commentColor) {
    keywordFormat.setForeground(keywordColor);
    commentFormat.setForeground(commentColor); // POPRAWIONO: wcześniej było keywordFormat

    for(int i=0; i<rules.size(); i++){
        // Jeśli reguła zawiera slashe, przypisz format komentarza
        if (rules[i].pattern.pattern().contains("//")){
            rules[i].format = commentFormat;
        }
        else{
            rules[i].format = keywordFormat;
        }
    }
    rehighlight(); // Odśwież widok
}

void Highlighter::highlightBlock(const QString &text){
    for(const auto &rule : std::as_const(rules)){
        QRegularExpressionMatchIterator it = rule.pattern.globalMatch(text);
        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}
