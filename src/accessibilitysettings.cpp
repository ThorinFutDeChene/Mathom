#include "accessibilitysettings.h"

#include "note.h"
#include "notecontent.h"

#include <QApplication>
#include <QBrush>
#include <QFont>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QObject>
#include <QRegularExpression>
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
#include <QVector>
#include <QWidget>

#include <KConfigGroup>
#include <KSharedConfig>


namespace
{

void enable(
    AccessibilityConfiguration &config,
    AccessibilityModule module)
{
    config.modules |= module;
}


struct SyllableRange
{
    int start = 0;
    int length = 0;
};


bool isFrenchVowel(QChar character)
{
    static const QString vowels =
        QStringLiteral(
            "aeiouy"
            "àâä"
            "éèêë"
            "îï"
            "ôö"
            "ùûü"
            "ÿ"
            "æœ");

    return vowels.contains(
        character.toLower());
}


bool isCommonFrenchOnset(
    const QString &cluster)
{
    static const QStringList onsets = {
        QStringLiteral("bl"),
        QStringLiteral("br"),
        QStringLiteral("cl"),
        QStringLiteral("cr"),
        QStringLiteral("dr"),
        QStringLiteral("fl"),
        QStringLiteral("fr"),
        QStringLiteral("gl"),
        QStringLiteral("gr"),
        QStringLiteral("pl"),
        QStringLiteral("pr"),
        QStringLiteral("tr"),
        QStringLiteral("vr"),
        QStringLiteral("ch"),
        QStringLiteral("ph"),
        QStringLiteral("th"),
        QStringLiteral("gn")
    };

    return onsets.contains(
        cluster.toLower());
}


/*
 * Segmentation orthographique francaise volontairement prudente.
 *
 * Ce moteur constitue une implementation Mathom independante.
 * Il ne reprend pas le code de LireCouleur.
 *
 * Regles principales :
 * - les voyelles consecutives forment un noyau ;
 * - une consonne entre deux noyaux rejoint la syllabe suivante ;
 * - les groupes courants bl/br/cl/cr/... restent ensemble ;
 * - les consonnes doubles sont separees.
 *
 * Le moteur sera enrichi progressivement pour traiter les cas
 * particuliers du francais, les lettres muettes et les phonemes.
 */
QVector<SyllableRange> syllableRanges(
    const QString &word)
{
    struct Nucleus
    {
        int start;
        int end;
    };

    QVector<Nucleus> nuclei;

    int position = 0;

    while (position < word.length()) {

        if (!isFrenchVowel(word.at(position))) {
            ++position;
            continue;
        }

        const int start = position;

        while (position < word.length()
               && isFrenchVowel(word.at(position))) {

            ++position;
        }

        nuclei.append(
            {start, position});
    }


    /*
     * Aucun ou un seul noyau vocalique :
     * le mot entier est considere comme une syllabe.
     */
    if (nuclei.size() <= 1) {
        return {
            {0, word.length()}
        };
    }


    QVector<int> boundaries;

    for (int index = 0;
         index < nuclei.size() - 1;
         ++index) {

        const int consonantStart =
            nuclei.at(index).end;

        const int consonantEnd =
            nuclei.at(index + 1).start;

        const int consonantCount =
            consonantEnd - consonantStart;

        int boundary =
            consonantStart;


        /*
         * Une seule consonne :
         *
         * ma-man
         * li-re
         */
        if (consonantCount == 1) {
            boundary =
                consonantStart;
        }


        /*
         * Plusieurs consonnes :
         * on conserve comme attaque de la syllabe suivante
         * les groupes consonantiques francais les plus usuels.
         *
         * ta-ble
         * a-près
         * ar-bre
         * a-chat
         */
        else if (consonantCount >= 2) {

            int onsetLength = 1;

            const QString lastTwo =
                word.mid(
                    consonantEnd - 2,
                    2);

            if (isCommonFrenchOnset(lastTwo))
                onsetLength = 2;

            boundary =
                consonantEnd - onsetLength;
        }


        if (boundary > 0
            && boundary < word.length()) {

            if (boundaries.isEmpty()
                || boundaries.last() != boundary) {

                boundaries.append(boundary);
            }
        }
    }


    QVector<SyllableRange> result;

    int start = 0;

    for (int boundary : boundaries) {

        if (boundary <= start)
            continue;

        result.append(
            {start, boundary - start});

        start = boundary;
    }

    if (start < word.length()) {
        result.append(
            {start, word.length() - start});
    }

    return result;
}


void applyDyslexiaPreset(
    AccessibilityConfiguration &config)
{
    /*
     * Modules deja operationnels dans le profil Dyslexie.
     *
     * Les futurs modules LireCouleur seront ajoutes ici
     * progressivement lorsqu'ils seront implementes.
     */
    enable(config, AccessibilityModule::AdaptedFont);
    enable(config, AccessibilityModule::LargerText);
    enable(config, AccessibilityModule::LetterSpacing);
    enable(config, AccessibilityModule::WordSpacing);
    enable(config, AccessibilityModule::LineSpacing);
}


void applyCustomModules(
    AccessibilityConfiguration &config)
{
    auto sharedConfig =
        KSharedConfig::openConfig();

    KConfigGroup custom(
        sharedConfig,
        QStringLiteral("Accessibility Custom Modules"));

    auto readModule =
        [&custom, &config](
            const QString &key,
            AccessibilityModule module)
    {
        if (custom.readEntry(key, false))
            enable(config, module);
    };

    readModule(
        QStringLiteral("adaptedFont"),
        AccessibilityModule::AdaptedFont);

    readModule(
        QStringLiteral("largerText"),
        AccessibilityModule::LargerText);

    readModule(
        QStringLiteral("letterSpacing"),
        AccessibilityModule::LetterSpacing);

    readModule(
        QStringLiteral("wordSpacing"),
        AccessibilityModule::WordSpacing);

    readModule(
        QStringLiteral("lineSpacing"),
        AccessibilityModule::LineSpacing);

    readModule(
        QStringLiteral("paragraphSpacing"),
        AccessibilityModule::ParagraphSpacing);

    readModule(
        QStringLiteral("syllableColoring"),
        AccessibilityModule::SyllableColoring);

    readModule(
        QStringLiteral("phonemeColoring"),
        AccessibilityModule::PhonemeColoring);

    readModule(
        QStringLiteral("graphemeHighlight"),
        AccessibilityModule::GraphemeHighlight);

    readModule(
        QStringLiteral("confusableLetters"),
        AccessibilityModule::ConfusableLetters);

    readModule(
        QStringLiteral("alternatingLines"),
        AccessibilityModule::AlternatingLines);

    readModule(
        QStringLiteral("readingGuide"),
        AccessibilityModule::ReadingGuide);

    readModule(
        QStringLiteral("activeLineHighlight"),
        AccessibilityModule::ActiveLineHighlight);

    readModule(
        QStringLiteral("dimOtherLines"),
        AccessibilityModule::DimOtherLines);

    readModule(
        QStringLiteral("textToSpeech"),
        AccessibilityModule::TextToSpeech);

    readModule(
        QStringLiteral("speechTracking"),
        AccessibilityModule::SpeechTracking);

    readModule(
        QStringLiteral("reducedDistractions"),
        AccessibilityModule::ReducedDistractions);

    readModule(
        QStringLiteral("largerControls"),
        AccessibilityModule::LargerControls);


    /*
     * Valeurs personnalisables.
     *
     * L'interface Personnalise les exposera ensuite.
     */
    config.fontFamily =
        custom.readEntry(
            QStringLiteral("fontFamily"),
            config.fontFamily);

    config.fontPointSize =
        custom.readEntry(
            QStringLiteral("fontPointSize"),
            config.fontPointSize);

    config.letterSpacingPercent =
        custom.readEntry(
            QStringLiteral("letterSpacingPercent"),
            config.letterSpacingPercent);

    config.wordSpacing =
        custom.readEntry(
            QStringLiteral("wordSpacing"),
            config.wordSpacing);

    config.lineSpacingPercent =
        custom.readEntry(
            QStringLiteral("lineSpacingPercent"),
            config.lineSpacingPercent);

    config.paragraphSpacing =
        custom.readEntry(
            QStringLiteral("paragraphSpacingValue"),
            config.paragraphSpacing);
}


QFont accessibleFont(
    const QFont &base,
    const AccessibilityConfiguration &config)
{
    QFont font(base);

    if (config.has(AccessibilityModule::AdaptedFont))
        font.setFamily(config.fontFamily);

    if (config.has(AccessibilityModule::LargerText))
        font.setPointSizeF(config.fontPointSize);

    if (config.has(AccessibilityModule::LetterSpacing)) {
        font.setLetterSpacing(
            QFont::PercentageSpacing,
            config.letterSpacingPercent);
    }

    if (config.has(AccessibilityModule::WordSpacing))
        font.setWordSpacing(config.wordSpacing);

    return font;
}


class AccessibilityHighlighter : public QSyntaxHighlighter
{
public:
    explicit AccessibilityHighlighter(
        QTextDocument *document)
        : QSyntaxHighlighter(document)
    {
        setObjectName(
            QStringLiteral(
                "mathomAccessibilityHighlighter"));
    }

