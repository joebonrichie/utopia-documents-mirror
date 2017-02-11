/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
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

#include <utopia2/qt/elidedlabel.h>

#include <QVariant>

#include <QDebug>

namespace Utopia
{

    class ElidedLabelPrivate
    {
    public:
        ElidedLabelPrivate(ElidedLabel * label, const QString & fullText = QString())
            : label(label), elideMode(Qt::ElideRight), fullText(fullText)
        {
            if (!fullText.isNull()) {
                elide();
            }
        }

        ElidedLabel * label;
        Qt::TextElideMode elideMode;
        QString fullText;

        void elide()
        {
            label->QLabel::setText(label->fontMetrics().elidedText(fullText, elideMode, label->width() - (label->margin() * 2)));
        }
    };




    ElidedLabel::ElidedLabel(QWidget * parent, Qt::WindowFlags f)
        : QLabel(parent, f), d(new ElidedLabelPrivate(this))
    {}

    ElidedLabel::ElidedLabel(const QString & text, QWidget * parent, Qt::WindowFlags f)
        : QLabel(parent, f), d(new ElidedLabelPrivate(this, text))
    {}

    ElidedLabel::~ElidedLabel()
    {}

    Qt::TextElideMode ElidedLabel::elideMode() const
    {
        return d->elideMode;
    }

    void ElidedLabel::resizeEvent(QResizeEvent * e)
    {
        d->elide();
        QLabel::resizeEvent(e);
    }

    void ElidedLabel::setText(const QString & text)
    {
        d->fullText = text;
        d->elide();
        setToolTip(d->fullText == this->text() ? QString() : d->fullText);
    }

    void ElidedLabel::setTextElideMode(Qt::TextElideMode mode)
    {
        if (d->elideMode != mode) {
            d->elideMode = mode;
            d->elide();
        }
    }

} // namespace Utopia
