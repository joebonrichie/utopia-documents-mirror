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

#include <utopia2/auth/qt/resetpassworddialog_p.h>
#include <utopia2/auth/qt/resetpassworddialog.h>
#include <utopia2/auth/service.h>
#include <utopia2/qt/spinner.h>

#include <QEventLoop>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QUrlQuery>

#include <QDebug>

namespace Kend
{

    ResetPasswordDialogPrivate::ResetPasswordDialogPrivate(ResetPasswordDialog * dialog)
        : QObject(dialog), dialog(dialog)
    {
        dialog->setFixedWidth(500);

        QVBoxLayout * dialogLayout = new QVBoxLayout(dialog);
        dialogLayout->setSpacing(0);
        dialogLayout->setContentsMargins(0, 0, 0, 0);

        gridLayout = new QGridLayout;
        gridLayout->setSpacing(8);
        gridLayout->setContentsMargins(12, 12, 12, 0);
        dialogLayout->addLayout(gridLayout);

        QLabel * msgLabel = new QLabel;
        msgLabel->setText(
            "To reset your password, you must provide your registered email address. "
            "Instructions on how to choose a new password will then be sent to you."
        );
        msgLabel->setWordWrap(true);
        gridLayout->addWidget(msgLabel, 0, 0, 1, 3);

        QLabel * emailLabel = new QLabel("Email address:");
        gridLayout->addWidget(emailLabel, 1, 0, Qt::AlignRight);
        email = new QLineEdit;
        gridLayout->addWidget(email, 1, 1);
        emailErrorIcon = new QLabel;
        emailErrorIcon->setFixedSize(20, 20);
        gridLayout->addWidget(emailErrorIcon, 1, 2, Qt::AlignCenter);

        emailErrorText = new QLabel;
        emailErrorText->setObjectName("errorLabel");
        emailErrorText->setWordWrap(true);
        emailErrorText->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        emailErrorText->hide();
        gridLayout->addWidget(emailErrorText, 2, 1);

        /////////////////////////////////////////////////////////////////////////////////
        // Control buttons
        QHBoxLayout * buttonLayout = new QHBoxLayout;
        buttonLayout->setContentsMargins(12, 12, 12, 12);
        buttonLayout->setSpacing(8);
        QPushButton * sendButton = new QPushButton("Send");
        sendButton->setDefault(true);
        connect(sendButton, SIGNAL(clicked()), this, SLOT(onSendButtonClicked()));
        QPushButton * cancelButton = new QPushButton("Cancel");
        cancelButton->setDefault(false);
        connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelButtonClicked()));
        spinner = new Utopia::Spinner;
        spinner->setFixedSize(18, 18);
        buttonLayout->addWidget(cancelButton, 0);
        buttonLayout->addStretch(1);
        buttonLayout->addWidget(spinner, 0);
        buttonLayout->addWidget(sendButton, 0);
        dialogLayout->addStretch(1);
        dialogLayout->addLayout(buttonLayout, 0);

        // Layout
        gridLayout->setColumnStretch(0, 0);
        gridLayout->setColumnStretch(1, 1);
        gridLayout->setColumnStretch(2, 0);

        dialog->setFixedHeight(dialog->heightForWidth(500));
        dialog->setSizeGripEnabled(false);
    }

    bool ResetPasswordDialogPrivate::commit()
    {
        static const QString notFoundMsg("This email address is not registered with Utopia.");
        static const QString networkErrorMsg(
            "A network error has occurred that has prevented your request from being "
            "processed. Please check your network connectivity and try again."
        );
        static const QString unknownErrorMsg(
            "Oops! Something went wrong when asking our servers to begin the process of "
            "resetting your password. Please try again."
        );

        bool error = false;
        if (service) {
            dialog->setEnabled(false);
            spinner->start();

            // Create a request URI
            QUrl url(service->resourceUrl(Service::AuthenticationResource));
            url.setPath(url.path() + "/backends/" + service->authenticationMethod() + "/request-password-reset");
            QUrlQuery query(url.query());
            query.addQueryItem("user_id", email->text().replace("+", "%2B"));
            url.setQuery(query);

            QEventLoop commitLoop;
            QNetworkReply * reply = service->post(QNetworkRequest(url));
            connect(reply, SIGNAL(finished()), &commitLoop, SLOT(quit()));
            error = commitLoop.exec() != 0;
            reply->deleteLater();

            // Success?
            switch (reply->error()) {
            case QNetworkReply::ContentNotFoundError: // 404 Not Found
                setEmailError(notFoundMsg);
                error = true;
                break;
            case QNetworkReply::ConnectionRefusedError: // Network errors
            case QNetworkReply::RemoteHostClosedError:
            case QNetworkReply::HostNotFoundError:
            case QNetworkReply::TimeoutError:
            case QNetworkReply::TemporaryNetworkFailureError:
            case QNetworkReply::UnknownNetworkError:
                QMessageBox::warning(dialog, "Network error", networkErrorMsg);
                error = true;
                break;
            case QNetworkReply::NoError:
                // SUCCESS
                break;
            default: // some other error!?
                QMessageBox::warning(dialog, "Unexpected error", unknownErrorMsg);
                error = true;
                break;
            }

            spinner->stop();
            dialog->setEnabled(true);
        }
        return !error;
    }

    void ResetPasswordDialogPrivate::informEmailSent()
    {
        static QString title("Password reset requested");
        static QString msg(
            "You will shortly receive an email with instructions on how "
            "to reset your password."
        );
        QMessageBox::information(dialog, title, msg);
    }

    void ResetPasswordDialogPrivate::onCancelButtonClicked()
    {
        // Reset error labels
        setEmailError(QString());
        resize();
        dialog->reject();
    }

    void ResetPasswordDialogPrivate::onSendButtonClicked()
    {
        static const QRegExp emailRegExp("[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*@(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?");
        static const QString invalidEmailErrorText("You must enter a valid email address");

        if (service) {
            // Reset error labels
            setEmailError(QString());

            resize();

            // Apply changes
            bool error = false;
            // Email address - MANDATORY
            QString emailText(email->text());
            if (emailText.isEmpty() || !emailRegExp.exactMatch(emailText)) {
                setEmailError(invalidEmailErrorText);
                error = true;
            }

            resize();

            if (!error && commit()) {
                informEmailSent();
                dialog->accept();
            }
        }

        resize();
    }

    void ResetPasswordDialogPrivate::reset(Service * service)
    {
        this->service = service;

        dialog->setEmail(QString());
    }

    void ResetPasswordDialogPrivate::resize()
    {
        gridLayout->activate();
        dialog->layout()->activate();
        dialog->setFixedHeight(dialog->heightForWidth(500));
    }

    void ResetPasswordDialogPrivate::setEmailError(const QString & msg)
    {
        emailErrorIcon->setPixmap(msg.isEmpty() ? QPixmap() : QPixmap(":/icons/error-cross.png"));
        emailErrorText->setText(msg);
        emailErrorText->setVisible(!msg.isEmpty());
    }




    ResetPasswordDialog::ResetPasswordDialog(QWidget * parent)
        : QDialog(parent), d(new ResetPasswordDialogPrivate(this))
    {
        setWindowTitle("Reset a lost password");
    }

    void ResetPasswordDialog::clear()
    {
        d->reset();
    }

    void ResetPasswordDialog::setEmail(const QString & email)
    {
        d->setEmailError(QString());
        d->email->setText(email);
        d->resize();
    }

    void ResetPasswordDialog::setService(Service * service)
    {
        d->reset(service);
    }

    void ResetPasswordDialog::showEvent(QShowEvent * e)
    {
        d->resize();
        QDialog::showEvent(e);
    }

} // namespace Kend
