/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#include <athenaeum/librarydelegate.h>
#include <athenaeum/librarymodel.h>

#include <QApplication>
#include <QDebug>
#include <QPainter>

namespace Athenaeum
{

    LibraryDelegate::LibraryDelegate(QObject * parent)
        : QStyledItemDelegate(parent)
    {}

    LibraryDelegate::~LibraryDelegate()
    {}

    QWidget * LibraryDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
        // Only the first column is editable
        if (index.column() > 0) {
            // Perhaps check to see if this item cannot be edited (headings?)
            return 0;
        } else {
            return QStyledItemDelegate::createEditor(parent, option, index);
        }
    }

    void LibraryDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
    {
	    //qDebug() << "LibraryDelegate::paint" << index;
        if (index.parent().isValid()) {
            painter->save();

            // Split cell into two:
            //  1. Normal styled item
            //  2. Lozenge with item count / unread count

            // Get useful information from the index
            int item_count = index.data(LibraryModel::ItemCountRole).toInt();
            int unread_item_count = index.data(LibraryModel::UnreadItemCountRole).toInt();
            bool can_fetch_more = index.data(LibraryModel::CanFetchMoreRole).toBool();
            AbstractBibliographicCollection::State state = index.data(LibraryModel::StateRole).value< AbstractBibliographicCollection::State >();

            // Calculate the width of the Lozenge
            //  w = width of text + lozenge + padding
            static const int padding = 1;
            static const int spacing = 1;
            QFont font(option.font);
            font.setPointSize(font.pointSize() - 3);
            font.setBold(true);
            QFontMetrics fontMetrics(font);
            QString item_count_text = QString::number(item_count) + (can_fetch_more ? "+" : "");
            int item_count_lozenge_width = fontMetrics.width(item_count_text);
            int unread_item_count_lozenge_width = fontMetrics.width(QString::number(unread_item_count));
            int lozenge_radius = qRound(option.fontMetrics.height() / 2.0) - spacing;
            int lozenge_height = 2 * lozenge_radius;
            int lozenge_width = padding +
                                lozenge_radius +
                                item_count_lozenge_width +
                                lozenge_radius +
                                padding;
            if (unread_item_count > 0) {
                lozenge_width += padding +
                                 lozenge_radius +
                                 unread_item_count_lozenge_width +
                                 lozenge_radius;
            }

            // Leave room for status icon to do its thing
            if (state == AbstractBibliographicCollection::BusyState) {
                lozenge_width += 20;
            }

            // Partition space
            QStyleOptionViewItemV4 option_left(option);
            option_left.rect.adjust(0, 0, -lozenge_width, 0);
            QStyleOptionViewItemV4 option_right(option);
            option_right.rect.setLeft(option_left.rect.right());

            // Draw normal item
            const QStyleOptionViewItemV3 * optionV3 = qstyleoption_cast< const QStyleOptionViewItemV3 * >(&option);
            const QWidget * widget = optionV3 ? optionV3->widget : 0;
            QStyle * style = widget ? widget->style() : QApplication::style();
            initStyleOption(&option_left, index);
            style->drawControl(QStyle::CE_ItemViewItem, &option_left, painter, widget);
            initStyleOption(&option_right, index);
			//option_right.icon = QIcon();
			//option_right.text = QString();
            //style->drawControl(QStyle::CE_ItemViewItem, &option_right, painter, widget);
            style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option_right, painter, widget);

            // Draw item count lozenge
            int top = option.rect.top() + qRound((option.rect.height() - lozenge_height) / 2.0);
            int left = qRound(option_right.rect.left() + spacing);

            {
                QImage image(item_count_lozenge_width + 2 * lozenge_radius, lozenge_height, QImage::Format_ARGB32);
                image.fill(Qt::transparent);
                QPainterPath path;
                path.setFillRule(Qt::WindingFill);
                path.addEllipse(0, 0, lozenge_height, lozenge_height);
                path.addEllipse(item_count_lozenge_width, 0, lozenge_height, lozenge_height);
                path.addRect(lozenge_radius, 0, item_count_lozenge_width, lozenge_height);
                QPainter imagePainter(&image);
                if (option.state & QStyle::State_Selected) {
                    imagePainter.setBrush(QColor(255, 255, 255, 200));
                } else {
                    imagePainter.setBrush(QColor(0, 0, 0, 100));
                }
                imagePainter.setRenderHints(QPainter::Antialiasing, true);
                imagePainter.setPen(Qt::NoPen);
                imagePainter.drawPath(path);
                imagePainter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
                imagePainter.setBrush(Qt::NoBrush);
                imagePainter.setPen(Qt::black);
                imagePainter.setFont(font);
                imagePainter.drawText(image.rect(), Qt::AlignCenter, item_count_text);
                painter->drawImage(left, top, image);
            }

            painter->restore();
        } else if (index.isValid()) {
            QStyleOptionViewItem option_heading(option);
            option_heading.font.setPointSize(option_heading.font.pointSize() - 3);
            option_heading.font.setBold(true);
            option_heading.palette.setColor(QPalette::Text, QColor(0, 0, 0, 180));
            QStyledItemDelegate::paint(painter, option_heading, index);
        }
    }

}
