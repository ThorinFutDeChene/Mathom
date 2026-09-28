#ifndef ACCESSIBILITYSETTINGS_H
#define ACCESSIBILITYSETTINGS_H

#include "basket_export.h"

class QGraphicsItem;
class QTextEdit;

class BASKET_EXPORT AccessibilitySettings
{
public:
    static bool dyslexiaEnabled();

    // Editeur actif
    static void applyToTextEditor(QTextEdit *editor);

    // Affichage permanent des Mathoms dans la scene
    static void applyToGraphicsItem(
        QGraphicsItem *item,
        bool requestRelayout = true);

    // Actualise editeurs + Mathoms affiches
    static void refreshAllDisplays();
};

#endif
