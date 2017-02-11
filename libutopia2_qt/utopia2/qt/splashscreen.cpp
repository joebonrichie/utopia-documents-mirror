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

#include <utopia2/qt/splashscreen.h>

#include <utopia2/global.h>

#include <QtDebug>
#include <QPainter>

namespace Utopia
{

    /**
     *  \brief SplashScreen constructor.
     */
    SplashScreen::SplashScreen(const QPixmap& pixmap_, Qt::WindowFlags f_)
        : QSplashScreen(pixmap_, f_), _color(Qt::black), _alignment(Qt::AlignCenter + Qt::AlignBottom)
    {
        this->changeMessage("Initialising Utopia...");
    }

    /**
     *  \brief SplashScreen constructor.
     */
    SplashScreen::SplashScreen(QWidget* parent_, const QPixmap& pixmap_, Qt::WindowFlags f_)
        : QSplashScreen(parent_, pixmap_, f_), _color(Qt::black), _alignment(Qt::AlignCenter + Qt::AlignBottom)
    {
        this->changeMessage("Initialising Utopia...");
    }

    /**
     *  \brief Update SplashScreen message.
     */
    void SplashScreen::changeMessage(const QString& message_)
    {
        this->showMessage(message_.trimmed(), this->_alignment, this->_color);
    }

    void SplashScreen::drawContents(QPainter * painter)
    {
        this->QSplashScreen::drawContents(painter);

        painter->save();
        painter->setRenderHint(QPainter::TextAntialiasing);
        QFontMetrics fm = painter->fontMetrics();

        painter->setPen(Qt::black);
        painter->setBrush(QColor(0,0,0,0));

        float height = fm.height();

        painter->drawText(26,200, "Documents v" + Utopia::versionString());
        painter->drawText(26,200 + height + 2, "Copyright (c) 2008-2012");
        painter->drawText(26,200 + (height + 2)*2, "Lost Island Labs");
        painter->restore();

    }

    /**
     *  \brief Update SplashScreen message.
     */
    void SplashScreen::changeMessage(const QString& message_, int alignment_, const QColor& color_)
    {
        this->showMessage(message_, alignment_, color_);
    }

    /**
     *  \brief Change default message color.
     */
    void SplashScreen::setDefaultColor(const QColor& color_)
    {
        this->_color = color_;
    }

    /**
     *  \brief Change default message alignment.
     */
    void SplashScreen::setDefaultAlignment(int alignment_)
    {
        this->_alignment = alignment_;
    }

} // namespace Utopia
