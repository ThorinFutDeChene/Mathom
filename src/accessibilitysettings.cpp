#include "accessibilitysettings.h"

#include <QApplication>
#include <QFont>
#include <QObject>
#include <QStringList>
#include <QSignalBlocker>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextBlockFormat>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QTextDocument>
#include <QTextEdit>
#include <QVariant>
#include <QWidget>

#include <KConfigGroup>
#include <KSharedConfig>

namespace
{

class AccessibilityHighlighter : public QSyntaxHighlighter
{
public:
    explicit AccessibilityHighlighter(QTextDocument *document)
        : QSyntaxHighlighter(document)
    {
        setObjectName(QStringLiteral("mathomAccessibilityHighlighter"));
    }

    void setDyslexiaEnabled(bool enabled)
    {
        m_dyslexiaEnabled = enabled;
        rehighlight();
    }

protected:
    void highlightBlock(const QString &text) override
    {
        if (!m_dyslexiaEnabled || text.isEmpty())
            return;

        /*
         * Visual overlay only.
         *
         * QSyntaxHighlighter formats are not written into the Mathom HTML.
         * Existing bold/italic/colour information therefore remains intact.
         */
        QTextCharFormat format;

        format.setFontFamilies(
            QStringList{QStringLiteral("Noto Sans")});

        format.setFontPointSize(14.0);
        format.setFontLetterSpacing(110.0);
        format.setFontWordSpacing(3.0);

        setFormat(0, text.length(), format);
    }

private:
    bool m_dyslexiaEnabled = false;
};

AccessibilityHighlighter *accessibilityHighlighter(QTextEdit *editor)
{
    QTextDocument *document = editor->document();

    QObject *object =
        document->findChild<QObject *>(
            QStringLiteral("mathomAccessibilityHighlighter"),
            Qt::FindDirectChildrenOnly);

    auto *highlighter =
        dynamic_cast<AccessibilityHighlighter *>(object);

    if (!highlighter)
        highlighter = new AccessibilityHighlighter(document);

    return highlighter;
}

} // namespace


bool AccessibilitySettings::dyslexiaEnabled()
{
    KConfigGroup group(
        KSharedConfig::openConfig(),
        QStringLiteral("Accessibility Profiles"));

    return group.readEntry(
        QStringLiteral("dyslexia"),
        false);
}


void AccessibilitySettings::applyToTextEditor(QTextEdit *editor)
{
    if (!editor)
        return;

    if (!editor->property("mathomAccessibilityEditor").toBool())
        return;

    const bool dyslexia = dyslexiaEnabled();

    /*
     * Rich-text Mathoms:
     *
     * Never alter the real QTextDocument formatting.
     * A QSyntaxHighlighter supplies a display-only accessibility layer.
     */
    if (editor->property("mathomAccessibilityRichText").toBool()) {

        accessibilityHighlighter(editor)
            ->setDyslexiaEnabled(dyslexia);

        editor->viewport()->update();
        return;
    }

    /*
     * Plain-text Mathoms:
     *
     * Their saved content contains no rich formatting, so their display
     * font and block spacing can safely be adapted directly.
     */

    if (!editor->property("mathomOriginalFont").isValid()) {
        editor->setProperty(
            "mathomOriginalFont",
            QVariant::fromValue(editor->font()));
    }

    const QFont originalFont =
        editor->property("mathomOriginalFont").value<QFont>();

    const QSignalBlocker editorBlocker(editor);
    const QSignalBlocker documentBlocker(editor->document());

    QFont font = originalFont;

    if (dyslexia) {
        font.setFamily(QStringLiteral("Noto Sans"));
        font.setPointSizeF(14.0);
        font.setLetterSpacing(
            QFont::PercentageSpacing,
            110.0);
        font.setWordSpacing(3.0);
    }

    editor->setFont(font);
    editor->document()->setDefaultFont(font);

    for (QTextBlock block = editor->document()->begin();
         block.isValid();
         block = block.next()) {

        QTextCursor cursor(block);
        QTextBlockFormat format = cursor.blockFormat();

        if (dyslexia) {
            format.setLineHeight(
                150.0,
                QTextBlockFormat::ProportionalHeight);
        } else {
            format.setLineHeight(
                100.0,
                QTextBlockFormat::SingleHeight);
        }

        cursor.setBlockFormat(format);
    }

    editor->viewport()->update();
}


void AccessibilitySettings::refreshOpenEditors()
{
    const auto widgets = QApplication::allWidgets();

    for (QWidget *widget : widgets) {

        auto *editor =
            qobject_cast<QTextEdit *>(widget);

        if (!editor)
            continue;

        if (!editor->property(
                "mathomAccessibilityEditor").toBool())
            continue;

        applyToTextEditor(editor);
    }
}
