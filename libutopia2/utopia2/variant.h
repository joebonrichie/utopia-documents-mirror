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

#ifndef Utopia_VARIANT_H
#define Utopia_VARIANT_H

#include <utopia2/config.h>

#include <QString>
#include <cstring>
#include <iostream>
#include <cstdio>

namespace Utopia
{

    //
    // VariantBase class
    //

    class LIBUTOPIA_API VariantBase
    {
    public:
        // Destructor
        virtual ~VariantBase();

        // Serialisers
        virtual QString type() const;
        virtual QString representation() const;

    }; // class VariantBase

    //
    // VariantCommon class
    //

    template< typename value_type > class LIBUTOPIA_API VariantCommon : public VariantBase
    {
    public:
        // Constructors
        VariantCommon()
            : VariantBase()
        {};
        VariantCommon(const value_type& value_)
            : VariantBase(), _value(value_)
        {};
        VariantCommon(const VariantCommon< value_type >& rhs_)
            : VariantBase()
        { *this = rhs_; };

        // Assignment methods
        VariantCommon< value_type >& operator = (const VariantCommon< value_type >& rhs_)
        {
            _value = rhs_._value;
            return *this;
        };
        VariantCommon< value_type >& operator = (const value_type& rhs_)
        {
            _value = rhs_;
            return *this;
        };
        void set_value(const value_type& value_)
        { _value = value_; };

        // Accessor methods
        operator value_type& ()
        {
            return _value;
        };
        operator const value_type& () const
        {
            return _value;
        };
        value_type& value()
        { return _value; };
        const value_type& value() const
        { return _value; };

        // Destructor
        virtual ~VariantCommon() {};

        // Serialisers
        virtual QString type() const
        { return "?"; };
        virtual QString representation() const
        { return "?"; };

    private:
        // Value
        value_type _value;

    }; // class VariantCommon

    //
    // Variant class
    //

    template< typename value_type > class LIBUTOPIA_API Variant : public VariantCommon< value_type > {}; // class Variant

    // Partial specialisations
    template<> class LIBUTOPIA_API Variant< QString > : public VariantCommon< QString >
    {
    public:
        // Constructors
        Variant()
        {};
        Variant(const QString& value_)
            : VariantCommon< QString >(value_)
        {};
        Variant(const VariantCommon< QString >& rhs_)
            : VariantCommon< QString >(rhs_)
        {};

        // Serialiser
        QString type() const
        { return "QString"; }
        QString representation() const
        { return value(); }
    };

    template<> class LIBUTOPIA_API Variant< int > : public VariantCommon< int >
    {
    public:
        Variant()
        {};
        Variant(const int& value_)
            : VariantCommon< int >(value_)
        {};
        Variant(const VariantCommon< int >& rhs_)
            : VariantCommon< int >(rhs_)
        {};

        // Serialiser
        QString type() const
        { return "int"; }
        QString representation() const
        {
            char buf[100] = {0};
            std::sprintf(buf, "%d", value());
            return buf;
        }
    };

    template<> class LIBUTOPIA_API Variant< float > : public VariantCommon< float >
    {
    public:
        Variant()
        {};
        Variant(const float& value_)
            : VariantCommon< float >(value_)
        {};
        Variant(const VariantCommon< float >& rhs_)
            : VariantCommon< float >(rhs_)
        {};

        // Serialiser
        QString type() const
        { return "float"; }
        QString representation() const
        {
            char buf[100] = {0};
            std::sprintf(buf, "%f", value());
            return buf;
        }
    };

    template<> class LIBUTOPIA_API Variant< double > : public VariantCommon< double >
    {
    public:
        Variant()
        {};
        Variant(const double& value_)
            : VariantCommon< double >(value_)
        {};
        Variant(const VariantCommon< double >& rhs_)
            : VariantCommon< double >(rhs_)
        {};

        // Serialiser
        QString type() const
        { return "double"; }
        QString representation() const
        {
            char buf[100] = {0};
            std::sprintf(buf, "%f", value());
            return buf;
        }
    };

    template<> class LIBUTOPIA_API Variant< char > : public VariantCommon< char >
    {
    public:
        Variant()
        {};
        Variant(const char& value_)
            : VariantCommon< char >(value_)
        {};
        Variant(const VariantCommon< char >& rhs_)
            : VariantCommon< char >(rhs_)
        {};

        // Serialiser
        QString type() const
        { return "char"; }
        QString representation() const
        {
            char buf[2] = {0};
            buf[0] = value();
            return buf;
        }
    };

    template<> class LIBUTOPIA_API Variant< bool > : public VariantCommon< bool >
    {
    public:
        Variant()
        {};
        Variant(const bool& value_)
            : VariantCommon< bool >(value_)
        {};
        Variant(const VariantCommon< bool >& rhs_)
            : VariantCommon< bool >(rhs_)
        {};

        // Serialiser
        QString type() const
        { return "bool"; }
        QString representation() const
        { return value() ? "true" : "false"; }
    };

} // namespace Utopia

#endif // Utopia_VARIANT_H
