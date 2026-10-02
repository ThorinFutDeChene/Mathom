/**
 * SPDX-FileCopyrightText: (C) 2003 by Sébastien Laoût <slaout@linux62.org>
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "aboutdata.h"
#include "mathomicons.h"

#include <KLocalizedString>
#include <QApplication>
#include <QIcon>

AboutData::AboutData()
    : KAboutData(AboutData::componentName(),
                 AboutData::displayName(),
                 QString::fromLatin1(MATHOM_VERSION_STRING),
                 i18n("<p><b>Taking care of your ideas.</b></p>"
                      "<p>A note-taking application that makes it easy to record ideas as you think, and quickly find them later. "
                      "Organizing your notes has never been so easy.</p>"),
                 KAboutLicense::GPL_V2,
                 i18n("Copyright © 2026 Thorinux Systems; based on BasKet Note Pads. Original copyrights © 2003–2007 Sébastien Laoût and © 2013–2019 Gleb Baryshev"),
                 QString(),
                 QString())
{
    setHomepage(QStringLiteral("https://github.com/ThorinFutDeChene/Mathom-Notes"));
    setBugAddress(QByteArray());
    setOrganizationDomain(QByteArrayLiteral("thorinux.fr"));
    setDesktopFileName(QStringLiteral("fr.thorinux.mathom"));
    setProgramLogo(MathomIcons::application());

    // Mathom has its own support and diagnostics workflow.  Replacing the
    // default KAboutData author text also prevents KDE's default bug-report
    // address from being presented as a Mathom contact point.
    setCustomAuthorText(
        i18n("For questions or help with Mathom Notes: contact@thorinux.fr"),
        i18n("For questions or help with Mathom Notes: <a href=\"mailto:contact@thorinux.fr\">contact@thorinux.fr</a>"));

    addAuthor(QStringLiteral("Thorinux Systems"),
              i18n("Mathom Notes maintainer and user support"),
              QStringLiteral("contact@thorinux.fr"));

}

QString AboutData::componentName()
{
    return QStringLiteral("mathom");
}

QString AboutData::displayName()
{
    return QStringLiteral("Mathom Notes");
}
