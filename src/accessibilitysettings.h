#ifndef ACCESSIBILITYSETTINGS_H
#define ACCESSIBILITYSETTINGS_H

#include "basket_export.h"

class QTextEdit;

class BASKET_EXPORT AccessibilitySettings
{
public:
    static bool dyslexiaEnabled();
    static void applyToTextEditor(QTextEdit *editor);
    static void refreshOpenEditors();
};

#endif