    void setConfiguration(
        const AccessibilityConfiguration &configuration)
    {
        m_configuration = configuration;
        rehighlight();
    }

protected:
    void highlightBlock(
        const QString &text) override
    {
        if (text.isEmpty())
            return;

        QTextCharFormat baseFormat;
        bool hasBaseFormatting = false;


        /*
         * Modules typographiques.
         */
        if (m_configuration.has(
                AccessibilityModule::AdaptedFont)) {

            baseFormat.setFontFamilies(
                QStringList{
                    m_configuration.fontFamily});

            hasBaseFormatting = true;
        }

        if (m_configuration.has(
                AccessibilityModule::LargerText)) {

            baseFormat.setFontPointSize(
                m_configuration.fontPointSize);

            hasBaseFormatting = true;
        }

        if (m_configuration.has(
                AccessibilityModule::LetterSpacing)) {

            baseFormat.setFontLetterSpacing(
                m_configuration
                    .letterSpacingPercent);

            hasBaseFormatting = true;
        }

        if (m_configuration.has(
                AccessibilityModule::WordSpacing)) {

            baseFormat.setFontWordSpacing(
                m_configuration.wordSpacing);

            hasBaseFormatting = true;
        }


        if (hasBaseFormatting) {
            setFormat(
                0,
                text.length(),
                baseFormat);
        }


        /*
         * Coloration syllabique.
         *
         * Couche purement visuelle :
         * aucune couleur n'est enregistree dans le HTML du Mathom.
         */
        if (m_configuration.has(
                AccessibilityModule::SyllableColoring)) {

            static const QRegularExpression wordExpression(
                QStringLiteral("\\p{L}+"),
                QRegularExpression::
                    UseUnicodePropertiesOption);

            auto matches =
                wordExpression.globalMatch(text);

            while (matches.hasNext()) {

                const QRegularExpressionMatch match =
                    matches.next();

                const QString word =
                    match.captured();

                const QVector<SyllableRange> ranges =
                    syllableRanges(word);

                for (int index = 0;
                     index < ranges.size();
                     ++index) {

                    const SyllableRange &range =
                        ranges.at(index);

                    QTextCharFormat format =
                        baseFormat;

                    const QColor color =
                        (index % 2 == 0)
                        ? m_configuration.syllableColor1
                        : m_configuration.syllableColor2;

                    format.setForeground(
                        QBrush(color));

                    setFormat(
                        match.capturedStart()
                            + range.start,
                        range.length,
                        format);
                }
            }
        }
    }

private:
    AccessibilityConfiguration m_configuration;
};


AccessibilityHighlighter *accessibilityHighlighter(
    QTextDocument *document)
{
    if (!document)
        return nullptr;

    QObject *object =
        document->findChild<QObject *>(
            QStringLiteral(
                "mathomAccessibilityHighlighter"),
            Qt::FindDirectChildrenOnly);

    auto *highlighter =
        dynamic_cast<AccessibilityHighlighter *>(
            object);

    if (!highlighter) {
        highlighter =
            new AccessibilityHighlighter(
                document);
    }

    return highlighter;
}

} // namespace


