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

#include "hyperlinking.h"

#include <papyro/documentview.h>
#include <papyro/papyrotab.h>
#include <papyro/papyrowindow.h>
#include <papyro/utils.h>

#include <QApplication>
#include <QClipboard>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPainter>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>

#include <QtDebug>




/////////////////////////////////////////////////////////////////////////////////////////
/// HyperlinkDialog

HyperlinkDialog::HyperlinkDialog(QWidget * parent, Qt::WindowFlags f)
    : QDialog(parent, f)
{
    setWindowTitle("Make Hyperlink...");

    QGroupBox * groupBox = new QGroupBox(tr("Hyperlink URL"));
    QVBoxLayout * boxLayout = new QVBoxLayout(groupBox);
    boxLayout->addStrut(512);
    boxLayout->setContentsMargins(8, 8, 8, 8);
    _urlLineEdit = new QLineEdit;
    //_urlLineEdit->setPlaceholderText("http://www.example.com/");
    boxLayout->addWidget(_urlLineEdit);

    QHBoxLayout * buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
    _statusLabel = new QLabel;
    _statusLabel->hide();
    buttonLayout->addWidget(_statusLabel);
    _spinner = new Utopia::Spinner;
    _spinner->hide();
    buttonLayout->addWidget(_spinner);
    _cancelButton = new QPushButton("Cancel");
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    buttonLayout->addWidget(_cancelButton);
    _stopButton = new QPushButton("Stop");
    _stopButton->hide();
    connect(_stopButton, SIGNAL(clicked()), this, SLOT(cancel()));
    buttonLayout->addWidget(_stopButton);
    _okButton = new QPushButton("OK");
    _okButton->setDefault(true);
    connect(_okButton, SIGNAL(clicked()), this, SLOT(verify()));
    buttonLayout->addWidget(_okButton);

    QVBoxLayout * dialogLayout = new QVBoxLayout(this);
    dialogLayout->addWidget(groupBox);
    dialogLayout->addStretch(1);
    dialogLayout->addLayout(buttonLayout);
    dialogLayout->setSpacing(8);
    dialogLayout->setContentsMargins(8, 8, 8, 8);
}

HyperlinkDialog::~HyperlinkDialog()
{}

void HyperlinkDialog::cancel()
{
    setMessage(QString());
    setSpinning(false);
    _okButton->setEnabled(true);
    _cancelButton->show();
    _stopButton->hide();
}

QNetworkReply * HyperlinkDialog::get(const QNetworkRequest & request)
{
    QNetworkReply * reply = networkAccessManager()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(finished()));
    connect(reply, SIGNAL(sslErrors(const QList< QSslError > &)), this, SLOT(sslErrors(const QList< QSslError > &)));
    return reply;
}

void HyperlinkDialog::finished()
{
    static int redirects = 0; // Count the number of redirects

    QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
    reply->deleteLater();

    QUrl redirectedUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectedUrl.isValid()) {
        if (redirectedUrl.isRelative()) {
            QString redirectedAuthority = redirectedUrl.authority();
            redirectedUrl = reply->url().resolved(redirectedUrl);
            if (!redirectedAuthority.isEmpty()) {
                redirectedUrl.setAuthority(redirectedAuthority);
            }
        }
        if (redirects++ < 4) {
            QNetworkRequest request = reply->request();
            request.setUrl(redirectedUrl);
            get(request);
            return;
        } else {
            setError("Cannot resolve URL (too many redirects)");
        }
    }

    redirects = 0;

    // Success? Failure?
    if (!redirectedUrl.isValid()) {
        QNetworkReply::NetworkError error = reply->error();
        switch (error) {
        case QNetworkReply::NoError:
            setMessage("Saving...", true);
            emit verified();
            break;
        case QNetworkReply::TimeoutError:
        case QNetworkReply::OperationCanceledError:
            setError("Connection timed out");
            break;
        case QNetworkReply::HostNotFoundError:
            setError("Host not found");
            break;
        case QNetworkReply::ContentNotFoundError:
            setError("Content not found");
            break;
        case QNetworkReply::SslHandshakeFailedError:
            break;
        default:
            setError(QString("Cannot resolve hyperlink (%1)").arg(reply->error()));
            break;
        }
    }

    _urlLineEdit->setEnabled(true);
}

void HyperlinkDialog::reset(const QString & url)
{
    _urlLineEdit->setText(url);
    _urlLineEdit->setEnabled(true);
    setMessage(QString());
    _cancelButton->show();
    _stopButton->hide();
    _okButton->setEnabled(true);
}

void HyperlinkDialog::setError(const QString & error)
{
    setMessage("<font color=red><strong>" + error + "</strong></font>");
    setSpinning(false);

    _okButton->setEnabled(true);
    _cancelButton->show();
    _stopButton->hide();
}

void HyperlinkDialog::setMessage(const QString & msg, bool spinning)
{
    if (msg.isEmpty()) {
        _statusLabel->hide();
    } else {
        _statusLabel->setText(msg);
        _statusLabel->show();
    }

    setSpinning(spinning);
}

