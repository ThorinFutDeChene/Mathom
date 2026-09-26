/**
 * SPDX-FileCopyrightText: (C) 2003 Sébastien Laoût <slaout@linux62.org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef NEWBASKETDIALOG_H
#define NEWBASKETDIALOG_H

#include <QDialog>

#include <QMap>

class KIconButton;
class QLineEdit;
class KComboBox;
class QTreeWidgetItem;
class QPushButton;

class BasketScene;


/** The dialog to create a new basket from a template.
 * @author Sébastien Laoût
 */
class NewBasketDialog : public QDialog
{
    Q_OBJECT
public:
    NewBasketDialog(BasketScene *parentBasket, QWidget *parent = nullptr);
    ~NewBasketDialog() override;
protected Q_SLOTS:
    void slotOk();
    void returnPressed();
    void nameChanged(const QString &newName);

protected:
    bool event(QEvent *event) override;

private:
    int populateBasketsList(QTreeWidgetItem *item, int indent, int index);
    KIconButton *m_icon;
    QLineEdit *m_name;
    KComboBox *m_createIn;
    QMap<int, BasketScene *> m_basketsMap;
    QPushButton *okButton;
};

#endif // NEWBASKETDIALOG_H
