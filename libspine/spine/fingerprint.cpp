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

#include <sstream>
#include <iomanip>
#include <spine/fingerprint.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

namespace Spine {

    class Sha256Private {
        friend class Sha256;

    private:
        Sha256Private() : 
            _buf(0), _valid(false)
        {
            _buf = new unsigned char [EVP_MAX_MD_SIZE];
            //_md = EVP_get_digestbyname("sha256");
            //_mdctx = EVP_MD_CTX_create();
            _ctx256 = std::auto_ptr<SHA256_CTX>(new SHA256_CTX);
            //EVP_DigestInit_ex(_mdctx, _md, NULL);
        }
        ~Sha256Private()
        {
            //EVP_MD_CTX_destroy(_mdctx);
            delete [] _buf;
        }

        //EVP_MD_CTX * _mdctx;
        //const EVP_MD * _md;
        std::auto_ptr<SHA256_CTX> _ctx256;
        
        unsigned char *_buf;
        bool _valid;
    };

    Sha256::Sha256() : d(new Sha256Private)
    {
        SHA256_Init(d->_ctx256.get());
    }

    Sha256::~Sha256()
    {
        delete d;
    }

    void Sha256::update(unsigned char * data_, size_t length_)
    {
        //EVP_DigestUpdate(d->_mdctx, data_, length_);
        SHA256_Update(d->_ctx256.get(), data_, length_);
        d->_valid=true;
    }

    bool Sha256::isValid() const {
        return d->_valid;
    }

    std::string Sha256::calculateHash() {
        std::stringstream hexhash;

        if(!d->_valid) {
            // no data added
            d->_buf[0]=0;
        } else {
            // calculate hash
            unsigned int md_len;
            //EVP_DigestFinal_ex(d->_mdctx, d->_buf, &md_len);
            SHA256_Final(d->_buf, d->_ctx256.get());
            md_len = SHA256_DIGEST_LENGTH;
            
            for(unsigned int i = 0; i < md_len; ++i) {
                hexhash << std::hex << std::setw(2) << std::setfill('0') << (int)d->_buf[i];
            }
        }

        return hexhash.str();
    }

    char const Fingerprint::_base[] = "http://utopia.cs.manchester.ac.uk/fingerprint/";
}
