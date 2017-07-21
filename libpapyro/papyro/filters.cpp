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

#include <papyro/filters.h>
#include <papyro/abstractbibliography.h>

#include <QDateTime>
#include <QRegExp>

#include <QDebug>

namespace Athenaeum
{

    class TextFilterPrivate
    {
    public:
        TextFilterPrivate()
            : column(0), role(Qt::DisplayRole)
        {}

        QRegExp regExp;
        int column;
        int role;
    }; // class FilterPrivate

    TextFilter::TextFilter(const QRegExp & regExp, int column, int role, QObject * parent)
        : AbstractFilter(parent), d(new TextFilterPrivate)
    {
        setRegExp(regExp);
        setColumn(column);
        setRole(role);
    }

    TextFilter::TextFilter(const QString & string, int column, int role, QObject * parent)
        : AbstractFilter(parent), d(new TextFilterPrivate)
    {
        setFixedString(string);
        setColumn(column);
        setRole(role);
    }

    TextFilter::~TextFilter()
    {
        delete d;
    }

    bool TextFilter::accepts(const QModelIndex & index) const
    {
        if (index.column() == d->column) {
            return d->regExp.indexIn(index.data(d->role).toString()) >= 0;
        } else {
            return d->regExp.indexIn(index.sibling(index.row(), d->column).data(d->role).toString()) >= 0;
        }
    }

    int TextFilter::column() const
    {
        return d->column;
    }

    QRegExp TextFilter::regExp() const
    {
        return d->regExp;
    }

    int TextFilter::role() const
    {
        return d->role;
    }

    void TextFilter::setColumn(int column)
    {
        d->column = column;
        emit filterChanged();
    }

    void TextFilter::setFixedString(const QString & string)
    {
        QRegExp regExp(string);
        regExp.setPatternSyntax(QRegExp::FixedString);
        regExp.setCaseSensitivity(Qt::CaseInsensitive);
        setRegExp(regExp);
    }

    void TextFilter::setRegExp(const QRegExp & regExp)
    {
        d->regExp = regExp;
        emit filterChanged();
    }

    void TextFilter::setRole(int role)
    {
        d->role = role;
        emit filterChanged();
    }




    class DateTimeFilterPrivate
    {
    public:
        DateTimeFilterPrivate()
            : column(0), role(Qt::DisplayRole)
        {}

        QDateTime dateTimeFrom;
        QDateTime dateTimeTo;
        int column;
        int role;
    }; // class FilterPrivate

    DateTimeFilter::DateTimeFilter(QObject * parent)
        : AbstractFilter(parent), d(new DateTimeFilterPrivate)
    {}

    DateTimeFilter::~DateTimeFilter()
    {
        delete d;
    }

    bool DateTimeFilter::accepts(const QModelIndex & index) const
    {
        QDateTime concerned((index.column() == d->column ? index : index.sibling(index.row(), d->column)).data(d->role).toDateTime());
        return !concerned.isNull() && (d->dateTimeFrom.isNull() || concerned >= d->dateTimeFrom) &&
               (d->dateTimeTo.isNull() || concerned <= d->dateTimeTo);
    }

    int DateTimeFilter::column() const
    {
        return d->column;
    }

    QDateTime DateTimeFilter::dateTimeFrom() const
    {
        return d->dateTimeFrom;
    }

    QDateTime DateTimeFilter::dateTimeTo() const
    {
        return d->dateTimeTo;
    }

    int DateTimeFilter::role() const
    {
        return d->role;
    }

    void DateTimeFilter::setColumn(int column)
    {
        d->column = column;
        emit filterChanged();
    }

    void DateTimeFilter::setDateTimeFrom(const QDateTime & dateTimeFrom)
    {
        d->dateTimeFrom = dateTimeFrom;
        emit filterChanged();
    }

    void DateTimeFilter::setDateTimeTo(const QDateTime & dateTimeTo)
    {
        d->dateTimeTo = dateTimeTo;
        emit filterChanged();
    }

    void DateTimeFilter::setRole(int role)
    {
        d->role = role;
        emit filterChanged();
    }




    StarredFilter::StarredFilter(QObject * parent)
        : AbstractFilter(parent)
    {}

    bool StarredFilter::accepts(const QModelIndex & index) const
    {
        Citation::Flags flags =
            index.data(Citation::FlagsRole).value< Citation::Flags >();
        return flags & Citation::StarredFlag;
    }




    class ANDFilterPrivate
    {
    public:
        ANDFilterPrivate(ANDFilter * filter, AbstractFilter * subordinate1, AbstractFilter * subordinate2)
        {
            subordinates << subordinate1 << subordinate2;
            reparent();
        }

        ANDFilterPrivate(ANDFilter * filter, const QList< AbstractFilter * > & subordinates)
            : f(filter), subordinates(subordinates)
        {
            reparent();
        }