void HyperlinkDialog::setSpinning(bool spinning)
{
    if (spinning) {
        _spinner->start();
        _spinner->show();
    } else {
        _spinner->stop();
        _spinner->hide();
    }
}

void HyperlinkDialog::sslErrors(const QList< QSslError > & errors)
{
    QNetworkReply * reply = static_cast< QNetworkReply * >(sender());
    bool ignore = false;
    QStringList errorStrings;
    foreach(const QSslError & error, errors) {
        switch (error.error())
        {
        case QSslError::SelfSignedCertificate:
        case QSslError::SelfSignedCertificateInChain:
        case QSslError::CertificateUntrusted:
            ignore = true;
            break;
        default:
            errorStrings << QString("\"%1\"").arg(error.errorString());
            break;
        }
    }
    if (errorStrings.size() > 0) {
        QString title;
        QString question = "Would you still like to save this hyperlink?";
        if (errorStrings.size() == 1) {
            title = "An SSL error occurred...";
        } else {
            title = "Some SSL errors occurred...";
        }
        QMessageBox::StandardButton action =
            QMessageBox::warning(this,
                                 title,
                                 QString("<span>%1</span><br/><br/><strong>%2</strong>").arg(errorStrings.join("<br/>")).arg(question),
                                 QMessageBox::Save | QMessageBox::Discard,
                                 QMessageBox::Discard);
        ignore = (action == QMessageBox::Save);
    }
    if (ignore) reply->ignoreSslErrors();
    else setError("");
}

void HyperlinkDialog::verify()
{
    QString url(_urlLineEdit->text());
    if (QUrl(url).isValid() && (url.startsWith("http://", Qt::CaseInsensitive) || url.startsWith("https://", Qt::CaseInsensitive))) {
        setMessage("Verifying...", true);
        _cancelButton->hide();
        _stopButton->show();
        _okButton->setEnabled(false);
        _urlLineEdit->setEnabled(false);
        get(QNetworkRequest(_urlLineEdit->text()));
    } else {
        setError("Invalid URL (must be HTTP or HTTPS)");
    }
}

QString HyperlinkDialog::url() const {
    return _urlLineEdit->text();
}




/////////////////////////////////////////////////////////////////////////////////////////
/// HyperlinkFactory

HyperlinkFactory::HyperlinkFactory()
    : QObject(0), Papyro::SelectionProcessor(), Papyro::AnnotationProcessor()
{}

HyperlinkFactory::~HyperlinkFactory()
{}

void HyperlinkFactory::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    if (!annotations.empty()) {
        Spine::AnnotationHandle annotation = *annotations.begin();
        QUrl url(Papyro::qStringFromUnicode(annotation->getFirstProperty("property:webpageUrl")));
        QString target(Papyro::qStringFromUnicode(annotation->getFirstProperty("property:webpageUrlTarget")));
        QString anchor(Papyro::qStringFromUnicode(annotation->getFirstProperty("property:destinationAnchorName")));
        if (!anchor.isEmpty()) {
            target = QString("pdf; anchor=%1; ").arg(anchor) + target;
        }
        Papyro::PapyroWindow::currentWindow()->requestUrl(url, target);
    }
}

bool HyperlinkFactory::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return annotation->hasProperty("property:webpageUrl");
}

int HyperlinkFactory::category() const
{
    return 10;
}

QIcon HyperlinkFactory::icon() const
{
    return generateFromMonoPixmap(QPixmap(":/processors/hyperlinking/icon.png"));
}

QIcon HyperlinkFactory::icon(Spine::DocumentHandle document, Spine::AnnotationHandle /*annotation*/) const
{
    return QIcon();
}

void HyperlinkFactory::processSelection(Spine::DocumentHandle document, Spine::CursorHandle, const QPoint & globalPos)
{
    const static QRegExp whitespaceRX("\\s+");
    const static QRegExp schemeRX("^(http(s)?://).*", Qt::CaseInsensitive);
    const static QRegExp urlRX("^(http(s)?://)?[\\w-]+(\\.[\\w-]+)+(:[0-9]+)?(/.*)?", Qt::CaseInsensitive);
    const static QRegExp trailingDotsRX("\\.*$");

    QString text = Papyro::qStringFromUnicode(document->selectionText());
    text = text.remove(whitespaceRX);
    if (urlRX.exactMatch(text)) {
        if (!schemeRX.exactMatch(text)) {
            text = "http://" + text;
        }
        text = text.remove(trailingDotsRX);
        if (text.count('/') < 3) {
            text += '/';
        }
    } else {
        text = QString();
    }

    if (!hyperlinkDialog) {
        hyperlinkDialog = new HyperlinkDialog;
        connect(hyperlinkDialog.data(), SIGNAL(verified()), this, SLOT(submit()));
    }

    // Ask the user for a hyperlink
    hyperlinkDialog->reset(text);
    hyperlinkDialog->exec();
}

