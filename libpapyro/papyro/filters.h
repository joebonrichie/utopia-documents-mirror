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

#ifndef ATHENAEUM_FILTERS_H
#define ATHENAEUM_FILTERS_H

#include <papyro/abstractfilter.h>

#include <QList>

namespace Athenaeum
{

    class TextFilterPrivate;
    class TextFilter : public AbstractFilter
    {
        Q_OBJECT

    public:
        TextFilter(const QRegExp & regExp, int column, int role = Qt::DisplayRole, QObject * parent = 0);
        TextFilter(const QString & string, int column, int role = Qt::DisplayRole, QObject * parent = 0);
        ~TextFilter();

        bool accepts(const QModelIndex & index) const;
        int column() const;
        QRegExp regExp() const;
        int role() const;
        void setColumn(int column);
        void setFixedString(const QString & string);
        void setRegExp(const QRegExp & regExp);
        void setRole(int role);

    protected:
        TextFilterPrivate * d;
    }; // class TextFilter




    class DateTimeFilterPrivate;
    class DateTimeFilter : public AbstractFilter
    {
        Q_OBJECT

    public:
        DateTimeFilter(QObject * parent = 0);
        ~DateTimeFilter();

        bool accepts(const QModelIndex & index) const;
        int column() const;
        QDateTime dateTimeFrom() const;
        QDateTime dateTimeTo() const;
        int role() const;
        void setColumn(int column);
        void setDateTimeFrom(const QDateTime & dateTimeFrom);
        void setDateTimeTo(const QDateTime & dateTimeTo);
        void setRole(int role);

    protected:
        DateTimeFilterPrivate * d;
    }; // class DateTimeFilter




    class StarredFilter : public AbstractFilter
    {
        Q_OBJECT

    public:
        StarredFilter(QObject * parent = 0);

        bool accepts(const QModelIndex & index) const;
    }; // class StarredFilter




    class ANDFilterPrivate;
    class ANDFilter : public AbstractFilter
    {
        Q_OBJECT

    public:
        ANDFilter(AbstractFilter * subordinate1, AbstractFilter * subordinate2, QObject * parent = 0);
        ANDFilter(const QList< AbstractFilter * > & subordinates, QObject * parent = 0);
        ~ANDFilter();

        bool accepts(const QModelIndex & index) const;

        void setSubordinates(AbstractFilter * subordinate1, AbstractFilter * subordinate2);
        void setSubordinates(const QList< AbstractFilter * > & subordinates);
    protected:
        ANDFilterPrivate * d;
    }; // class AbstractFilter




    class ORFilterPrivate;
    class ORFilter : public AbstractFilter
    {
        Q_OBJECT

    public:
        ORFilter(AbstractFilter * subordinate1, AbstractFilter * subordinate2, QObject * parent = 0);
        ORFilter(const QList< AbstractFilter * > & subordinates, QObject * parent = 0);
        ~ORFilter();

        bool accepts(const QModelIndex & index) const;

        void setSubordinates(AbstractFilter * subordinate1, AbstractFilter * subordinate2);
        void setSubordinates(const QList< AbstractFilter * > & subordinates);
    protected:
        ORFilterPrivate * d;
    }; // class AbstractFilter




    class NOTFilterPrivate;
    class NOTFilter : public AbstractFilter
    {
        Q_OBJECT

    public:
        NOTFilter(AbstractFilter * subordinate, QObject * parent = 0);
        ~NOTFilter();

        bool accepts(const QModelIndex & index) const;

        void setSubordinate(AbstractFilter * subordinate);
    protected:
        NOTFilterPrivate * d;
    }; // class AbstractFilter

} // namespace Athenaeum

#endif // ATHENAEUM_ABSTRACTFILTER_H


