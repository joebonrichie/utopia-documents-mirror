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

#ifndef Utopia_QT_SPLASHSCREEN
#define Utopia_QT_SPLASHSCREEN

#include <utopia2/qt/config.h>

#include <QColor>
#include <QPixmap>
#include <QSplashScreen>
#include <QString>

class QWidget;

namespace Utopia
{

    class LIBUTOPIA_QT_API SplashScreen : public QSplashScreen
    {
        Q_OBJECT

    public:
        // Constructor
        SplashScreen(const QPixmap& pixmap_ = QPixmap(":/images/splash.png"),
                     Qt::WindowFlags f_ = 0);
        // Constructor
        SplashScreen(QWidget* parent_,
                     const QPixmap& pixmap_ = QPixmap(":/images/splash.png"),
                     Qt::WindowFlags f_ = 0);

    public Q_SLOTS:
        // Update initialisation
        void changeMessage(const QString& message_);
        // Update initialisation
        void changeMessage(const QString& message_, int alignment_, const QColor& color_);
        // Change default text color
        void setDefaultColor(const QColor& color_);
        // Change default text alignment
        void setDefaultAlignment(int alignment_);

    protected:
        void drawContents(QPainter * painter);

    private:
        // Color of text
        QColor _color;
        // Alignment of text
        int _alignment;
    };

} // namespace Utopia

#endif // Utopia_QT_SPLASHSCREEN
