/**
 * SPDX-FileCopyrightText: (C) 2026 Thorinux
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "spreadsheetcontent.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QPalette>

#include <KLocalizedString>

#include "basketscene.h"
#include "common.h"
#include "filter.h"
#include "htmlexporter.h"
#include "note.h"

namespace
{
constexpr qreal RowHeaderWidth = 38.0;
constexpr qreal ColumnHeaderHeight = 24.0;
constexpr qreal RowHeight = 26.0;
constexpr qreal MinimumSpreadsheetWidth = 340.0;
}

SpreadsheetItem::SpreadsheetItem(Note *parent, SpreadsheetContent *content)
    : QGraphicsItem(parent)
    , m_note(parent)
    , m_content(content)
{
}

QRectF SpreadsheetItem::boundingRect() const
{
    return QRectF(0, 0, m_width, m_height);
}

void SpreadsheetItem::setSize(qreal width, qreal height)
{
    if (qFuzzyCompare(m_width, width) && qFuzzyCompare(m_height, height))
        return;

    prepareGeometryChange();
    m_width = width;
    m_height = height;
    update();
}

void SpreadsheetItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_note || !m_content)
        return;

    const int rows = qMax(1, m_content->rowCount());
    const int columns = qMax(1, m_content->columnCount());
    const qreal dataWidth = qMax<qreal>(1.0, m_width - RowHeaderWidth);
    const qreal cellWidth = dataWidth / columns;

    QColor background = m_note->backgroundColor();
    QColor foreground = m_note->textColor();
    QColor grid = foreground;
    grid.setAlpha(90);
    QColor header = background.darker(background.lightness() > 128 ? 106 : 125);

    painter->save();
    painter->setFont(m_note->font());
    painter->fillRect(boundingRect(), background);
    painter->fillRect(QRectF(0, 0, m_width, ColumnHeaderHeight), header);
    painter->fillRect(QRectF(0, 0, RowHeaderWidth, m_height), header);

    painter->setPen(grid);
    painter->drawRect(boundingRect().adjusted(0, 0, -1, -1));
    painter->drawLine(QPointF(RowHeaderWidth, 0), QPointF(RowHeaderWidth, m_height));
    painter->drawLine(QPointF(0, ColumnHeaderHeight), QPointF(m_width, ColumnHeaderHeight));

    const QFontMetrics metrics(m_note->font());

    for (int column = 0; column < columns; ++column) {
        const qreal x = RowHeaderWidth + column * cellWidth;
        painter->drawLine(QPointF(x, 0), QPointF(x, m_height));
        const QRectF headerRect(x + 3, 0, cellWidth - 6, ColumnHeaderHeight);
        painter->setPen(foreground);
        painter->drawText(headerRect, Qt::AlignCenter, SpreadsheetContent::columnName(column));
        painter->setPen(grid);
    }

    for (int row = 0; row < rows; ++row) {
        const qreal y = ColumnHeaderHeight + row * RowHeight;
        painter->drawLine(QPointF(0, y), QPointF(m_width, y));

        painter->setPen(foreground);
        painter->drawText(QRectF(2, y, RowHeaderWidth - 4, RowHeight), Qt::AlignCenter, QString::number(row + 1));

        for (int column = 0; column < columns; ++column) {
            const qreal x = RowHeaderWidth + column * cellWidth;
            const QRectF textRect(x + 4, y + 1, cellWidth - 8, RowHeight - 2);
            const QString value = metrics.elidedText(m_content->displayValue(row, column), Qt::ElideRight, qMax(0, int(textRect.width())));
            painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter, value);
        }

        painter->setPen(grid);
    }

    painter->restore();
}

SpreadsheetContent::SpreadsheetContent(Note *parent, const QString &fileName, bool lazyLoad)
    : NoteContent(parent, NoteType::Spreadsheet, fileName)
    , m_item(parent, this)
{
    resetToDefault();

    if (parent) {
        parent->addToGroup(&m_item);
        m_item.setPos(parent->contentX(), Note::NOTE_MARGIN);
    }

    basket()->addWatchedFile(fullPath());
    loadFromFile(lazyLoad);
    refreshView();
}

SpreadsheetContent::~SpreadsheetContent()
{
    if (note())
        note()->removeFromGroup(&m_item);
}

void SpreadsheetContent::resetToDefault()
{
    m_rows = DefaultRows;
    m_columns = DefaultColumns;
    m_cells = QVector<QVector<QString>>(m_rows, QVector<QString>(m_columns));
}

QString SpreadsheetContent::cell(int row, int column) const
{
    if (row < 0 || row >= m_rows || column < 0 || column >= m_columns)
        return QString();

    return m_cells.at(row).at(column);
}

QString SpreadsheetContent::displayValue(int row, int column) const
{
    return cell(row, column);
}

void SpreadsheetContent::setTableData(int rows, int columns, const QVector<QVector<QString>> &cells)
{
    m_rows = qBound(1, rows, 200);
    m_columns = qBound(1, columns, 50);
    m_cells = QVector<QVector<QString>>(m_rows, QVector<QString>(m_columns));

    const int copyRows = qMin(m_rows, cells.size());
    for (int row = 0; row < copyRows; ++row) {
        const int copyColumns = qMin(m_columns, cells.at(row).size());
        for (int column = 0; column < copyColumns; ++column)
            m_cells[row][column] = cells.at(row).at(column);
    }

    refreshView();
}

QString SpreadsheetContent::columnName(int column)
{
    QString result;
    int value = column;

    do {
        result.prepend(QChar(QLatin1Char('A').unicode() + (value % 26)));
        value = value / 26 - 1;
    } while (value >= 0);

    return result;
}

QString SpreadsheetContent::toText(const QString &)
{
    QStringList lines;
    for (int row = 0; row < m_rows; ++row) {
        QStringList values;
        for (int column = 0; column < m_columns; ++column)
            values.append(cell(row, column));
        lines.append(values.join(QLatin1Char('\t')));
    }
    return lines.join(QLatin1Char('\n'));
}

QString SpreadsheetContent::toHtml(const QString &, const QString &)
{
    QString html = QStringLiteral("<table border=\"1\" cellspacing=\"0\" cellpadding=\"3\"><thead><tr><th></th>");
    for (int column = 0; column < m_columns; ++column)
        html += QStringLiteral("<th>%1</th>").arg(columnName(column).toHtmlEscaped());
    html += QStringLiteral("</tr></thead><tbody>");

    for (int row = 0; row < m_rows; ++row) {
        html += QStringLiteral("<tr><th>%1</th>").arg(row + 1);
        for (int column = 0; column < m_columns; ++column)
            html += QStringLiteral("<td>%1</td>").arg(displayValue(row, column).toHtmlEscaped());
        html += QStringLiteral("</tr>");
    }

    html += QStringLiteral("</tbody></table>");
    return html;
}

bool SpreadsheetContent::useFile() const
{
    return true;
}

bool SpreadsheetContent::canBeSavedAs() const
{
    return true;
}

QString SpreadsheetContent::saveAsFilters() const
{
    return i18n("Mathom Spreadsheet (*.mcalc)");
}

bool SpreadsheetContent::match(const FilterData &data)
{
    return toText(QString()).contains(data.string, Qt::CaseInsensitive);
}

void SpreadsheetContent::exportToHTML(HTMLExporter *exporter, int)
{
    exporter->stream << toHtml(QString(), QString());
}

QString SpreadsheetContent::cssClass() const
{
    return QStringLiteral("spreadsheet");
}

qreal SpreadsheetContent::setWidthAndGetHeight(qreal width)
{
    const qreal itemWidth = qMax(MinimumSpreadsheetWidth, width - 1.0);
    const qreal itemHeight = ColumnHeaderHeight + RowHeight * m_rows;
    m_item.setSize(itemWidth, itemHeight);
    return itemHeight;
}

bool SpreadsheetContent::loadFromFile(bool)
{
    QByteArray raw;
    if (!FileStorage::loadFromFile(fullPath(), &raw) || raw.trimmed().isEmpty())
        return false;

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(raw, &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        return false;

    const QJsonObject object = document.object();
    const int rows = qBound(1, object.value(QStringLiteral("rows")).toInt(DefaultRows), 200);
    const int columns = qBound(1, object.value(QStringLiteral("columns")).toInt(DefaultColumns), 50);
    QVector<QVector<QString>> cells(rows, QVector<QString>(columns));

    const QJsonArray rowsArray = object.value(QStringLiteral("cells")).toArray();
    for (int row = 0; row < qMin(rows, rowsArray.size()); ++row) {
        const QJsonArray columnsArray = rowsArray.at(row).toArray();
        for (int column = 0; column < qMin(columns, columnsArray.size()); ++column)
            cells[row][column] = columnsArray.at(column).toString();
    }

    setTableData(rows, columns, cells);
    return true;
}

bool SpreadsheetContent::saveToFile()
{
    QJsonObject object;
    object.insert(QStringLiteral("format"), QStringLiteral("mathom-spreadsheet"));
    object.insert(QStringLiteral("version"), 1);
    object.insert(QStringLiteral("rows"), m_rows);
    object.insert(QStringLiteral("columns"), m_columns);

    QJsonArray rowsArray;
    for (int row = 0; row < m_rows; ++row) {
        QJsonArray columnsArray;
        for (int column = 0; column < m_columns; ++column)
            columnsArray.append(cell(row, column));
        rowsArray.append(columnsArray);
    }
    object.insert(QStringLiteral("cells"), rowsArray);

    return FileStorage::saveToFile(fullPath(), QJsonDocument(object).toJson(QJsonDocument::Compact));
}

void SpreadsheetContent::fontChanged()
{
    refreshView();
}

QString SpreadsheetContent::editToolTipText() const
{
    return i18n("Edit this spreadsheet");
}

QPixmap SpreadsheetContent::feedbackPixmap(qreal width, qreal height)
{
    const QSizeF sourceSize = m_item.boundingRect().size();
    if (sourceSize.isEmpty())
        return QPixmap();

    const qreal scale = qMin<qreal>(1.0, qMin(width / sourceSize.width(), height / sourceSize.height()));
    const QSize target(qMax(1, qRound(sourceSize.width() * scale)), qMax(1, qRound(sourceSize.height() * scale)));

    QPixmap pixmap(target);
    pixmap.fill(note()->backgroundColor().darker(FEEDBACK_DARKING));

    QPainter painter(&pixmap);
    painter.scale(scale, scale);
    m_item.paint(&painter, nullptr, nullptr);
    painter.end();

    return pixmap;
}

void SpreadsheetContent::refreshView()
{
    const qreal currentWidth = m_item.boundingRect().width();
    m_item.setSize(qMax(MinimumSpreadsheetWidth, currentWidth), ColumnHeaderHeight + RowHeight * m_rows);
    m_item.update();
    contentChanged(MinimumSpreadsheetWidth);
}
