/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "diagnosticmanager.h"

#include <QAbstractButton>
#include <QApplication>
#include <QDateTime>
#include <QDesktopServices>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QSysInfo>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QUuid>
#include <QWidget>

namespace
{
constexpr auto sessionPrefix = "session-";
constexpr auto sessionSuffix = ".jsonl";
constexpr auto reportPrefix = "report-";
constexpr auto reportSuffix = ".txt";

QString sessionStem(const QString &sessionPath)
{
    QString name = QFileInfo(sessionPath).fileName();
    if (name.startsWith(QLatin1String(sessionPrefix)))
        name.remove(0, QString::fromLatin1(sessionPrefix).size());
    if (name.endsWith(QLatin1String(sessionSuffix)))
        name.chop(QString::fromLatin1(sessionSuffix).size());
    return name;
}

QString safeObjectName(QObject *object)
{
    if (!object)
        return QString();

    const QString name = object->objectName().trimmed();
    if (name.isEmpty())
        return QStringLiteral("<unnamed>");
    return name.left(120);
}
}

DiagnosticManager &DiagnosticManager::instance()
{
    static DiagnosticManager manager;
    return manager;
}

DiagnosticManager::DiagnosticManager() = default;

DiagnosticManager::~DiagnosticManager()
{
    closeSession();
}

QString DiagnosticManager::diagnosticsDirectory() const
{
    return QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation)
        + QStringLiteral("/diagnostics");
}

QString DiagnosticManager::currentSessionId() const
{
    return m_sessionId;
}

QStringList DiagnosticManager::pendingReports() const
{
    return m_pendingReports;
}

void DiagnosticManager::openDiagnosticsFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(diagnosticsDirectory()));
}

void DiagnosticManager::startSession()
{
    if (m_started)
        return;

    QDir dir(diagnosticsDirectory());
    if (!dir.exists() && !dir.mkpath(QStringLiteral(".")))
        return;

    inspectPreviousSessions();

    const QString stamp = QDateTime::currentDateTime().toString(QStringLiteral("yyyyMMdd-HHmmss-zzz"));
    const QString uuid = QUuid::createUuid().toString(QUuid::WithoutBraces).left(8);
    m_sessionId = QStringLiteral("MTH-%1-%2").arg(stamp, uuid);
    m_sessionPath = dir.filePath(QStringLiteral("%1%2%3").arg(
        QLatin1String(sessionPrefix), m_sessionId, QLatin1String(sessionSuffix)));

    m_sessionFile = new QFile(m_sessionPath, this);
    if (!m_sessionFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        delete m_sessionFile;
        m_sessionFile = nullptr;
        return;
    }

    m_startedAtMs = QDateTime::currentMSecsSinceEpoch();
    m_started = true;
    m_closed = false;

    qApp->installEventFilter(this);

    QVariantMap details;
    details.insert(QStringLiteral("version"), QCoreApplication::applicationVersion());
    details.insert(QStringLiteral("qt_version"), QString::fromLatin1(qVersion()));
    details.insert(QStringLiteral("os"), QSysInfo::prettyProductName());
    details.insert(QStringLiteral("kernel"), QStringLiteral("%1 %2").arg(QSysInfo::kernelType(), QSysInfo::kernelVersion()));
    details.insert(QStringLiteral("architecture"), QSysInfo::currentCpuArchitecture());
    details.insert(QStringLiteral("pid"), QCoreApplication::applicationPid());
    logEvent(QStringLiteral("SESSION_OPEN"), details);

    m_heartbeatTimer = new QTimer(this);
    m_heartbeatTimer->setInterval(60000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
        logEvent(QStringLiteral("HEARTBEAT"));
    });
    m_heartbeatTimer->start();
}

void DiagnosticManager::closeSession()
{
    if (!m_started || m_closed)
        return;

    logEvent(QStringLiteral("SESSION_CLOSE"), {
        {QStringLiteral("reason"), QStringLiteral("normal_exit")}
    });

    m_closed = true;

    if (m_heartbeatTimer)
        m_heartbeatTimer->stop();

    if (qApp)
        qApp->removeEventFilter(this);

    if (m_sessionFile) {
        m_sessionFile->flush();
        m_sessionFile->close();
    }
}

