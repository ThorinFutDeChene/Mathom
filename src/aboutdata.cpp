/**
 * SPDX-FileCopyrightText: (C) 2003 by Sébastien Laoût <slaout@linux62.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "aboutdata.h"
#include "mathomicons.h"
#include <basket_version.h>

#include <KLocalizedString>
#include <QApplication>
#include <QIcon>

AboutData::AboutData()
    : KAboutData(AboutData::componentName(),
                 AboutData::displayName(),
                 QStringLiteral("0.1.12-dev7"),
                 i18n("<p><b>Taking care of your ideas.</b></p>"
                      "<p>A note-taking application that makes it easy to record ideas as you think, and quickly find them later. "
                      "Organizing your notes has never been so easy.</p>"),
                 KAboutLicense::GPL_V2,
                 i18n("Copyright © 2026 Thorinux Systems; based on BasKet Note Pads. Original copyrights © 2003–2007 Sébastien Laoût and © 2013–2019 Gleb Baryshev"),
                 QString(),
                 QString())
{
    setHomepage(QStringLiteral("https://github.com/ThorinFutDeChene/Mathom"));
    setBugAddress(QByteArray());
    setOrganizationDomain(QByteArrayLiteral("thorinux.fr"));
    setDesktopFileName(QStringLiteral("fr.thorinux.mathom"));
    setProgramLogo(MathomIcons::application());

    // Mathom has its own support and diagnostics workflow.  Replacing the
    // default KAboutData author text also prevents KDE's default bug-report
    // address from being presented as a Mathom contact point.
    setCustomAuthorText(
        i18n("For questions or help with Mathom: contact@thorinux.fr"),
        i18n("For questions or help with Mathom: <a href=\"mailto:contact@thorinux.fr\">contact@thorinux.fr</a>"));

    addAuthor(QStringLiteral("Thorinux Systems"),
              i18n("Mathom maintainer and user support"),
              QStringLiteral("contact@thorinux.fr"));

    // Keep the BasKet lineage visible for attribution, but do not present
    // historical BasKet contributors as current Mathom contacts.
    addCredit(QStringLiteral("Carl Schwan"),
              i18n("BasKet historical co-maintainer"));
    addCredit(QStringLiteral("Niccolò Venerandi"),
              i18n("BasKet historical co-maintainer"));
    addCredit(QStringLiteral("OmegaPhil"),
              i18n("BasKet historical contributor — paste as plain text option"));
    addCredit(QStringLiteral("Kelvie Wong"),
              i18n("BasKet former maintainer"));
    addCredit(QStringLiteral("Sébastien Laoût"),
              i18n("BasKet original author"));
    addCredit(QStringLiteral("Petri Damstén"),
              i18n("BasKet historical contributor — encryption, Kontact integration, KnowIt importer"));
    addCredit(QStringLiteral("Alex Gontmakher"),
              i18n("BasKet historical contributor — auto lock, save-status icon, HTML copy/paste, basket name tooltip, drop to basket name"));
    addCredit(QStringLiteral("Marco Martin"),
              i18n("BasKet historical contributor — original icon"));
}

QString AboutData::componentName()
{
    return QStringLiteral("mathom");
}

QString AboutData::displayName()
{
    return QStringLiteral("Mathom");
}
