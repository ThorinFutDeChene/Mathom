/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef PAGEPROPERTIESDIALOG_H
#define PAGEPROPERTIESDIALOG_H

#include <QDialog>
#include <QMap>

class BasketScene;
class QComboBox;
class QRadioButton;
class QSpinBox;
class KColorCombo2;

class PagePropertiesDialog final : public QDialog
{
public:
    explicit PagePropertiesDialog(
        BasketScene *basket,
        QWidget *parent = nullptr);

private:
    void applyChanges();

    BasketScene *m_basket = nullptr;

    QComboBox *m_backgroundImage = nullptr;
    KColorCombo2 *m_backgroundColor = nullptr;
    KColorCombo2 *m_textColor = nullptr;

    QRadioButton *m_columnForm = nullptr;
    QRadioButton *m_freeForm = nullptr;
    QSpinBox *m_columnCount = nullptr;

    QMap<int, QString> m_backgroundImagesMap;
};

#endif
