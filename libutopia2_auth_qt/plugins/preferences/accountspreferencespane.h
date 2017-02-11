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

#include <utopia2/initializer.h>
#include <utopia2/qt/preferencespane.h>

#include <utopia2/qt/slidelayout.h>
#include <utopia2/auth/servicemanager.h>
#include <utopia2/auth/user.h>

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <QStyledItemDelegate>
#include <QDialog>
#include <QEventLoop>
#include <QPointer>
#include <QTabWidget>
#include <QVariantMap>

class QCheckBox;
class QComboBox;
class QGridLayout;
class QGroupBox;
class QLabel;
class QLineEdit;
class QModelIndex;
class QPushButton;
class QStackedLayout;
class QTableView;
class QToolButton;

namespace Kend
{

    class Service;
    class ServiceManagerModel;
    class UserInfoEditor;




    class NewAccountDialog : public QDialog
    {
        Q_OBJECT

    public:
        NewAccountDialog(QWidget * parent = 0, Qt::WindowFlags f = 0);

    protected:
        Utopia::SlideLayout * slideLayout;
        QPushButton * cancelButton;
        QPushButton * nextButton;
        QPushButton * previousButton;
        QPushButton * doneButton;

        struct {
            QWidget * widget;
            QLineEdit * serviceUrlLineEdit;
            QLabel * errorLabel;
        } page_one;

        struct {
            QWidget * widget;
            QLabel * serviceDescription;
            QComboBox * authenticationMethodComboBox;
        } page_two;

        struct {
            QWidget * widget;
            QLabel * serviceDescription;
            QLabel * authenticationMethod;
        } page_three;

        boost::shared_ptr< ServiceManager > serviceManager;
        QPointer< Service > newService;

    protected slots:
        void onDoneButtonClicked();
        void onNextButtonClicked();
        void onPreviousButtonClicked();
    };




    class AccountInfoPanel : public QWidget
    {
        Q_OBJECT

    public:
        AccountInfoPanel(Service * service, QWidget * parent = 0);

        QPointer< Service > service;

        bool apply();
        void discard();

    signals:
        void closeProfileDialog();
        void modified();
        void openProfileDialog(Service * service, const QString & id);

    protected slots:
        void onDescriptionChanged(const QString & text);
        void onUserNameChanged(const QString & text);
        void onPasswordChanged(const QString & text);
        void onProfileButtonClicked();
        void onAnonymousChanged(bool anonymous);
        void onLinkActivated(const QString & href);
        void onServiceStateChanged(Kend::Service::ServiceState state);

    protected:
        QGridLayout * serviceInfoLayout;

        QLabel * serviceType;
        QLabel * serviceName;
        QLabel * userNameLabel;
        QLineEdit * userName;
        QLabel * registerLabel;
        QLabel * passwordLabel;
        QLineEdit * password;
        QLabel * resetPasswordLabel;
        QLineEdit * descriptionLineEdit;
        QCheckBox * anonymousCheckBox;
        QLabel * disclaimer;
        QPushButton * profileButton;

        QEventLoop loop;

        boost::scoped_ptr< User > user;

        bool serviceNameAlternative;

        bool eventFilter(QObject * obj, QEvent * event);
        void refreshInformation();
        void setServiceName();
    };




    class AccountDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

    public:
        AccountDelegate(QObject * parent = 0);

        void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
        QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;
    };




    class AccountsPreferencesPane : public Utopia::PreferencesPane
    {
        Q_OBJECT

    public:
        AccountsPreferencesPane(QWidget * parent = 0, Qt::WindowFlags f = 0);

        bool apply();
        void discard();
        QIcon icon() const;
        bool isModified() const;
        bool isValid() const;
        QString title() const;
        int weight() const;

    protected slots:
        void onAccountInfoPanelModified();
        void onAddServiceButtonClicked();
        void onCloseProfileDialog();
        void onCurrentRowChanged(const QModelIndex & current, const QModelIndex & previous);
        void onOpenProfileDialog(Service * service, const QString & id);
        void onRemoveServiceButtonClicked();
        void onServiceAdded(Kend::Service * service);
        void onServiceRemoved(Kend::Service * service);

    protected:
        boost::shared_ptr< ServiceManager > serviceManager;
        ServiceManagerModel * serviceManagerModel;

        QTableView * tableView;
        QGroupBox * detailsGroup;
        QToolButton * addServiceButton;
        QToolButton * removeServiceButton;

        UserInfoEditor * userInfoEditor;

        QPointer< Service > selectedService;

        QStackedLayout * infoStack;
    };

}
