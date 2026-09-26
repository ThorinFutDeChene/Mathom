/**
 * SPDX-FileCopyrightText: (C) 2003 by Sébastien Laoût <slaout@linux62.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "basketproperties.h"

#include <QApplication>
#include <QButtonGroup>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPixmap>
#include <QPushButton>
#include <QRadioButton>
#include <QStringList>
#include <QStyle>
#include <QVBoxLayout>

#include <KComboBox>
#include <KConfigGroup>
#include <KIconDialog>
#include <KIconLoader>
#include <KLocalizedString>
#include <KShortcutWidget>

#include <algorithm>

#include "backgroundmanager.h"
#include "basketscene.h"
#include "gitwrapper.h"
#include "global.h"
#include "kcolorcombo2.h"
#include "variouswidgets.h"

#include "ui_basketproperties.h"

BasketPropertiesDialog::BasketPropertiesDialog(BasketScene *basket, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::BasketPropertiesUi)
    , m_basket(basket)
{
    // Set up dialog options
    m_ui->setupUi(this);
    QPushButton *okButton = m_ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setDefault(true);
    setObjectName("BasketProperties");

    m_ui->icon->setIconType(KIconLoader::NoGroup, KIconLoader::Application);
    m_ui->icon->setIcon(m_basket->icon());

    int size = std::max(m_ui->icon->sizeHint().width(), m_ui->icon->sizeHint().height());
    m_ui->icon->setFixedSize(size, size); // Make it square!
    m_ui->name->setText(m_basket->basketName());
    m_ui->name->setMinimumWidth(m_ui->name->fontMetrics().maxWidth() * 20);

    // Mathom shelf tab color
    auto *tabColorLabel =
        new QLabel(i18n("Tab color:"), this);

    m_tabColorAutomatic =
        new QCheckBox(i18n("Automatic"), this);

    m_tabColor =
        new KColorCombo2(this);

    QColor initialTabColor = m_basket->tabColor();

    if (!initialTabColor.isValid())
        initialTabColor = QColor::fromHsl(210, 145, 195);

    m_tabColor->setColor(initialTabColor);
    m_tabColorAutomatic->setChecked(
        m_basket->tabColorAutomatic());

    m_tabColor->setEnabled(
        !m_tabColorAutomatic->isChecked());

    auto *tabColorControls =
        new QHBoxLayout();

    tabColorControls->setContentsMargins(0, 0, 0, 0);
    tabColorControls->addWidget(m_tabColorAutomatic);
    tabColorControls->addWidget(m_tabColor, 1);

    m_ui->appearanceLayout->addWidget(
        tabColorLabel,
        0,
        0);

    m_ui->appearanceLayout->addLayout(
        tabColorControls,
        0,
        1);

    connect(
        m_tabColorAutomatic,
        &QCheckBox::toggled,
        m_tabColor,
        &QWidget::setDisabled);


    // Keyboard Shortcut:
    QList<QKeySequence> shortcuts{m_basket->shortcut()};
    m_ui->shortcut->setShortcut(shortcuts);

    m_ui->helpLabel->setMessage(
        i18n("<p><strong>Easily Remember your Shortcuts</strong>:<br>"
             "With the first option, giving the current location a shortcut of the form <strong>Alt+Letter</strong> will underline that letter in the organization tree.<br>"
             "For instance, if you assign the shortcut <i>Alt+T</i> to a location named <i>Tips</i>, it will be displayed as <i><u>T</u>ips</i> "
             "in the tree. "
             "It helps you visualize the shortcuts to remember them more quickly.</p>"
             "<p><strong>Local vs Global</strong>:<br>"
             "The first option allows you to show the location while the main window is active. "
             "Global shortcuts are valid from anywhere, even if the window is hidden.</p>"
             "<p><strong>Show vs Switch</strong>:<br>"
             "The last option makes this location the current one without opening the main window. "
             "It is useful in addition to the configurable global shortcuts, eg. to paste the clipboard or the selection into the current location from "
             "anywhere.</p>"));

    connect(m_ui->shortcut, &KShortcutWidget::shortcutChanged, this, &BasketPropertiesDialog::capturedShortcut);

    switch (m_basket->shortcutAction()) {
    default:
    case 0:
        m_ui->showBasket->setChecked(true);
        break;
    case 1:
        m_ui->globalButton->setChecked(true);
        break;
    case 2:
        m_ui->switchButton->setChecked(true);
        break;
    }

    // Connect the Ok and Apply buttons to actually apply the changes
    connect(okButton, &QPushButton::clicked, this, &BasketPropertiesDialog::applyChanges);
    connect(m_ui->buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked, this, &BasketPropertiesDialog::applyChanges);
}

BasketPropertiesDialog::~BasketPropertiesDialog()
{
    delete m_ui;
}

bool BasketPropertiesDialog::event(QEvent *event)
{
    const bool result = QDialog::event(event);
    if (event->type() == QEvent::Polish) {
        m_ui->name->setFocus();
    }
    return result;
}

void BasketPropertiesDialog::applyChanges()
{
    if (m_ui->showBasket->isChecked()) {
        m_basket->setShortcut(m_ui->shortcut->shortcut()[0], 0);
    } else if (m_ui->globalButton->isChecked()) {
        m_basket->setShortcut(m_ui->shortcut->shortcut()[0], 1);
    } else if (m_ui->switchButton->isChecked()) {
        m_basket->setShortcut(m_ui->shortcut->shortcut()[0], 2);
    }

    if (m_tabColorAutomatic->isChecked()) {
        // Preserve an existing automatic color. If the user explicitly
        // switches from custom back to automatic, invalidate it so the
        // navigation bar computes a new well-separated color.
        if (!m_basket->tabColorAutomatic())
            m_basket->setTabColor(QColor(), true);
    } else {
        m_basket->setTabColor(
            m_tabColor->color(),
            false);
    }

    // Called last because it emits propertiesChanged() so the
    // navigation tree also refreshes the name, icon, shortcut
    // and shelf tab color.
    m_basket->setShelfIdentity(
        m_ui->icon->icon(),
        m_ui->name->text());
    GitWrapper::commitBasket(m_basket);
    m_basket->save();
}

void BasketPropertiesDialog::capturedShortcut(const QList<QKeySequence> &sc)
{
    // TODO: Validate it!
    m_ui->shortcut->setShortcut(sc);
}

#include "moc_basketproperties.cpp"
