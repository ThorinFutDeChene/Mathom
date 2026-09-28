#include "accessibilitysettings.h"

#include "note.h"
#include "notecontent.h"

#include <QApplication>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QObject>
#include <QSet>
#include <QSignalBlocker>
#include <QStringList>
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

QFont dyslexiaFont(const QFont &base)
{
    QFont font(base);

    font.setFamily(QStringLiteral("Noto Sans"));
    font.setPointSizeF(14.0);
    font.setLetterSpacing(
        QFont::PercentageSpacing,
        110.0);
    font.setWordSpacing(3.0);

    return font;
}


class AccessibilityHighlighter : public QSyntaxHighlighter
{
public:
    explicit AccessibilityHighlighter(QTextDocument *document)
        : QSyntaxHighlighter(document)
    {
        setObjectName(
            QStringLiteral("mathomAccessibilityHighlighter"));
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


AccessibilityHighlighter *accessibilityHighlighter(
    QTextDocument *document)
{
    if (!document)
        return nullptr;

    QObject *object =
        document->findChild<QObject *>(
            QStringLiteral("mathomAccessibilityHighlighter"),
            Qt::FindDirectChildrenOnly);

    auto *highlighter =
        dynamic_cast<AccessibilityHighlighter *>(object);

    if (!highlighter)
        highlighter =
            new AccessibilityHighlighter(document);

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


void AccessibilitySettings::applyToTextEditor(
    QTextEdit *editor)
{
    if (!editor)
        return;

    if (!editor->property(
            "mathomAccessibilityEditor").toBool())
        return;

    const bool dyslexia = dyslexiaEnabled();

    /*
     * Editeur HTML :
     * couche visuelle temporaire uniquement.
     */
    if (editor->property(
            "mathomAccessibilityRichText").toBool()) {

        AccessibilityHighlighter *highlighter =
            accessibilityHighlighter(
                editor->document());

        if (highlighter)
            highlighter->setDyslexiaEnabled(dyslexia);

        editor->viewport()->update();
        return;
    }

    /*
     * Editeur texte brut.
     */
    if (!editor->property("mathomOriginalFont").isValid()) {
        editor->setProperty(
            "mathomOriginalFont",
            QVariant::fromValue(editor->font()));
    }

    const QFont originalFont =
        editor->property(
            "mathomOriginalFont").value<QFont>();

    const QSignalBlocker editorBlocker(editor);
    const QSignalBlocker documentBlocker(
        editor->document());

    QFont font = originalFont;

    if (dyslexia)
        font = dyslexiaFont(originalFont);

    editor->setFont(font);
    editor->document()->setDefaultFont(font);

    for (QTextBlock block =
             editor->document()->begin();
         block.isValid();
         block = block.next()) {

        QTextCursor cursor(block);
        QTextBlockFormat format =
            cursor.blockFormat();

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


void AccessibilitySettings::applyToGraphicsItem(
    QGraphicsItem *item,
    bool requestRelayout)
{
    if (!item)
        return;

    /*
     * Les contenus texte Mathom sont des enfants directs
     * de l'objet Note.
     */
    auto *note =
        dynamic_cast<Note *>(item->parentItem());

    if (!note || !note->content())
        return;

    /*
     * Evite de modifier d'autres objets graphiques appartenant
     * eventuellement a la Note.
     */
    if (note->content()->graphicsItem() != item)
        return;

    const bool dyslexia = dyslexiaEnabled();

    /*
     * Mathom HTML affiche hors edition :
     * QGraphicsTextItem + QTextDocument.
     *
     * QSyntaxHighlighter agit uniquement sur le rendu :
     * le HTML enregistre reste intact.
     */
    if (auto *rich =
            dynamic_cast<QGraphicsTextItem *>(item)) {

        AccessibilityHighlighter *highlighter =
            accessibilityHighlighter(
                rich->document());

        if (highlighter)
            highlighter->setDyslexiaEnabled(dyslexia);

        rich->update();

        if (requestRelayout)
            note->requestRelayout();

        return;
    }

    /*
     * Anciennes notes texte brut :
     * QGraphicsSimpleTextItem.
     */
    if (auto *plain =
            dynamic_cast<QGraphicsSimpleTextItem *>(item)) {

        QFont font = note->font();

        if (dyslexia)
            font = dyslexiaFont(font);

        plain->setFont(font);
        plain->update();

        if (requestRelayout)
            note->requestRelayout();
    }
}


void AccessibilitySettings::refreshAllDisplays()
{
    const auto widgets =
        QApplication::allWidgets();

    /*
     * 1. Editeurs actuellement ouverts.
     */
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

    /*
     * 2. Tous les Mathoms affiches dans les scenes.
     *
     * C'est cette partie qui rend le profil permanent
     * quand aucune ligne n'est en edition.
     */
    QSet<QGraphicsScene *> processedScenes;

    for (QWidget *widget : widgets) {

        auto *view =
            qobject_cast<QGraphicsView *>(widget);

        if (!view || !view->scene())
            continue;

        QGraphicsScene *scene = view->scene();

        if (processedScenes.contains(scene))
            continue;

        processedScenes.insert(scene);

        const auto items = scene->items();

        for (QGraphicsItem *item : items)
            applyToGraphicsItem(item);
    }
}
