
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
    void setColors(QColor keywordColor, QColor commentColor);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    QVector<HighlightRule> rules;
    QTextCharFormat keywordFormat;
    QTextCharFormat commentFormat;
    QStringList keywords = {"void", "int", "float", "double",
                            "long", "bool", "char", "char8_t",
                            "char16_t", "char32_t", "short", "signed",
                            "unsigned", "wchar_t", "if", "else",
                            "switch", "case", "default", "for",
                            "while", "do", "break", "continue",
                            "goto", "true", "false", "nullptr",
                            "new", "delete","alignas",
                            "alignof", "class", "struct", "union",
                            "enum", "friend","mutable", "this",
                            "public", "private", "protected",
                            "inline","explicit","virtual","override",
                            "final", "constexpr",
                            "consteval", "constinit", "operator",
                            "typedef", "using", "typename", "template",
                            "concept", "requires", "try", "catch",
                            "throw", "noexcept", "const_cast",
                            "dynamic_cast", "reinterpret_cast",
                            "static_cast", "decltype", "typeid",
                            "const", "static", "static_assert",
                            "extern", "register", "thread_local",
                            "volatile", "export", "namespace",
                            "co_await", "co_return", "co_yield",
                            "and", "and_eq", "or", "or_eq",
                            "not_eq", "bitand", "bitor", "compl",
                            "xor", "xor_eq", "asm", "auto", "return",
                            "sizeof", "contract_assert", "pre", "post",
                            "import", "module", "atomic_cancel",
                            "atomic_commit", "atomic_noexcept","not",
                            "synchronized", "__has_embed", "#embed",
                            "reflexpr", "_Pragma", "defined",
                            "__has_include"};
};

#endif
