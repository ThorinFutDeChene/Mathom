/**
 * SPDX-FileCopyrightText: (C) 2026 Thorinux
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef SPREADSHEETCONTENT_H
#define SPREADSHEETCONTENT_H

#include "notecontent.h"

#include <QGraphicsItem>
#include <QVector>

class QPainter;
class QStyleOptionGraphicsItem;

class SpreadsheetContent;

class SpreadsheetItem : public QGraphicsItem
{
public:
    SpreadsheetItem(Note *parent, SpreadsheetContent *content);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void setSize(qreal width, qreal height);

private:
    Note *m_note;
    SpreadsheetContent *m_content;
    qreal m_width = 540.0;
    qreal m_height = 232.0;
};

class SpreadsheetContent : public NoteContent
{
public:
    static constexpr int DefaultRows = 8;
    static constexpr int DefaultColumns = 5;

    SpreadsheetContent(Note *parent, const QString &fileName, bool lazyLoad = false);
    ~SpreadsheetContent() override;

    QString toText(const QString &cuttedFullPath) override;
    QString toHtml(const QString &imageName, const QString &cuttedFullPath) override;
    bool useFile() const override;
    bool canBeSavedAs() const override;
    QString saveAsFilters() const override;
    bool match(const FilterData &data) override;

    void exportToHTML(HTMLExporter *exporter, int indent) override;
    QString cssClass() const override;
    qreal setWidthAndGetHeight(qreal width) override;
    bool loadFromFile(bool lazyLoad) override;
    bool saveToFile() override;
    void fontChanged() override;
    QString editToolTipText() const override;
    QPixmap feedbackPixmap(qreal width, qreal height) override;

    QGraphicsItem *graphicsItem() override
    {
        return &m_item;
    }

    int rowCount() const
    {
        return m_rows;
    }

    int columnCount() const
    {
        return m_columns;
    }

    QString cell(int row, int column) const;
    QString displayValue(int row, int column) const;
    void setTableData(int rows, int columns, const QVector<QVector<QString>> &cells);

    static QString columnName(int column);

private:
    void resetToDefault();
    void refreshView();

    int m_rows = DefaultRows;
    int m_columns = DefaultColumns;
    QVector<QVector<QString>> m_cells;
    SpreadsheetItem m_item;
};

#endif // SPREADSHEETCONTENT_H