QVariantMap DiagnosticManager::baseRecord(const QString &event) const
{
    QVariantMap record;
    record.insert(QStringLiteral("timestamp"), QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
    record.insert(QStringLiteral("session"), m_sessionId);
    record.insert(QStringLiteral("event"), event);

    if (m_startedAtMs > 0)
        record.insert(QStringLiteral("uptime_ms"), QDateTime::currentMSecsSinceEpoch() - m_startedAtMs);

    return record;
}

void DiagnosticManager::logEvent(const QString &event, const QVariantMap &details)
{
    if (!m_started || !m_sessionFile)
        return;

    QVariantMap record = baseRecord(event);
    if (!details.isEmpty())
        record.insert(QStringLiteral("details"), details);

    writeRecord(record);
}

void DiagnosticManager::writeRecord(const QVariantMap &record)
{
    if (!m_sessionFile || !m_sessionFile->isOpen())
        return;

    const QJsonObject object = QJsonObject::fromVariantMap(record);
    m_sessionFile->write(QJsonDocument(object).toJson(QJsonDocument::Compact));
    m_sessionFile->write("\n");
    m_sessionFile->flush();
}

bool DiagnosticManager::sessionClosedNormally(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    bool closed = false;
    while (!file.atEnd()) {
        const QByteArray line = file.readLine();
        if (line.contains("\"event\":\"SESSION_CLOSE\""))
            closed = true;
    }
    return closed;
}

void DiagnosticManager::inspectPreviousSessions()
{
    QDir dir(diagnosticsDirectory());
    const QStringList sessions = dir.entryList(
        {QStringLiteral("%1*%2").arg(QLatin1String(sessionPrefix), QLatin1String(sessionSuffix))},
        QDir::Files,
        QDir::Name);

    QString lastClosed;
    for (const QString &name : sessions) {
        const QString path = dir.filePath(name);
        if (sessionClosedNormally(path))
            lastClosed = name;
    }

    if (!lastClosed.isEmpty())
        cleanupOldSessions(lastClosed);

    const QStringList remaining = dir.entryList(
        {QStringLiteral("%1*%2").arg(QLatin1String(sessionPrefix), QLatin1String(sessionSuffix))},
        QDir::Files,
        QDir::Name);

    for (const QString &name : remaining) {
        const QString path = dir.filePath(name);
        if (sessionClosedNormally(path))
            continue;

        const QString reportPath = createReportForSession(path);
        if (!reportPath.isEmpty())
            m_pendingReports.append(reportPath);
    }
}

void DiagnosticManager::cleanupOldSessions(const QString &lastClosedSessionFile)
{
    QDir dir(diagnosticsDirectory());
    const QStringList sessions = dir.entryList(
        {QStringLiteral("%1*%2").arg(QLatin1String(sessionPrefix), QLatin1String(sessionSuffix))},
        QDir::Files,
        QDir::Name);

    for (const QString &name : sessions) {
        if (name >= lastClosedSessionFile)
            continue;

        const QString oldSessionPath = dir.filePath(name);
        const QString oldReportPath = dir.filePath(
            QStringLiteral("%1%2%3").arg(
                QLatin1String(reportPrefix),
                sessionStem(oldSessionPath),
                QLatin1String(reportSuffix)));

        QFile::remove(oldReportPath);
        QFile::remove(oldSessionPath);
    }
}

QString DiagnosticManager::createReportForSession(const QString &sessionPath)
{
    QDir dir(diagnosticsDirectory());
    const QString reportPath = dir.filePath(
        QStringLiteral("%1%2%3").arg(
            QLatin1String(reportPrefix),
            sessionStem(sessionPath),
            QLatin1String(reportSuffix)));

    if (QFileInfo::exists(reportPath))
        return reportPath;

    QFile input(sessionPath);
    if (!input.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    QFile report(reportPath);
    if (!report.open(QIODevice::WriteOnly | QIODevice::Text))
        return QString();

    report.write("MATHOM - RAPPORT DE DIAGNOSTIC\n");
    report.write("==============================\n\n");
    report.write("Une session precedente ne contient pas de marqueur SESSION_CLOSE.\n");
    report.write("Elle est donc consideree comme terminee de facon anormale.\n");
    report.write("Cela ne prouve pas a lui seul qu'un bug de Mathom est la cause :\n");
    report.write("une extinction de l'ordinateur ou un arret force peuvent produire le meme resultat.\n\n");
    report.write("Le journal ne contient pas le texte saisi dans les Mathoms.\n\n");
    report.write("Version Mathom : ");
    report.write(QCoreApplication::applicationVersion().toUtf8());
    report.write("\nSysteme : ");
    report.write(QSysInfo::prettyProductName().toUtf8());
    report.write("\nArchitecture : ");
    report.write(QSysInfo::currentCpuArchitecture().toUtf8());
    report.write("\nRapport genere : ");
    report.write(QDateTime::currentDateTime().toString(Qt::ISODateWithMs).toUtf8());
    report.write("\n\n--- JOURNAL DE SESSION ---\n");
    report.write(input.readAll());
    report.flush();
    report.close();

    return reportPath;
}

void DiagnosticManager::openReport(const QString &reportPath)
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(reportPath));
}

