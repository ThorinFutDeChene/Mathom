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
                 QStringLiteral("0.1.2"),
                 i18n("<p><b>Taking care of your ideas.</b></p>"
                      "<p>A note-taking application that makes it easy to record ideas as you think, and quickly find them later. "
                      "Organizing your notes has never been so easy.</p>"),
                 KAboutLicense::GPL_V2,
                 i18n("Copyright © 2026 Thorinux Systems; based on BasKet Note Pads. Original copyrights © 2003–2007 Sébastien Laoût and © 2013–2019 Gleb Baryshev"),
                 QString(),
                 QString())
{
    // Mathom homepage will be added when the Thorinux Systems project page is available.
    setHomepage(QString());
    setOrganizationDomain(QByteArrayLiteral("thorinux.fr"));
    setDesktopFileName(QStringLiteral("fr.thorinux.mathom"));
    setProgramLogo(MathomIcons::application());

    addAuthor(QStringLiteral("Thorinux Systems"), i18n("Mathom fork maintainer"));

    addAuthor(i18n("Carl Schwan"), i18n("Co-Maintainer"), QStringLiteral("carl@carlschwan.eu"), QStringLiteral("https://carlschwan.eu"));
    addAuthor(i18n("Niccolò Venerandi"), i18n("Co-Maintainer"), QStringLiteral("niccolo@venerandi.com"), QStringLiteral("https://niccolo.venerandi.com/"));
    addAuthor(i18n("OmegaPhil"), i18n("Paste as plaintext option"), QStringLiteral("OmegaPhil@startmail.com"));
    addAuthor(i18n("Kelvie Wong"), i18n("Ex-Maintainer"), QStringLiteral("kelvie@ieee.org"));
    addAuthor(i18n("Sébastien Laoût"), i18n("Original Author"), QStringLiteral("slaout@linux62.org"));
    addAuthor(i18n("Petri Damstén"), i18n("Basket encryption, Kontact integration, KnowIt importer"), QStringLiteral("damu@iki.fi"));
    addAuthor(i18n("Alex Gontmakher"),
              i18n("Baskets auto lock, save-status icon, HTML copy/paste, basket name tooltip, drop to basket name"),
              QStringLiteral("gsasha@cs.technion.ac.il"));
    addAuthor(i18n("Marco Martin"), i18n("Original icon"), QStringLiteral("m4rt@libero.it"));
}

QString AboutData::componentName()
{
    return QStringLiteral("mathom");
}

QString AboutData::displayName()
{
    return QStringLiteral("Mathom");
}