        void reparent()
        {
            foreach (AbstractFilter * filter, subordinates) {
                QObject::connect(filter, SIGNAL(filterChanged()), f, SIGNAL(filterChanged()));
                if (filter->parent() == 0) {
                    filter->setParent(f);
                }
            }
        }

        ANDFilter * f;
        QList< AbstractFilter * > subordinates;
    }; // class ANDFilterPrivate

    ANDFilter::ANDFilter(AbstractFilter * subordinate1, AbstractFilter * subordinate2, QObject * parent)
        : AbstractFilter(parent), d(new ANDFilterPrivate(this, subordinate1, subordinate2))
    {}

    ANDFilter::ANDFilter(const QList< AbstractFilter * > & subordinates, QObject * parent)
        : AbstractFilter(parent), d(new ANDFilterPrivate(this, subordinates))
    {}

    ANDFilter::~ANDFilter()
    {
        delete d;
    }

    bool ANDFilter::accepts(const QModelIndex & index) const
    {
        foreach (AbstractFilter * filter, d->subordinates) {
            if (!filter->accepts(index)) {
                return false;
            }
        }
        return true;
    }

    void ANDFilter::setSubordinates(AbstractFilter * subordinate1, AbstractFilter * subordinate2)
    {
        QList< AbstractFilter * > subordinates;
        subordinates << subordinate1 << subordinate2;
        setSubordinates(subordinates);
    }

    void ANDFilter::setSubordinates(const QList< AbstractFilter * > & subordinates)
    {
        foreach (AbstractFilter * filter, d->subordinates) {
            delete filter;
        }
        d->subordinates = subordinates;
        emit filterChanged();
    }




    class ORFilterPrivate
    {
    public:
        ORFilterPrivate(ORFilter * filter, AbstractFilter * subordinate1, AbstractFilter * subordinate2)
            : f(filter)
        {
            subordinates << subordinate1 << subordinate2;
            reparent();
        }

        ORFilterPrivate(ORFilter * filter, const QList< AbstractFilter * > & subordinates)
            : f(filter), subordinates(subordinates)
        {
            reparent();
        }

        void reparent()
        {
            foreach (AbstractFilter * filter, subordinates) {
                QObject::connect(filter, SIGNAL(filterChanged()), f, SIGNAL(filterChanged()));
                if (filter->parent() == 0) {
                    filter->setParent(f);
                }
            }
        }

        ORFilter * f;
        QList< AbstractFilter * > subordinates;
    }; // class ORFilterPrivate

    ORFilter::ORFilter(AbstractFilter * subordinate1, AbstractFilter * subordinate2, QObject * parent)
        : AbstractFilter(parent), d(new ORFilterPrivate(this, subordinate1, subordinate2))
    {}

    ORFilter::ORFilter(const QList< AbstractFilter * > & subordinates, QObject * parent)
        : AbstractFilter(parent), d(new ORFilterPrivate(this, subordinates))
    {}

    ORFilter::~ORFilter()
    {
        delete d;
    }

    bool ORFilter::accepts(const QModelIndex & index) const
    {
        foreach (AbstractFilter * filter, d->subordinates) {
            if (filter->accepts(index)) {
                return true;
            }
        }
        return false;
    }

    void ORFilter::setSubordinates(AbstractFilter * subordinate1, AbstractFilter * subordinate2)
    {
        QList< AbstractFilter * > subordinates;
        subordinates << subordinate1 << subordinate2;
        setSubordinates(subordinates);
    }

    void ORFilter::setSubordinates(const QList< AbstractFilter * > & subordinates)
    {
        foreach (AbstractFilter * filter, d->subordinates) {
            delete filter;
        }
        d->subordinates = subordinates;
        emit filterChanged();
    }




    class NOTFilterPrivate
    {
    public:
        NOTFilterPrivate(NOTFilter * filter, AbstractFilter * subordinate)
            : f(filter), subordinate(subordinate)
        {
            QObject::connect(subordinate, SIGNAL(filterChanged()), f, SIGNAL(filterChanged()));
            if (subordinate->parent()) {
                subordinate->setParent(f);
            }
        }

        NOTFilter * f;
        AbstractFilter * subordinate;
    }; // class NOTFilterPrivate

    NOTFilter::NOTFilter(AbstractFilter * subordinate, QObject * parent)
        : AbstractFilter(parent), d(new NOTFilterPrivate(this, subordinate))
    {}

    NOTFilter::~NOTFilter()
    {
        delete d;
    }

    bool NOTFilter::accepts(const QModelIndex & index) const
    {
        return !d->subordinate->accepts(index);
    }

    void NOTFilter::setSubordinate(AbstractFilter * subordinate)
    {
        if (d->subordinate) {
            delete subordinate;
        }
        d->subordinate = subordinate;
        emit filterChanged();
    }

} // namespace Athenaeum