AccessibilityConfiguration
AccessibilitySettings::effectiveConfiguration()
{
    AccessibilityConfiguration configuration;

    auto config =
        KSharedConfig::openConfig();

    KConfigGroup profiles(
        config,
        QStringLiteral("Accessibility Profiles"));

    /*
     * Les profils sont des PRESETS.
     *
     * Ils ne modifient jamais directement l'affichage.
     */
    if (profiles.readEntry(
            QStringLiteral("dyslexia"),
            false)) {

        applyDyslexiaPreset(configuration);
    }

    /*
     * Les autres profils seront raccordes a leurs presets
     * au fur et a mesure de leur implementation.
     *
     * Les cases peuvent deja coexister sans conflit.
     */

    if (profiles.readEntry(
            QStringLiteral("custom"),
            false)) {

        applyCustomModules(configuration);
    }

    return configuration;
}


void AccessibilitySettings::applyToTextEditor(
    QTextEdit *editor)
{
    if (!editor)
        return;

    if (!editor->property(
            "mathomAccessibilityEditor").toBool())
        return;

    const AccessibilityConfiguration configuration =
        effectiveConfiguration();

    /*
     * Editeur HTML :
     * couche de presentation uniquement.
     */
    if (editor->property(
            "mathomAccessibilityRichText").toBool()) {

        AccessibilityHighlighter *highlighter =
            accessibilityHighlighter(
                editor->document());

        if (highlighter) {
            highlighter->setConfiguration(
                configuration);
        }

        editor->viewport()->update();
        return;
    }

    /*
     * Editeur texte brut.
     */
    if (!editor->property(
            "mathomOriginalFont").isValid()) {

        editor->setProperty(
            "mathomOriginalFont",
            QVariant::fromValue(
                editor->font()));
    }

    const QFont originalFont =
        editor->property(
            "mathomOriginalFont").value<QFont>();

    const QSignalBlocker editorBlocker(editor);
    const QSignalBlocker documentBlocker(
        editor->document());

    editor->setFont(
        accessibleFont(
            originalFont,
            configuration));

    editor->document()->setDefaultFont(
        accessibleFont(
            originalFont,
            configuration));

    for (QTextBlock block =
             editor->document()->begin();
         block.isValid();
         block = block.next()) {

        QTextCursor cursor(block);

        QTextBlockFormat format =
            cursor.blockFormat();

        if (configuration.has(
                AccessibilityModule::LineSpacing)) {

            format.setLineHeight(
                configuration.lineSpacingPercent,
                QTextBlockFormat::ProportionalHeight);

        } else {

            format.setLineHeight(
                100.0,
                QTextBlockFormat::SingleHeight);
        }

        if (configuration.has(
                AccessibilityModule::ParagraphSpacing)) {

            format.setBottomMargin(
                configuration.paragraphSpacing);
        }

        cursor.setBlockFormat(format);
    }

    /*
     * Les modules visuels tels que la coloration syllabique
     * utilisent egalement le highlighter dans l'editeur texte brut.
     */
    AccessibilityHighlighter *highlighter =
        accessibilityHighlighter(
            editor->document());

    if (highlighter) {
        highlighter->setConfiguration(
            configuration);
    }

    editor->viewport()->update();
}