void DiagnosticManager::prepareEmail(const QString &reportPath)
{
    const QString subject =
        QStringLiteral("[Mathom Bug] %1").arg(QFileInfo(reportPath).completeBaseName());

    const QString body =
        QStringLiteral("Bonjour,\n\n"
                       "Mathom a genere un rapport de diagnostic apres un arret anormal.\n"
                       "Le rapport de diagnostic est joint automatiquement a ce message.\n\n"
                       "Merci.");

    logEvent(
        QStringLiteral("REPORT_EMAIL_PREPARE_BEGIN"),
        {{QStringLiteral("report"), QFileInfo(reportPath).fileName()}});

    // xdg-email supports attachments and uses the user's configured mail
    // application. This is preferred over a plain mailto: URL, because
    // mailto has no portable attachment mechanism.
    const QString xdgEmail = QStandardPaths::findExecutable(QStringLiteral("xdg-email"));
    if (!xdgEmail.isEmpty()) {
        const QStringList arguments = {
            QStringLiteral("--utf8"),
            QStringLiteral("--subject"),
            subject,
            QStringLiteral("--body"),
            body,
            QStringLiteral("--attach"),
            reportPath,
            QStringLiteral("contact@thorinux.fr")
        };

        if (QProcess::startDetached(xdgEmail, arguments)) {
            logEvent(
                QStringLiteral("REPORT_EMAIL_PREPARED"),
                {{QStringLiteral("method"), QStringLiteral("xdg-email")},
                 {QStringLiteral("attachment"), QFileInfo(reportPath).fileName()}});
            return;
        }
    }

    // Fallback for desktops where xdg-email is unavailable. The report
    // folder is opened as well so the user can attach the file manually.
    QUrl mail(QStringLiteral("mailto:contact@thorinux.fr"));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("subject"), subject);
    query.addQueryItem(
        QStringLiteral("body"),
        QStringLiteral("Bonjour,\n\n"
                       "Mathom a genere un rapport de diagnostic apres un arret anormal.\n"
                       "L'ajout automatique de la piece jointe n'a pas fonctionne.\n"
                       "Merci de joindre le rapport ouvert dans le dossier de diagnostic.\n\n"
                       "Fichier : %1")
            .arg(QFileInfo(reportPath).fileName()));
    mail.setQuery(query);

    QDesktopServices::openUrl(mail);
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(reportPath).absolutePath()));

    logEvent(
        QStringLiteral("REPORT_EMAIL_PREPARED"),
        {{QStringLiteral("method"), QStringLiteral("mailto-fallback")},
         {QStringLiteral("attachment"), QStringLiteral("manual")}});
}

void DiagnosticManager::showPendingReportDialog(QWidget *parent)
{
    if (m_pendingReports.isEmpty())
        return;

    const QString reportPath = m_pendingReports.constLast();

    // Keep the diagnostic workflow available after "Voir le rapport".
    // Opening the report must not make the email action disappear.
    for (;;) {
        QMessageBox box(parent);
        box.setIcon(QMessageBox::Warning);
        box.setWindowTitle(tr("Mathom - Rapport de diagnostic"));
        box.setText(tr("Mathom ne s'est pas ferme normalement lors de la derniere utilisation."));
        box.setInformativeText(
            tr("Un rapport technique a ete cree. Il ne contient pas le texte de tes notes.\n\n"
               "Tu peux d'abord le consulter puis revenir ici pour le transmettre a Thorinux."));

        auto *viewButton =
            box.addButton(tr("Voir le rapport"), QMessageBox::ActionRole);
        auto *emailButton =
            box.addButton(tr("Preparer un e-mail a Thorinux"), QMessageBox::ActionRole);
        box.addButton(tr("Fermer"), QMessageBox::RejectRole);

        box.exec();

        if (box.clickedButton() == viewButton) {
            logEvent(
                QStringLiteral("REPORT_VIEW"),
                {{QStringLiteral("report"), QFileInfo(reportPath).fileName()}});
            openReport(reportPath);
            continue;
        }

        if (box.clickedButton() == emailButton)
            prepareEmail(reportPath);

        break;
    }
}

bool DiagnosticManager::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_started || m_closed || !event)
        return QObject::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress) {
        const auto *mouseEvent = static_cast<QMouseEvent *>(event);
        QVariantMap details;
        details.insert(QStringLiteral("class"), QString::fromLatin1(watched->metaObject()->className()));
        details.insert(QStringLiteral("object"), safeObjectName(watched));
        details.insert(QStringLiteral("button"), int(mouseEvent->button()));
        details.insert(QStringLiteral("x"), qRound(mouseEvent->position().x()));
        details.insert(QStringLiteral("y"), qRound(mouseEvent->position().y()));
        logEvent(QStringLiteral("MOUSE_PRESS"), details);
    } else if (event->type() == QEvent::KeyPress) {
        const auto *keyEvent = static_cast<QKeyEvent *>(event);
        QVariantMap details;
        details.insert(QStringLiteral("class"), QString::fromLatin1(watched->metaObject()->className()));
        details.insert(QStringLiteral("object"), safeObjectName(watched));
        details.insert(QStringLiteral("modifiers"), int(keyEvent->modifiers()));

        const bool printable =
            !keyEvent->text().isEmpty()
            && keyEvent->text().at(0).isPrint();

        if (printable) {
            details.insert(QStringLiteral("input"), QStringLiteral("printable_character"));
        } else {
            details.insert(QStringLiteral("key"), keyEvent->key());
        }

        logEvent(QStringLiteral("KEY_PRESS"), details);
    }

    return QObject::eventFilter(watched, event);
}
