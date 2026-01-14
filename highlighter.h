#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>
#include <QVector>

class Highlighter : public QSyntaxHighlighter {
    Q_OBJECT
public:
    explicit Highlighter(QTextDocument *parent = nullptr);

    // Updated to handle all colors
    void setColors(QColor keywordColor, QColor commentColor,
                   QColor stringColor = QColor(230, 219, 116),
                   QColor typeColor = QColor(102, 217, 239));

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> rules;

    // Store the current colors
    QColor m_keywordColor;
    QColor m_commentColor;
    QColor m_stringColor;
    QColor m_typeColor;

    void setupRules(); // New method to build the rules list

    QStringList keywords = {
#include "keywords.inc"
    };
};

#endif