QList< boost::shared_ptr< Papyro::SelectionProcessor > > HyperlinkFactory::selectionProcessors(Spine::DocumentHandle document, Spine::CursorHandle cursor)
{
    QList< boost::shared_ptr< Papyro::SelectionProcessor > > list;
    if (hasTextSelection(document, cursor) || hasAreaSelection(document, cursor)) {
        list << boost::shared_ptr< Papyro::SelectionProcessor >(new HyperlinkFactory);
    }
    return list;
}

void HyperlinkFactory::submit()
{

}

QString HyperlinkFactory::title() const
{
    return "Make Hyperlink...";
}

QString HyperlinkFactory::title(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    QUrl url(Papyro::qStringFromUnicode(annotation->getFirstProperty("property:webpageUrl")));
    if (url.scheme() == "mailto") {
        return "Send Email...";
    }
    return "Open Hyperlink...";
}

int HyperlinkFactory::weight() const
{
    return 16;
}




/////////////////////////////////////////////////////////////////////////////////////////
/// MailToFactory

MailToFactory::MailToFactory()
    : Papyro::AnnotationProcessor()
{}

MailToFactory::~MailToFactory()
{}

void MailToFactory::activate(Spine::DocumentHandle document, Spine::AnnotationSet annotations, const QPoint & globalPos)
{
    if (!annotations.empty()) {
        Spine::AnnotationHandle annotation = *annotations.begin();
        QString address = Papyro::qStringFromUnicode(annotation->getFirstProperty("property:webpageUrl").substr(7));
        if (!address.isEmpty()) {
            QClipboard * clipboard = QApplication::clipboard();
            clipboard->setText(address, QClipboard::Clipboard);
        }
    }
}

bool MailToFactory::canActivate(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return annotation->hasProperty("property:webpageUrl") && annotation->getFirstProperty("property:webpageUrl").substr(0, 7) == "mailto:";
}

int MailToFactory::category() const
{
    return 10;
}

QString MailToFactory::title(Spine::DocumentHandle document, Spine::AnnotationHandle annotation) const
{
    return "Copy Email Address";
}

int MailToFactory::weight() const
{
    return 17;
}




/////////////////////////////////////////////////////////////////////////////////////////
/// HyperlinkRenderer

QMap< int, QPainterPath > HyperlinkRenderer::bounds(Spine::DocumentHandle document, Spine::AnnotationHandle annotation)
{
    //QMap< int, QPainterPath > paths(getRoundedPathsForAreas(annotation));
    QMap< int, QPainterPath > paths(getPathsForAreas(annotation));
    //QMapIterator< int, QPainterPath > a_iter(getRoundedPathsForText(annotation));
    QMapIterator< int, QPainterPath > a_iter(getPathsForText(annotation));
    while (a_iter.hasNext()) {
        a_iter.next();
        paths[a_iter.key()].addPath(a_iter.value());
    }
    return paths;
}

QString HyperlinkRenderer::id()
{
    return "hyperlink";
}

QMap< int, QPicture > HyperlinkRenderer::render(Spine::DocumentHandle document, const Spine::AnnotationSet & annotations, State state)
{
    QColor defaultColor(0, 0, 180);
    QMap< int, QPicture > pictures;
    QMap< int, QPicture > multiPictures;
    QHash< QString, Spine::AnnotationSet > groups;

    // Group by colour
    foreach (Spine::AnnotationHandle annotation, annotations) {
        QString colorStr(Papyro::qStringFromUnicode(annotation->getFirstProperty("property:color")));
        if (colorStr.isEmpty()) {
            colorStr = Papyro::qStringFromUnicode(annotation->getFirstProperty("session:color"));
        }
        QColor color(colorStr);
        if (!color.isValid()) {
            color = defaultColor;
        }
        groups[color.name()].insert(annotation);
    }

    QHashIterator< QString, Spine::AnnotationSet > iter(groups);
    while (iter.hasNext()) {
        iter.next();
        QColor color(iter.key());

        QMap< int, QPicture > pictures;
        QMap< int, QPainterPath > textBounds(getPathsForText(iter.value()));
        QMap< int, QPainterPath > areaBounds(getPathsForAreas(iter.value()));
        QSet< int > pages = QSet< int >::fromList(textBounds.keys()) + QSet< int >::fromList(areaBounds.keys());
        foreach (int page, pages) {
            QPainter painter(&pictures[page]);
            painter.setBrush(color);
            painter.setPen(color);
            painter.setCompositionMode(QPainter::CompositionMode_Screen);
            painter.drawPath(areaBounds[page]);
            painter.drawPath(textBounds[page]);
            if (state == Hover) {
                painter.setCompositionMode(QPainter::CompositionMode_Multiply);
                painter.setOpacity(0.04);
                painter.setPen(Qt::NoPen);
                painter.drawPath(areaBounds[page]);
                painter.drawPath(textBounds[page]);
            }
        }

        multiPictures.unite(pictures);
    }

    foreach (int page, multiPictures.keys()) {
        QPainter painter(&pictures[page]);
        foreach (QPicture picture, multiPictures.values(page)) {
            painter.drawPicture(0, 0, picture);
        }
    }

    return pictures;
}

int HyperlinkRenderer::weight()
{
    return 10;
}
