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

#include <utopia2/encryption.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <inttypes.h>

#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QRegExp>
#include <QSettings>
#include <QStringList>
#include <QUuid>

#include <QtDebug>

namespace Utopia
{

#define K1 0x01357dc8
#define K2 0xbc07209e
#define K3 0x98b2314f
#define K4 0x97f11379
#define K5 0x3ad645fd
#define K6 0x20b32345
#define K7 0xc3878ab1
#define K8 0x0ba41ed8
#define K9 0x33cb7cb5

    static struct {
        uint32_t a, b, c, d, e, f, g, h, i;
    } _legacy_security_block_pk = { K1, K2, K3, K4, K5, K6, K7, K8, K9 };
    static struct {
        uint32_t a, b, c, d;
    } _legacy_security_block_iv = { K3, K8, K2, K7 };
    static struct {
        uint32_t a, b, c, d, e, f, g, h;
    } _legacy_security_block_hs = { K3, K8, K2, K7, K1, K6, K4, K5 };




    static QByteArray _get_stored_security_block(int i)
    {
        QSettings conf;
        conf.beginGroup("Security");
        int size = conf.beginReadArray("Encryption Blocks");
        if (i >= 0 && i < size) {
            conf.setArrayIndex(i);
            return conf.value("Block").toByteArray();
        }
        return QByteArray();
    }

    static void _set_stored_security_block(int i, const QByteArray & block)
    {
        QSettings conf;
        conf.beginGroup("Security");
        conf.beginWriteArray("Encryption Blocks");
        if (i >= 0) {
            conf.setArrayIndex(i);
            conf.setValue("Block", block);
        }
        conf.endArray();
        conf.endGroup();
    }

    static int _get_stored_security_block_count()
    {
        QSettings conf;
        conf.beginGroup("Security");
        return conf.beginReadArray("Encryption Blocks");
    }

    static QByteArray _get_stored_pk(int i)
    {
        QByteArray security_block(_get_stored_security_block(i));
        return security_block.mid(0, 36);
    }

    static QByteArray _get_stored_iv(int i)
    {
        QByteArray security_block(_get_stored_security_block(i));
        return security_block.mid(36, 16);
    }

    static QByteArray _get_stored_hs(int i)
    {
        QByteArray security_block(_get_stored_security_block(i));
        return security_block.mid(52, 32);
    }

    static QByteArray _hash(const QByteArray & plain)
    {
        QCryptographicHash hash(QCryptographicHash::Sha1);
        hash.addData(plain);
        return hash.result();
    }




    void generateSecurityBlocks()
    {
        bool recrypt = false;

        // If no encryption block is present...
        QByteArray block(_get_stored_security_block(0));
        if (block.isNull()) {
            // Store the legacy encryption block
            block = QByteArray(reinterpret_cast< const char * >(&_legacy_security_block_pk),
                               sizeof(_legacy_security_block_pk)) +
                    QByteArray(reinterpret_cast< const char * >(&_legacy_security_block_iv),
                               sizeof(_legacy_security_block_iv)) +
                    QByteArray(reinterpret_cast< const char * >(&_legacy_security_block_hs),
                               sizeof(_legacy_security_block_hs));
            _set_stored_security_block(0, block);
        }

        // If there isn't at least one non-legacy block, generate one
        block = _get_stored_security_block(1);
        if (block.isNull()) {
            unsigned char random_bytes[84];
            RAND_bytes(random_bytes, 84);
            block = QByteArray(reinterpret_cast< const char * >(random_bytes), sizeof(random_bytes));
            _set_stored_security_block(1, block);
            recrypt = true;
        }

        // Force re-encryption of legacy encrypted data
        if (recrypt) {
            QSettings conf;

            // Start by re-encrypting the account credentials
            conf.beginGroup("Services");
            conf.beginGroup("Store");
            foreach (const QString & uuid, conf.childGroups()) {
                conf.beginGroup(uuid);
                conf.beginGroup("properties");
                QByteArray ciphertext = conf.value("cachedCredentials").toByteArray();
                ciphertext = encrypt(decrypt(ciphertext, QUuid(uuid).toString()), QUuid(uuid).toString());
                conf.setValue("cachedCredentials", ciphertext);
                conf.endGroup();
                conf.endGroup();
            }
            conf.endGroup();
            conf.endGroup();

            // Then the configuration data maps
            conf.beginGroup("Configurations");
            foreach (const QString & uuid, conf.childGroups()) {
                conf.beginGroup(uuid);
                QByteArray ciphertext = conf.value("data").toByteArray();
                ciphertext = encrypt(decrypt(ciphertext, QUuid(uuid).toString()), QUuid(uuid).toString());
                conf.setValue("data", ciphertext);
                conf.endGroup();
            }
            conf.endGroup();

            // Then proxy credentials
            conf.beginGroup("Networking");
            conf.beginGroup("Proxies");
            conf.beginGroup("Realms");
            foreach (const QString & key, conf.childGroups()) {
                conf.beginGroup(key);
                QString username = conf.value("username").toString();
                QString ciphertext = conf.value("password").toString();
                ciphertext = encryptPassword(username, decryptPassword(username, ciphertext));
                conf.setValue("password", ciphertext);
                conf.endGroup();
            }
            conf.endGroup();
            conf.beginGroup("Hosts");
            foreach (const QString & key, conf.childGroups()) {
                conf.beginGroup(key);
                QString username = conf.value("username").toString();
                QString ciphertext = conf.value("password").toString();
                ciphertext = encryptPassword(username, decryptPassword(username, ciphertext));
                conf.setValue("password", ciphertext);
                conf.endGroup();
            }
            conf.endGroup();
            conf.endGroup();
            conf.endGroup();
        }
    }




