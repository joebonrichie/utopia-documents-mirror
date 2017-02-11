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

#ifndef FINGERPRINT_INCL_
#define FINGERPRINT_INCL_

#include <memory>
#include <string>

namespace Spine {

    class Sha256Private;

    class Sha256 {
    public:
        Sha256();
        ~Sha256();
        void update(unsigned char * data_, size_t length_);
        bool isValid() const;
        std::string calculateHash();

    private:
        Sha256Private *d;
    };

    class Fingerprint {
    public:

        template <class T>
        static T binaryFingerprintIri(T value_) {
            // sha256 binary hash of file

            return T(_base) + T("1/") + value_;
        }

        template <class T>
        static T character1FingerprintIri(T value_) {
            // sha256 of chars excluding 1 inch around edge of page

            return T(_base) + T("2/") + value_;
        }

        template <class T>
        static T character2FingerprintIri(T value_) {
            // sha256 of chars excluding first page and
            // excluding 1 inch around edge of page

            return T(_base) + T("3/") + value_;
        }

        template <class T>
        static T image1FingerprintIri(T value_) {
            // sha256 of image data excluding any images falling entirely
            // within 1 inch margin around edge of page and where
            // (area.boundingBox.width() * area.boundingBox.height())
            // > 5000.0

            return T(_base) + T("4/") + value_;
        }

        template <class T>
        static T image2FingerprintIri(T value_) {
            // sha256 of image data excluding first page and any images
            // falling entirely within 1 inch margin around edge of page and
            // where (area.boundingBox.width() * area.boundingBox.height())
            // > 5000.0

            return T(_base) + T("5/") + value_;
        }

        template <class T>
        static T xmpFingerprintIri(T value_) {
            // The xmpMM:DocumentID is a robust GUID created once for new
            // resources. Different renditions are expected to have
            // different values for xmpMM:DocumentID; it is not updated by
            // each save operation.

            // It should be guaranteed to be globally unique. In practical
            // terms, this means that the probability of a collision is so
            // remote as to be effectively impossible. Typically, 128- or
            // 144-bit numbers are used, encoded as hexadecimal strings.

            // The specification does not require any particular methodology
            // for creating a GUID, nor does it require any specific means
            // of formatting the GUID as a simple XMP value.

            // The only valid operations on XMP IDs are to create them, to
            // assign one to another, and to compare two of them for
            // equality. This comparison shall use the Unicode string value
            // as-is, using a direct byte-for-byte check for equality.

            // see: www.adobe.com/devnet/xmp/pdfs/XMPSpecificationPart1.pdf

            return T(_base) + T("6/") + value_;
        }

        template <class T>
        static T pdfFileIDFingerprintIri(T value_) {

            // File identifiers are defined by the optional ID entry in a
            // PDF file's trailer dictionary. The value of this entry is an
            // array of two strings. The first string is a permanent
            // identifier based on the contents of the file at the time it
            // was originally created and does not change when the file is
            // incrementally updated. The second string is a changing
            // identifier based on the file's contents at the time it was
            // last updated. When a file is first written, both identifiers
            // are set to the same value.

            // This fingerprint uses the first entry.

            return T(_base) + T("7/") + value_;
        }

    private:
        char const static _base[];
    };

}

#endif /* FINGERPRINT_INCL_ */