void AccessibilitySettings::applyToGraphicsItem(
    QGraphicsItem *item,
    bool requestRelayout)
{
    if (!item)
        return;

    auto *note =
        dynamic_cast<Note *>(
            item->parentItem());

    if (!note || !note->content())
        return;

    if (note->content()->graphicsItem() != item)
        return;

    const AccessibilityConfiguration configuration =
        effectiveConfiguration();

    /*
     * Mathom HTML hors edition.
     */
    if (auto *rich =
            dynamic_cast<QGraphicsTextItem *>(
                item)) {

        AccessibilityHighlighter *highlighter =
            accessibilityHighlighter(
                rich->document());

        if (highlighter) {
            highlighter->setConfiguration(
                configuration);
        }

        rich->update();

        if (requestRelayout)
            note->requestRelayout();

        return;
    }

    /*
     * Ancien Mathom texte brut.
     */
    if (auto *plain =
            dynamic_cast<QGraphicsSimpleTextItem *>(
                item)) {

        plain->setFont(
            accessibleFont(
                note->font(),
                configuration));

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
     * Editeurs actifs.
     */
    for (QWidget *widget : widgets) {

        auto *editor =
            qobject_cast<QTextEdit *>(
                widget);

        if (!editor)
            continue;

        if (!editor->property(
                "mathomAccessibilityEditor").toBool())
            continue;

        applyToTextEditor(editor);
    }

    /*
     * Mathoms affiches dans toutes les scenes ouvertes.
     */
    QSet<QGraphicsScene *> processedScenes;

    for (QWidget *widget : widgets) {

        auto *view =
            qobject_cast<QGraphicsView *>(
                widget);

        if (!view || !view->scene())
            continue;

        QGraphicsScene *scene =
            view->scene();

        if (processedScenes.contains(scene))
            continue;

        processedScenes.insert(scene);

        const auto items =
            scene->items();

        for (QGraphicsItem *item : items)
            applyToGraphicsItem(item);
    }
}
