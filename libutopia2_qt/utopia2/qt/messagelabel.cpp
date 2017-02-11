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

#include <utopia2/qt/messagelabel.h>

#include <QApplication>

namespace Utopia
{
    MessageLabel::MessageLabel(QWidget * parent)
        : QLabel(parent)
    {
        /*
          this->setStyleSheet("QLabel#positive \
          {\
          font: 9px;\
          color: #11ea11;\
          background-color: none; \
          } \
          QLabel#negative \
          {\
          font: 9px;\
          color: red;\
          background-color: none; \
          } \
          QLabel#warning \
          {\
          color: red;\
          font: 9px;\
          background-color: none; \
          }\
          QLabel#message \
          {\
          color: #11ea11;\
          font: 9px;\
          background-color: none; \
          }");
        */
        connect(&_timer, SIGNAL(timeout()), this, SLOT(update()));
    }

    void MessageLabel::setInfoText(const QString & text, bool positive)
    {
        if (positive)
        {
            setObjectName("positive");
        }
        else
        {
            setObjectName("negative");
        }
        this->setStyle(QApplication::style());
        this->setText(text);
        this->show();
        emit updateStack(1);

        _timer.start(8000);
    }

    void MessageLabel::update()
    {
        this->setText("");
        emit updateStack(0);
        this->hide();
    }

}
