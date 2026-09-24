/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QColor>
#include <QList>
#include <QString>
#include <QWidget>

class BasketScene;
class QComboBox;
class QHBoxLayout;
class QScrollArea;
class QWidget;

class MathomNavigationBar final : public QWidget
{
    Q_OBJECT

public:
    struct Entry {
        QString title;
        BasketScene *basket = nullptr;
        QColor color;
    };

    explicit MathomNavigationBar(QWidget *parent = nullptr);

    static QColor automaticColor(
        const QList<QColor> &usedColors);

    void setMathomHouses(
        const QList<Entry> &houses,
        BasketScene *currentHouse);

    void setTabs(
        const QList<Entry> &tabs,
        BasketScene *activeBasket);

    void setBreadcrumb(
        const QList<Entry> &path);

Q_SIGNALS:
    void navigateRequested(BasketScene *basket);

private:
    static int hueDistance(int first, int second);

    void clearLayout(QHBoxLayout *layout);

    QComboBox *m_houseCombo = nullptr;

    QScrollArea *m_tabsScrollArea = nullptr;
    QWidget *m_tabsWidget = nullptr;
    QHBoxLayout *m_tabsLayout = nullptr;

    QWidget *m_breadcrumbWidget = nullptr;
    QHBoxLayout *m_breadcrumbLayout = nullptr;
};
