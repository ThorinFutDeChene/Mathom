#ifndef ACCESSIBILITYSETTINGS_H
#define ACCESSIBILITYSETTINGS_H

#include "basket_export.h"

#include <QColor>
#include <QFlags>
#include <QString>
#include <QtGlobal>

class QGraphicsItem;
class QTextEdit;

/*
 * Catalogue central des modules d'accessibilite.
 *
 * Les profils DYS/TDAH ne doivent jamais agir directement
 * sur le rendu. Ils activent une combinaison de ces modules.
 */
enum class AccessibilityModule : quint32
{
    None                 = 0x00000000,

    AdaptedFont          = 0x00000001,
    LargerText           = 0x00000002,
    LetterSpacing        = 0x00000004,
    WordSpacing          = 0x00000008,
    LineSpacing          = 0x00000010,
    ParagraphSpacing     = 0x00000020,

    SyllableColoring     = 0x00000040,
    PhonemeColoring      = 0x00000080,
    GraphemeHighlight    = 0x00000100,
    ConfusableLetters    = 0x00000200,

    AlternatingLines     = 0x00000400,
    ReadingGuide         = 0x00000800,
    ActiveLineHighlight  = 0x00001000,
    DimOtherLines        = 0x00002000,

    TextToSpeech         = 0x00004000,
    SpeechTracking       = 0x00008000,

    ReducedDistractions  = 0x00010000,
    LargerControls       = 0x00020000
};

Q_DECLARE_FLAGS(AccessibilityModules, AccessibilityModule)
Q_DECLARE_OPERATORS_FOR_FLAGS(AccessibilityModules)


/*
 * Configuration finale appliquee par Mathom.
 *
 * Elle ne sait pas si l'utilisateur est dyslexique, TDAH, etc.
 * Elle sait uniquement quels modules doivent etre actifs.
 */
struct BASKET_EXPORT AccessibilityConfiguration
{
    AccessibilityModules modules;

    QString fontFamily = QStringLiteral("OpenDyslexic");

    qreal fontPointSize = 14.0;
    qreal letterSpacingPercent = 110.0;
    qreal wordSpacing = 3.0;
    qreal lineSpacingPercent = 150.0;
    qreal paragraphSpacing = 0.0;

    /*
     * Coloration syllabique.
     * Bleu / rouge par defaut, modifiable plus tard
     * depuis le profil personnalise.
     */
    QColor syllableColor1 = QColor(QStringLiteral("#005BBB"));
    QColor syllableColor2 = QColor(QStringLiteral("#C00040"));

    bool has(AccessibilityModule module) const
    {
        return modules.testFlag(module);
    }
};


class BASKET_EXPORT AccessibilitySettings
{
public:
    /*
     * Fusionne :
     * - les profils selectionnes ;
     * - les modules du mode Personnalise.
     */
    static AccessibilityConfiguration effectiveConfiguration();

    static void applyToTextEditor(QTextEdit *editor);

    static void applyToGraphicsItem(
        QGraphicsItem *item,
        bool requestRelayout = true);

    static void refreshAllDisplays();
};

#endif
