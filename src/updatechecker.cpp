/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "updatechecker.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QRegularExpression>
#include <QUrl>
#include <QWidget>

#include <KLocalizedString>
#include <KMessageBox>

namespace
{
const QString updateApiUrl =
    QStringLiteral("https://api.github.com/repos/ThorinFutDeChene/Mathom/releases/latest");
}

UpdateChecker::UpdateChecker(QWidget *parent)
    : QObject(parent)
    , m_parentWidget(parent)
    , m_network(new QNetworkAccessManager(this))
{
}

void UpdateChecker::check(QWidget *parent)
{
    auto *checker = new UpdateChecker(parent);
    checker->start();
}

QString UpdateChecker::installedVersion() const
{
    QProcess process;

    process.start(
        QStringLiteral("dpkg-query"),
        {
            QStringLiteral("-W"),
            QStringLiteral("-f=${Version}"),
            QStringLiteral("mathom"),
        });

    if (!process.waitForFinished(3000)) {
        return {};
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        return {};
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

bool UpdateChecker::isVersionGreater(
    const QString &candidate,
    const QString &reference) const
{
    QProcess process;

    process.start(
        QStringLiteral("dpkg"),
        {
            QStringLiteral("--compare-versions"),
            candidate,
            QStringLiteral("gt"),
            reference,
        });

    if (!process.waitForFinished(3000)) {
        return false;
    }

    return process.exitStatus() == QProcess::NormalExit
        && process.exitCode() == 0;
}

void UpdateChecker::start()
{
    const QString currentVersion = installedVersion();

    if (currentVersion.isEmpty()) {
        KMessageBox::error(
            m_parentWidget,
            i18n("Unable to determine the installed Mathom Debian package version."),
            i18n("Mathom Update"));

        deleteLater();
        return;
    }

    QNetworkRequest request{QUrl(updateApiUrl)};

    request.setRawHeader(
        QByteArrayLiteral("Accept"),
        QByteArrayLiteral("application/vnd.github+json"));

    request.setRawHeader(
        QByteArrayLiteral("User-Agent"),
        QByteArrayLiteral("Mathom-Updater"));

    request.setRawHeader(
        QByteArrayLiteral("X-GitHub-Api-Version"),
        QByteArrayLiteral("2022-11-28"));

    QNetworkReply *reply = m_network->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, currentVersion]() {
        if (reply->error() != QNetworkReply::NoError) {
            KMessageBox::error(
                m_parentWidget,
                i18n(
                    "Unable to contact the Mathom update server: %1",
                    reply->errorString()),
                i18n("Mathom Update"));

            reply->deleteLater();
            deleteLater();
            return;
        }

        const QByteArray data = reply->readAll();
        reply->deleteLater();

        QJsonParseError parseError;
        const QJsonDocument document = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
            KMessageBox::error(
                m_parentWidget,
                i18n("The update server returned invalid data."),
                i18n("Mathom Update"));

            deleteLater();
            return;
        }

        const QJsonObject release = document.object();
        const QJsonArray assets = release.value(QStringLiteral("assets")).toArray();

        const QRegularExpression packagePattern(
            QStringLiteral("^mathom_(.+)_amd64\\.deb$"));

        QString newestVersion;

        for (const QJsonValue &value : assets) {
            const QJsonObject asset = value.toObject();
            const QString fileName = asset.value(QStringLiteral("name")).toString();

            const QRegularExpressionMatch match = packagePattern.match(fileName);

            if (!match.hasMatch()) {
                continue;
            }

            const QString version = match.captured(1);

            if (newestVersion.isEmpty()
                || isVersionGreater(version, newestVersion)) {
                newestVersion = version;
            }
        }

        if (newestVersion.isEmpty()) {
            KMessageBox::error(
                m_parentWidget,
                i18n("No Mathom Debian package was found in the latest release."),
                i18n("Mathom Update"));

            deleteLater();
            return;
        }

        const QString versions =
            i18n("Installed version: %1", currentVersion)
            + QStringLiteral("\n")
            + i18n("Available version: %1", newestVersion);

        if (isVersionGreater(newestVersion, currentVersion)) {
            KMessageBox::information(
                m_parentWidget,
                i18n("A Mathom update is available.")
                    + QStringLiteral("\n\n")
                    + versions,
                i18n("Mathom Update"));
        } else {
            KMessageBox::information(
                m_parentWidget,
                i18n("Mathom is up to date.")
                    + QStringLiteral("\n\n")
                    + versions,
                i18n("Mathom Update"));
        }

        deleteLater();
    });
}