    QByteArray encryptMap(const QVariantMap & map, const QString & salt)
    {
        QByteArray blob;
        QDataStream stream(&blob, QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_4_7);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream << map;
        return encrypt(blob, salt);
    }

    QVariantMap decryptMap(const QByteArray & ciphertext, const QString & salt)
    {
        QVariantMap map;
        QByteArray blob(decrypt(ciphertext, salt));
        QDataStream stream(blob);
        stream.setVersion(QDataStream::Qt_4_7);
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        stream >> map;
        return map;
    }

    QByteArray encrypt(const QByteArray & plaintext, const QString & salt)
    {
        QByteArray ciphertext;
        QByteArray in(plaintext);
        QByteArray hash;
        QByteArray iv;
        int block_index = _get_stored_security_block_count() - 1;
        if (salt.isEmpty()) {
            hash = _hash(_get_stored_hs(block_index));
            iv = _hash(_get_stored_iv(block_index)).right(8);
        } else {
            hash = _hash(salt.toUtf8());
            iv = QByteArray(hash.right(8));
        }

        QByteArray key(_get_stored_pk(block_index) + hash);


#if OPENSSL_VERSION_NUMBER < 0x010100000
        EVP_CIPHER_CTX ctx_s;
        EVP_CIPHER_CTX_init(&ctx_s);
        EVP_CIPHER_CTX * ctx = &ctx_s;
#else
        EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
#endif

        unsigned char outbuf[in.size() + EVP_MAX_BLOCK_LENGTH + 2048];
        int outlen, tmplen, final_rc = 0;

        if (EVP_EncryptInit(ctx, EVP_bf_cbc(), (const unsigned char *) key.constData(), (const unsigned char *) iv.constData()) &&
            EVP_EncryptUpdate(ctx, outbuf, &outlen, (const unsigned char *) in.constData(), in.size()) &&
            (final_rc = EVP_EncryptFinal(ctx, outbuf + outlen, &tmplen))) {
            outlen += tmplen;
            ciphertext = QByteArray((const char *) outbuf, outlen);
        }

#if OPENSSL_VERSION_NUMBER < 0x010100000
        EVP_CIPHER_CTX_cleanup(ctx);
#else
        EVP_CIPHER_CTX_free(ctx);
#endif
        if (!final_rc) {
            return QByteArray();
        }

        return QString("utopia:enc:%1:").arg(block_index).toUtf8() + ciphertext;
    }

    QString encrypt(const QString & plaintext, const QString & salt)
    {
        return encrypt(plaintext.toUtf8(), salt).toBase64();
    }

    QByteArray decrypt(const QByteArray & ciphertext, const QString & salt)
    {
        int block_index = 0;
        int ciphertext_offset = 0;
        QRegExp encRegExp("^utopia:enc:(\\d+):");
        if (encRegExp.indexIn(QString::fromUtf8(ciphertext)) == 0) {
            block_index = encRegExp.cap(1).toInt();
            ciphertext_offset = encRegExp.cap(0).size();
        }

        QByteArray plaintext;
        QByteArray in(ciphertext.mid(ciphertext_offset));
        QByteArray hash;
        QByteArray iv;
        if (salt.isEmpty()) {
            hash = _hash(_get_stored_hs(block_index));
            iv = _hash(_get_stored_iv(block_index)).right(8);
        } else {
            hash = _hash(salt.toUtf8());
            iv = QByteArray(hash.right(8));
        }

        QByteArray key(_get_stored_pk(block_index) + hash);

#if OPENSSL_VERSION_NUMBER < 0x010100000
        EVP_CIPHER_CTX ctx_s;
        EVP_CIPHER_CTX_init(&ctx_s);
        EVP_CIPHER_CTX * ctx = &ctx_s;
#else
        EVP_CIPHER_CTX * ctx = EVP_CIPHER_CTX_new();
#endif

        unsigned char outbuf[in.size()];
        int outlen, tmplen, final_rc = 0;

        if (EVP_DecryptInit(ctx, EVP_bf_cbc(), (const unsigned char *) key.constData(), (const unsigned char *) iv.constData()) &&
            EVP_DecryptUpdate(ctx, outbuf, &outlen, (const unsigned char *) in.constData(), in.size()) &&
            (final_rc = EVP_DecryptFinal(ctx, outbuf + outlen, &tmplen))) {
            outlen += tmplen;
            plaintext = QByteArray((const char *) outbuf, outlen);
        }

#if OPENSSL_VERSION_NUMBER < 0x010100000
        EVP_CIPHER_CTX_cleanup(ctx);
#else
        EVP_CIPHER_CTX_free(ctx);
#endif
        if (!final_rc) {
            return QByteArray();
        }

        return plaintext;
    }

    QString decrypt(const QString & ciphertext, const QString & salt)
    {
        return QString::fromUtf8(decrypt(QByteArray::fromBase64(ciphertext.toUtf8()), salt));
    }

    QString encryptPassword(const QString & username, const QString & password)
    {
        return encrypt(password, username);
    }

    QString decryptPassword(const QString & username, const QString & ciphertext)
    {
        return decrypt(ciphertext, username);
    }

}
