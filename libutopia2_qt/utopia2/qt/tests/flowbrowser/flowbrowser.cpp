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

#include "flowbrowser.h"

#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QImage>
#include <QTimer>

#include <QDebug>

QStringList findFiles(const QString& path = QString())
{
    QStringList files;

    QDir dir = QDir::current();
    if(!path.isEmpty())
        dir = QDir(path);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
#if QT_VERSION >= 0x040000
    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i)
    {
        QFileInfo fileInfo = list.at(i);
        files.append(dir.absoluteFilePath(fileInfo.fileName()));
    }
#else
    const QFileInfoList* list = dir.entryInfoList();
    if(list)
    {
        QFileInfoListIterator it( *list );
        QFileInfo * fi;
        while( (fi=it.current()) != 0 )
        {
            ++it;
            files.append(dir.absFilePath(fi->fileName()));
        }
    }
#endif

    return files;
}

Browser::Browser(const QStringList & files, QWidget * parent)
    : Utopia::FlowBrowser(parent)
{
    QImage img;
    int j = 0;
    for(int i = 0; i < (int)files.count(); i++)
    {
        if(img.load(files[i]) && !img.isNull())
        {
            append(j);
            images[j++] = img;
        }
    }
    connect(this, SIGNAL(requiresUpdate(int)), this, SLOT(queueImageUpdate(int)));
    connect(this, SIGNAL(selected(int)), this, SLOT(printMsg(int)));
    QTimer::singleShot(300, this, SLOT(updateImage()));
}

void Browser::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Right)
    {
        next();
    }
    else if (event->key() == Qt::Key_Left)
    {
        previous();
    }
    else if (event->key() == Qt::Key_PageDown)
    {
        next(10);
    }
    else if (event->key() == Qt::Key_PageUp)
    {
        previous(10);
    }
    else if (event->key() == Qt::Key_Home)
    {
        goTo(0);
    }
    else if (event->key() == Qt::Key_End)
    {
        goTo(count() - 1);
    }
    else
    {
        FlowBrowser::keyPressEvent(event);
    }
}

void Browser::queueImageUpdate(int i)
{
    queue.append(i);
}

void Browser::updateImage()
{
    while (!queue.isEmpty())
    {
        int i = queue.front();
        queue.pop_front();
        update(i, images[i]);
    }

    QTimer::singleShot(300, this, SLOT(updateImage()));
}

void Browser::printMsg(int i)
{
    qDebug() << "Selected:" << i;
}


int main (int argc, char** argv)
{
    QApplication a(argc, argv);

    QStringList files = (argc > 1) ? findFiles(QString(argv[1])) : findFiles();


    Browser * f = new Browser(files);
    f->setGeometry(100, 100, 1000, 400);
    f->show();

    a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
    int result = a.exec();

    delete f;

    return result;
}
