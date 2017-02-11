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

#include <papyro/playercontrols.h>

#include <QHBoxLayout>
#include <QToolButton>

#include <QPainter>

namespace Papyro
{

    PlayerControls::PlayerControls(QWidget *parent) : QWidget(parent)
    {
        this->_layout = new QHBoxLayout;

        this->setLayout(this->_layout);
        this->_layout->setContentsMargins(10,5,10,5);

        this->_playMode = true;

        this->_playpauseButton = new QToolButton();
        this->_playpauseButton->setStyleSheet(  "QToolButton {"
                                                "    border-image: url(:/icons/play.png);"
                                                " }"
                                                ""
                                                "QToolButton:pressed {"
                                                "    border-image: url(:/icons/play-pressed.png);"
                                                " }"
            );

        this->_playpauseButton->setFixedSize(20,20);
        this->_layout->addWidget(this->_playpauseButton);

        this->_launchButton = new QToolButton();
        this->_launchButton->setStyleSheet(  "QToolButton {"
                                             "    border-image: url(:/icons/launch.png);"
                                             " }"
                                             ""
                                             "QToolButton:pressed {"
                                             "    border-image: url(:/icons/launch-pressed.png);"
                                             " }"
            );

        this->_launchButton->setFixedSize(20,20);
        this->_layout->addWidget(this->_launchButton);


        this->_magnifyButton = new QToolButton();
        this->_magnifyButton->setStyleSheet(  "QToolButton {"
                                              "    border-image: url(:/icons/magnify.png);"
                                              " }"
                                              ""
                                              "QToolButton:pressed {"
                                              "    border-image: url(:/icons/magnify-pressed.png);"
                                              " }"
            );

        this->_magnifyButton->setFixedSize(20,20);
        this->_layout->addWidget(this->_magnifyButton);
        this->_magnifyButton->hide();

        connect(this->_playpauseButton, SIGNAL(clicked()), this, SLOT(togglePlayPause()));
        connect(this->_magnifyButton, SIGNAL(clicked()), this, SIGNAL(magnifyClicked()));
        connect(this->_launchButton, SIGNAL(clicked()), this, SIGNAL(launchClicked()));

        setMinimumHeight(30);
        setMaximumHeight(30);
    }

    void PlayerControls::addControl(QToolButton *newButton)
    {
        this->_layout->addWidget(newButton);
    }

    void PlayerControls::removeControl(QToolButton *button)
    {
        this->_layout->removeWidget(button);
    }

    void PlayerControls::togglePlayPause()
    {
        this->_playMode = !this->_playMode;
        if (!this->_playMode)
        {
            Q_EMIT playClicked();
            this->_playpauseButton->setStyleSheet(  "QToolButton {"
                                                    "    border-image: url(:/icons/pause.png);"
                                                    " }"
                                                    ""
                                                    "QToolButton:pressed {"
                                                    "    border-image: url(:/icons/pause-pressed.png);"
                                                    " }"
                );
        }
        else
        {
            Q_EMIT pauseClicked();
            this->_playpauseButton->setStyleSheet(  "QToolButton {"
                                                    "    border-image: url(:/icons/play.png);"
                                                    " }"
                                                    ""
                                                    "QToolButton:pressed {"
                                                    "    border-image: url(:/icons/play-pressed.png);"
                                                    " }"
                );
        }

        this->update();
    }


    QPoint PlayerControls::launchPos() const
    {
        return this->_launchButton->mapToGlobal(this->_launchButton->rect().bottomLeft());
    }

    QPoint PlayerControls::playPos() const
    {
        return this->_playpauseButton->mapToGlobal(this->_playpauseButton->rect().bottomLeft());
    }

    void PlayerControls::paintEvent(QPaintEvent * /* event */)
    {
        QPainter painter(this);

        QPainterPath roundedRect;
        roundedRect.addRoundedRect(this->rect(), 5,5);

        QPainterPath topRect;
        topRect.addRect(this->rect().x(), this->rect().y(), this->rect().width(), this->rect().height() * (1.0/3.0));


        QPainterPath topHalf = roundedRect.intersected(topRect);
        QPainterPath bottomHalf = roundedRect.subtracted(topRect);

        QLinearGradient topGrad(topHalf.boundingRect().topLeft(), topHalf.boundingRect().bottomLeft());
        topGrad.setColorAt(0, QColor(70,70,70));
        topGrad.setColorAt(1, QColor(44,44,44));

        QLinearGradient bottomGrad(topHalf.boundingRect().topLeft(), topHalf.boundingRect().bottomLeft());
        bottomGrad.setColorAt(0, QColor(30,30,30));
        bottomGrad.setColorAt(1, QColor(20,20,20));

        painter.setBrush(topGrad);
        painter.drawPath(topHalf);
        painter.setBrush(bottomGrad);
        painter.drawPath(bottomHalf);
        painter.setPen(QColor(36,36,36));
        painter.drawLine(topHalf.boundingRect().bottomLeft(), topHalf.boundingRect().bottomRight());
    }

}
