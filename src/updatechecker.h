/**
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QObject>
#include <QString>

class QWidget;
class QNetworkAccessManager;

class UpdateChecker final : public QObject
{
public:
    static void check(QWidget *parent);

private:
    explicit UpdateChecker(QWidget *parent);

    void start();

    QString installedVersion() const;
    bool isVersionGreater(const QString &candidate, const QString &reference) const;

    QWidget *m_parentWidget = nullptr;
    QNetworkAccessManager *m_network = nullptr;
};
