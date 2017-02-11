/*****************************************************************************
 *  
 *   This file is part of the libspine library.
 *       Copyright (c) 2008-2017 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   The libspine library is free software: you can redistribute it and/or
 *   modify it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE
 *   VERSION 3 as published by the Free Software Foundation.
 *   
 *   The libspine library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero
 *   General Public License for more details.
 *   
 *   You should have received a copy of the GNU Affero General Public License
 *   along with the libspine library. If not, see
 *   <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

#ifndef BOUNDINGBOX_H__
#define BOUNDINGBOX_H__

#include <algorithm>

#include <ostream>

namespace Spine
{

    struct BoundingBox
    {
        BoundingBox ()
            :x1(0), y1(0), x2(0), y2(0)
            { }
        BoundingBox (double x1_, double y1_, double x2_, double y2_)
            :x1(x1_), y1(y1_), x2(x2_), y2(y2_)
            { }
        void getMinMax (double *x1_, double *y1_, double *x2_, double *y2_) const
            {
                *x1_=x1;
                *y1_=y1;
                *x2_=x2;
                *y2_=y2;
            }
        bool operator == (const BoundingBox & rhs_) const {
            return rhs_.x1==x1 && rhs_.x2==x2 && rhs_.y1==y1 && rhs_.y2==y2;
        }

        bool operator != (const BoundingBox & rhs_) const {
            return !(rhs_==*this);
        }

        bool operator < (const BoundingBox & rhs_) const {
            return y1 < rhs_.y1
                || (y1 == rhs_.y1 && x1 < rhs_.x1)
                || (y1 == rhs_.y1 && x1 == rhs_.x1 && y2 < rhs_.y2)
                || (y1 == rhs_.y1 && x1 == rhs_.x1 && y2 == rhs_.y2 && x2 < rhs_.x2);
            // Arbitrary ordering for use in data structures such as sets (not intended to be page order)
        }

        void setMinMax (double x1_, double y1_, double x2_, double y2_)
            {
                x1=x1_;
                y1=y1_;
                x2=x2_;
                y2=y2_;
            }

        bool isValid() const
            {
                return x2>x1 && y2>y1;
            }

        void normalize()
            {
                *this=this->normalized();
            }

        BoundingBox normalized() const
            {
                return BoundingBox(std::min(x1,x2),std::min(y1,y2),std::max(x1,x2),std::max(y1,y2));
            }

        BoundingBox& operator|=(const BoundingBox &b_)
            {
                *this = *this | b_;
                return *this;
            }

        BoundingBox operator|(const BoundingBox &b_) const
            {
                BoundingBox b1 = this->normalized();
                BoundingBox b2 = b_.normalized();

                if (!b1.isValid()) {
                    return b2;
                }

                return BoundingBox (std::min(b1.x1, b2.x1),
                                    std::min(b1.y1, b2.y1),
                                    std::max(b1.x2, b2.x2),
                                    std::max(b1.y2, b2.y2));
            }

        double width() const
            {
                return x2-x1;
            }

        double height() const
            {
                return y2-y1;
            }

        bool contains(double x, double y) const
            {
                return x >= x1 && x <= x2 && y >= y1 && y <= y2;
            }

        void getCenter(double *x, double *y) const
            {
                *x=x1+width()/2.0;
                *y=y1+height()/2.0;
            }

        double x1;
        double y1;
        double x2;
        double y2;
    };

    inline std::ostream & operator << (std::ostream & os, const BoundingBox & bb)
    {
        os << "BB(" << bb.x1 << "," << bb.y1 << "," << bb.x2 << "," << bb.y2 << ")";
        return os;
    }

}

#endif // BOUNDINGBOX_H_
