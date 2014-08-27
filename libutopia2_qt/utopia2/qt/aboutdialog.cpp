/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
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

#include <utopia2/qt/aboutdialog.h>

#include "version_p.h"
#include <utopia2/global.h>

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

namespace Utopia
{
    AboutDialog::AboutDialog(QWidget * parent) : QDialog(parent)
    {
        resize(350, 300);
        setContentsMargins(0, 0, 0, 0);
        setAttribute(Qt::WA_MacAlwaysShowToolWindow);
        setWindowTitle("Utopia Documents");

        QVBoxLayout * layout = new QVBoxLayout(this);
        _centralWidget = new QWidget;

        QLabel * icon = new QLabel;
        icon->setAlignment(Qt::AlignCenter);
        icon->setPixmap(QPixmap(":images/UtopiaDocuments-128.png"));

        QLabel * label = new QLabel;
        label->setAlignment(Qt::AlignCenter);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
        label->setOpenExternalLinks(true);
        label->setTextFormat(Qt::RichText);
        label->setText(QString("Version %1<br/><br/>Copyright &copy; 2008-" UTOPIA_CURRENT_YEAR "<br/>Lost Island Labs<br/><br/><a style=\"color: #008; link: '#008'; text: '#008';\" href='http://utopiadocs.com/redirect.php?to=acknowledgements'>Acknowledgements</a>").arg(Utopia::versionString()));
        QVBoxLayout * iconLayout = new QVBoxLayout;
        iconLayout->addSpacing(10);
        iconLayout->addWidget(icon, Qt::AlignCenter);
        iconLayout->addWidget(label, Qt::AlignCenter);

        _centralWidget->setLayout(iconLayout);
        layout->addWidget(_centralWidget);
    }

} // namespace Utopia
