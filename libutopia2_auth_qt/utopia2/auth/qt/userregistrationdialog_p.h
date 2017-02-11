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

#ifndef KEND_USERREGISTRATIONDIALOG_P_H
#define KEND_USERREGISTRATIONDIALOG_P_H

#include <utopia2/auth/qt/config.h>

#include <QEventLoop>
#include <QMap>
#include <QObject>
#include <QPointer>

class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;

namespace Utopia
{
    class Spinner;
}

namespace Kend
{

    class Service;
    class UserRegistrationDialog;

    class UserRegistrationDialogPrivate : public QObject
    {
        Q_OBJECT

    public:
        UserRegistrationDialogPrivate(UserRegistrationDialog * editor = 0);

        UserRegistrationDialog * dialog;
        QPointer< Service > service;

        QGridLayout * gridLayout;

        QComboBox * title;
        QLineEdit * forename;
        QLineEdit * surname;
        QLineEdit * institution;

        QLabel * emailDisclaimer;
        QLineEdit * email;
        QLabel * emailErrorIcon;
        QLabel * emailErrorText;

        QLabel * passwordDisclaimer;
        QLineEdit * newPassword1;
        QLineEdit * newPassword2;
        QLabel * newPasswordErrorIcon;
        QLabel * newPasswordErrorText;

        QLabel * avatar;
        bool avatarChanged;

        Utopia::Spinner * spinner;

        QEventLoop commitLoop;

        QString emailConfirmNoticeTemplate;

        void resize();

    protected:
        bool commit(const QMap< QString, QString > & values);
        bool eventFilter(QObject * obj, QEvent * e);
        void informEmailConfirmation();
        void setEmailError(const QString & msg);
        void setPasswordError(const QString & msg);
        QLabel * verticalLabel(const QString & msg, QWidget * parent);

    public slots:
        void onCancelButtonClicked();
        void onOkButtonClicked();
        void reset(Service * service = 0);

    protected slots:
        void onCommitCompleted(bool success);
        void onLinkActivated(const QString & link);

        void pickAvatar();
    };

} // namespace Kend

#endif // KEND_USERREGISTRATIONDIALOG_P_H
