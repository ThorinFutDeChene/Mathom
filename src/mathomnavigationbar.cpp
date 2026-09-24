/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mathomnavigationbar.h"

#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QToolButton>
#include <QVBoxLayout>

#include <limits>

#include "basketscene.h"

MathomNavigationBar::MathomNavigationBar(QWidget *parent)
    : QWidget(parent)
{
    auto *mainLayout = new QVBoxLayout(this);

    mainLayout->setContentsMargins(6, 4, 6, 0);
    mainLayout->setSpacing(3);

    // --------------------------------------------------------
    // Ligne 1 : sélection de la Mathom-House + fil d'Ariane
    // --------------------------------------------------------

    auto *contextLayout = new QHBoxLayout();

    contextLayout->setContentsMargins(0, 0, 0, 0);
    contextLayout->setSpacing(8);

    m_houseCombo = new QComboBox(this);
    m_houseCombo->setMinimumWidth(190);
    m_houseCombo->setMaximumWidth(320);
    m_houseCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    contextLayout->addWidget(m_houseCombo);

    m_breadcrumbWidget = new QWidget(this);

    m_breadcrumbLayout = new QHBoxLayout(m_breadcrumbWidget);
    m_breadcrumbLayout->setContentsMargins(2, 0, 0, 0);
    m_breadcrumbLayout->setSpacing(3);
    m_breadcrumbLayout->addStretch();

    contextLayout->addWidget(m_breadcrumbWidget, 1);

    mainLayout->addLayout(contextLayout);

    // --------------------------------------------------------
    // Ligne 2 : intercalaires
    // --------------------------------------------------------

    m_tabsScrollArea = new QScrollArea(this);
    m_tabsScrollArea->setFrameShape(QFrame::NoFrame);
    m_tabsScrollArea->setWidgetResizable(true);
    m_tabsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tabsScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_tabsScrollArea->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Fixed);

    m_tabsWidget = new QWidget(m_tabsScrollArea);

    m_tabsLayout = new QHBoxLayout(m_tabsWidget);
    m_tabsLayout->setContentsMargins(0, 0, 0, 0);
    m_tabsLayout->setSpacing(2);
    m_tabsLayout->addStretch();

    m_tabsScrollArea->setWidget(m_tabsWidget);

    mainLayout->addWidget(m_tabsScrollArea);

    connect(
        m_houseCombo,
        &QComboBox::currentIndexChanged,
        this,
        [this](int index) {
            if (index < 0)
                return;

            const quintptr pointer =
                static_cast<quintptr>(
                    m_houseCombo->itemData(index).toULongLong());

            if (pointer == 0)
                return;

            Q_EMIT navigateRequested(
                reinterpret_cast<BasketScene *>(pointer));
        });
}

void MathomNavigationBar::clearLayout(QHBoxLayout *layout)
{
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);

        if (item->widget())
            delete item->widget();

        delete item;
    }
}

void MathomNavigationBar::setMathomHouses(
    const QList<Entry> &houses,
    BasketScene *currentHouse)
{
    const QSignalBlocker blocker(m_houseCombo);

    m_houseCombo->clear();

    int currentIndex = -1;

    for (int i = 0; i < houses.size(); ++i) {
        const Entry &entry = houses.at(i);

        const quintptr pointer =
            reinterpret_cast<quintptr>(entry.basket);

        m_houseCombo->addItem(
            entry.title,
            QVariant::fromValue<qulonglong>(
                static_cast<qulonglong>(pointer)));

        if (entry.basket == currentHouse)
            currentIndex = i;
    }

    if (currentIndex >= 0)
        m_houseCombo->setCurrentIndex(currentIndex);
}

int MathomNavigationBar::hueDistance(int first, int second)
{
    const int direct = qAbs(first - second);

    return qMin(direct, 360 - direct);
}

QColor MathomNavigationBar::automaticColor(
    const QList<QColor> &usedColors)
{
    if (usedColors.isEmpty())
        return QColor::fromHsl(210, 145, 195);

    int bestHue = 0;
    int bestDistance = -1;

    for (int candidate = 0; candidate < 360; ++candidate) {
        int minimumDistance = std::numeric_limits<int>::max();

        for (const QColor &used : usedColors) {
            const int usedHue = used.hslHue();

            if (usedHue < 0)
                continue;

            minimumDistance =
                qMin(
                    minimumDistance,
                    hueDistance(candidate, usedHue));
        }

        if (minimumDistance > bestDistance) {
            bestDistance = minimumDistance;
            bestHue = candidate;
        }
    }

    return QColor::fromHsl(bestHue, 145, 195);
}

void MathomNavigationBar::setTabs(
    const QList<Entry> &tabs,
    BasketScene *activeBasket)
{
    clearLayout(m_tabsLayout);

    QList<QColor> usedColors;
    int requiredHeight = 0;

    for (const Entry &entry : tabs) {
        const QColor color = automaticColor(usedColors);
        usedColors.append(color);

        auto *button = new QToolButton(m_tabsWidget);

        button->setText(entry.title);
        button->setCheckable(true);
        button->setChecked(entry.basket == activeBasket);

        const QColor border = color.darker(125);

        button->setStyleSheet(
            QStringLiteral(
                "QToolButton {"
                " background-color: %1;"
                " border: 1px solid %2;"
                " border-bottom: 0px;"
                " border-top-left-radius: 8px;"
                " border-top-right-radius: 8px;"
                " padding: 7px 14px;"
                " margin-right: 2px;"
                " color: #202020;"
                "}"
                "QToolButton:hover {"
                " border: 2px solid %2;"
                "}"
                "QToolButton:checked {"
                " font-weight: 700;"
                " border-bottom: 3px solid #404040;"
                " padding-top: 9px;"
                "}").arg(
                    color.name(),
                    border.name()));

        connect(
            button,
            &QToolButton::clicked,
            this,
            [this, basket = entry.basket]() {
                Q_EMIT navigateRequested(basket);
            });

        button->ensurePolished();
        requiredHeight =
            qMax(requiredHeight, button->sizeHint().height());

        m_tabsLayout->addWidget(button);
    }

    m_tabsLayout->addStretch();

    // La hauteur est calculée à partir des intercalaires eux-mêmes.
    // Cela conserve le trait noir de l'onglet actif tout en collant
    // la barre au plus près de la zone des mathoms.
    if (requiredHeight > 0) {
        m_tabsWidget->setFixedHeight(requiredHeight);
        m_tabsScrollArea->setFixedHeight(requiredHeight);
        m_tabsScrollArea->show();
    } else {
        m_tabsScrollArea->setFixedHeight(0);
        m_tabsScrollArea->hide();
    }
}

void MathomNavigationBar::setBreadcrumb(
    const QList<Entry> &path)
{
    clearLayout(m_breadcrumbLayout);

    for (int i = 0; i < path.size(); ++i) {
        const Entry &entry = path.at(i);

        auto *button = new QToolButton(m_breadcrumbWidget);

        button->setText(entry.title);
        button->setAutoRaise(true);

        if (i == path.size() - 1) {
            QFont font = button->font();
            font.setBold(true);
            button->setFont(font);
        }

        connect(
            button,
            &QToolButton::clicked,
            this,
            [this, basket = entry.basket]() {
                Q_EMIT navigateRequested(basket);
            });

        m_breadcrumbLayout->addWidget(button);

        if (i < path.size() - 1) {
            auto *separator =
                new QLabel(QStringLiteral("›"), m_breadcrumbWidget);

            m_breadcrumbLayout->addWidget(separator);
        }
    }

    m_breadcrumbLayout->addStretch();
}
