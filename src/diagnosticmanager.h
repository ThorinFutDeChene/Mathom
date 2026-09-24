/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DIAGNOSTICMANAGER_H
#define DIAGNOSTICMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

#include "basket_export.h"

class QFile;
class QTimer;
class QWidget;

class BASKET_EXPORT DiagnosticManager : public QObject
{
    Q_OBJECT

public:
    static DiagnosticManager &instance();

    void startSession();
    void closeSession();

    void logEvent(const QString &event, const QVariantMap &details = {});
    QString diagnosticsDirectory() const;
    QString currentSessionId() const;
    QStringList pendingReports() const;
    void openDiagnosticsFolder();

    void showPendingReportDialog(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    DiagnosticManager();
    ~DiagnosticManager() override;

    Q_DISABLE_COPY_MOVE(DiagnosticManager)

    void inspectPreviousSessions();
    void cleanupOldSessions(const QString &lastClosedSessionFile);
    bool sessionClosedNormally(const QString &path) const;
    QString createReportForSession(const QString &sessionPath);
    void openReport(const QString &reportPath);
    void prepareEmail(const QString &reportPath);
    QVariantMap baseRecord(const QString &event) const;
    void writeRecord(const QVariantMap &record);

    QFile *m_sessionFile = nullptr;
    QTimer *m_heartbeatTimer = nullptr;
    QString m_sessionId;
    QString m_sessionPath;
    QStringList m_pendingReports;
    qint64 m_startedAtMs = 0;
    bool m_started = false;
    bool m_closed = false;
};

#endif // DIAGNOSTICMANAGER_H
