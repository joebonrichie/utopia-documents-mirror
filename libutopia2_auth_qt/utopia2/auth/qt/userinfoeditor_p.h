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

#ifndef KEND_USERINFOEDITOR_P_H
#define KEND_USERINFOEDITOR_P_H

#include <utopia2/auth/qt/config.h>
#include <utopia2/auth/user.h>

#include <QEventLoop>
#include <QObject>
#include <QPointer>
#include <boost/scoped_ptr.hpp>

class QComboBox;
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
    class UserInfoEditor;
    class UserLabel;

    class UserInfoEditorPrivate : public QObject
    {
        Q_OBJECT

    public:
        UserInfoEditorPrivate(UserInfoEditor * editor = 0);

        UserInfoEditor * editor;
        boost::scoped_ptr< User > user;
        QPointer< Service > service;

        QComboBox * title;
        QLineEdit * forename;
        QLineEdit * surname;
        QLineEdit * institution;

        QGroupBox * emailGroup;
        QLabel * emailDisclaimer;
        QLineEdit * email;
        QLabel * emailErrorIcon;
        QLabel * emailErrorText;
        QLineEdit * confirmPassword;
        QLabel * confirmPasswordErrorIcon;
        QLabel * confirmPasswordErrorText;
        QLabel * emailChangeNotice;

        QGroupBox * passwordGroup;
        QLabel * passwordDisclaimer;
        QLineEdit * oldPassword;
        QLabel * oldPasswordErrorIcon;
        QLabel * oldPasswordErrorText;
        QLineEdit * newPassword1;
        QLineEdit * newPassword2;
        QLabel * newPasswordErrorIcon;
        QLabel * newPasswordErrorText;

        Utopia::Spinner * spinner;

        UserLabel * avatar;

        QEventLoop commitLoop;

        QString emailChangeNoticeTemplate;

        void resize();

    protected:
        bool commit(bool includeAvatar = true);
        void informEmailChange();
        void informEmailChangeCancel();
        void setEmailError(const QString & msg);
        void setConfirmPasswordError(const QString & msg);
        void setNewPasswordError(bool set = true);
        void setOldPasswordError(bool set = true);

    public slots:
        void onCancelButtonClicked();
        void onOkButtonClicked();
        void onUserInfoChanged();
        void reset(Service * service = 0, const QString & id = QString());

    protected slots:
        void onCommitCompleted(bool success);
        void onLinkActivated(const QString & link);

        void pickAvatar();
    };

} // namespace Kend

#endif // KEND_USERINFOEDITOR_P_H
