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

#include <papyro/printer.h>
#include <papyro/printer_p.h>
#include <papyro/utils.h>

#include <QDebug>
#include <QMutexLocker>
#include <QPainter>
#include <QPrintDialog>
#include <QPrinter>
#include <QProgressDialog>

namespace Papyro
{

    PrinterThread::PrinterThread(QObject * parent, Spine::DocumentHandle document, QPrinter * printer)
        : QThread(parent), document(document), cancelled(false), mutex(QMutex::Recursive), printer(printer)
    {}

    void PrinterThread::cancel()
    {
        QMutexLocker guard(&mutex);
        cancelled = true;
    }

    void PrinterThread::run()
    {
        mutex.lock();

        if (!cancelled) {

            // Which pages to print
            int step = 1;
            int fromPage = printer->printRange() == QPrinter::PageRange ? printer->fromPage() : 1;
            int toPage = printer->printRange() == QPrinter::PageRange ? printer->toPage() : document->numberOfPages();
            static const int maxResolution = 300;
            int resolution = qMin(printer->resolution(), maxResolution);

            // The order to print them in
            if (printer->pageOrder() == QPrinter::LastPageFirst) {
                step = -1;
                qSwap(fromPage, toPage);
            }

            int count = 0;
            for (int page = fromPage; page <= toPage && !cancelled; page += step) {
                mutex.unlock();
                Spine::Image image(document->newCursor(page)->page()->render(resolution));
                QImage pageImage(qImageFromSpineImage(&image));
                mutex.lock();

                emit imageGenerated(pageImage, (page == fromPage));
                emit progressChanged(++count);
            }

            if (cancelled) {
                printer->abort();
            }
        }

        mutex.unlock();
    }






    PrinterPrivate::PrinterPrivate(Printer * p)
        : QObject(p), p(p), mutex(QMutex::Recursive), painter(0), printer(0)
    {}

    PrinterPrivate::~PrinterPrivate()
    {}

    void PrinterPrivate::onFinished()
    {
        delete painter;
        painter = 0;
        delete printer;
        printer = 0;
    }

    void PrinterPrivate::onImageGenerated(QImage image, bool first)
    {
        if (!first) {
            printer->newPage();
        }

        QRect viewport(painter->viewport());
        QSize size(image.size());
        size.scale(viewport.size(), Qt::KeepAspectRatio);
        QPoint centring(qAbs(viewport.width() - size.width()) / 2.0,
                        qAbs(viewport.height() - size.height()) / 2.0);

        painter->setViewport(QRect(viewport.topLeft() + centring, size));
        painter->setWindow(image.rect());
        painter->drawImage(0, 0, image);
        painter->setViewport(viewport);
    }




    Printer::Printer(QObject * parent)
        : QObject(parent), d(new PrinterPrivate(this))
    {}

    Printer::~Printer()
    {}

    boost::shared_ptr< Printer > Printer::instance()
    {
        static boost::weak_ptr< Printer > singleton;
        boost::shared_ptr< Printer > shared(singleton.lock());
        if (singleton.expired())
        {
            shared = boost::shared_ptr< Printer >(new Printer());
            singleton = shared;
        }
        return shared;
    }

    bool Printer::print(Spine::DocumentHandle document, QWidget * parent)
    {
        if (document) {
            d->mutex.lock();

            // Create sensible default printer
            d->printer = new QPrinter(QPrinter::PrinterResolution);
            d->printer->setFullPage(true);
            d->printer->setCreator("Utopia");
            if (parent && parent->isWindow()) {
                d->printer->setDocName(parent->windowTitle());
            }

            // Provide the print dialog to the user for customisation
            QPrintDialog printDialog(d->printer, parent);
            printDialog.setWindowTitle(tr("Print Document"));
            printDialog.setOptions(QAbstractPrintDialog::PrintPageRange);
            if (printDialog.exec() == QDialog::Accepted) {
                // Open progress dialog
                int from = 0;
                int to = d->printer->printRange() == QPrinter::PageRange ? qAbs(1 + d->printer->toPage() - d->printer->fromPage()) : document->numberOfPages();
                QProgressDialog progressDialog("Printing...", "Cancel", from, to, parent);

                PrinterThread * thread = new PrinterThread(this, document, d->printer);

                connect(thread, SIGNAL(imageGenerated(QImage,bool)), d, SLOT(onImageGenerated(QImage,bool)));
                connect(thread, SIGNAL(finished()), d, SLOT(onFinished()));
                connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));

                connect(&progressDialog, SIGNAL(canceled()), thread, SLOT(cancel()));
                connect(thread, SIGNAL(progressChanged(int)), &progressDialog, SLOT(setValue(int)));
                connect(thread, SIGNAL(finished()), &progressDialog, SLOT(accept()));

                d->painter = new QPainter(d->printer);

                // Print document
                thread->start();

                d->mutex.unlock();

                return progressDialog.exec();
            }

            d->mutex.unlock();
        }

        return false;
    }

}
