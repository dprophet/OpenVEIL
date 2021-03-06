//	Copyright (c) 2017, TecSec, Inc.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are met:
//	
//		* Redistributions of source code must retain the above copyright
//		  notice, this list of conditions and the following disclaimer.
//		* Redistributions in binary form must reproduce the above copyright
//		  notice, this list of conditions and the following disclaimer in the
//		  documentation and/or other materials provided with the distribution.
//		* Neither the name of TecSec nor the names of the contributors may be
//		  used to endorse or promote products derived from this software 
//		  without specific prior written permission.
//		 
//	ALTERNATIVELY, provided that this notice is retained in full, this product
//	may be distributed under the terms of the GNU General Public License (GPL),
//	in which case the provisions of the GPL apply INSTEAD OF those given above.
//		 
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
//	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//	DISCLAIMED.  IN NO EVENT SHALL TECSEC BE LIABLE FOR ANY 
//	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//	LOSS OF USE, DATA OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
//	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Written by Roger Butler

#include "stdafx.h"

static const struct tag_AlgInfo
{
    const char* oid;
    SSL_HashAlgorithm hashAlg;
    SSL_SignatureAlgorithm sigAlg;
} gAlgs[] =
{
    { RSA_SHA1_SIGN_OID, sslhash_sha1, sslsign_rsa },
    { RSA_SHA224_SIGN_OID,sslhash_sha224,sslsign_rsa },
    { RSA_SHA256_SIGN_OID,sslhash_sha256, sslsign_rsa },
    { RSA_SHA384_SIGN_OID,sslhash_sha384,sslsign_rsa },
    { RSA_SHA512_SIGN_OID,sslhash_sha512,sslsign_rsa },
    { ECDSA_SHA1_OID,sslhash_sha1,sslsign_ecdsa },
    { ECDSA_SHA224_OID,sslhash_sha224,sslsign_ecdsa },
    { ECDSA_SHA256_OID,sslhash_sha256,sslsign_ecdsa },
    { ECDSA_SHA384_OID,sslhash_sha384,sslsign_ecdsa },
    { ECDSA_SHA512_OID,sslhash_sha512,sslsign_ecdsa },
    { DSA_SHA1_OID,sslhash_sha1,sslsign_dsa },
    { NIST_DSA_SHA224_OID,sslhash_sha224,sslsign_dsa },
    { NIST_DSA_SHA256_OID,sslhash_sha256,sslsign_dsa },
    { DSA_PARAMETER_SET,sslhash_sha1,sslsign_dsa },
};

static const struct tag_AlgInfo* getAlgInfo(const char* oid)
{
    for (size_t i = 0; i < sizeof(gAlgs) / sizeof(gAlgs[0]); i++)
    {
        if (strcmp(oid, gAlgs[i].oid) == 0)
            return &gAlgs[i];
    }
    return nullptr;
}

VEILCORE_API bool tscrypto::GetCertificateSignatureInfo(const tscrypto::tsCryptoString& oid, SSL_HashAlgorithm& hashAlg, SSL_SignatureAlgorithm& sigAlg)
{
    const struct tag_AlgInfo* alg = getAlgInfo(oid.c_str());

    if (alg == nullptr)
        return false;

    hashAlg = alg->hashAlg;
    sigAlg = alg->sigAlg;
    return true;
}
VEILCORE_API bool tscrypto::GetCertificateSignatureInfo(const tscrypto::tsCryptoData& oid, SSL_HashAlgorithm& hashAlg, SSL_SignatureAlgorithm& sigAlg)
{
    const struct tag_AlgInfo* alg = getAlgInfo(oid.ToOIDString().c_str());

    if (alg == nullptr)
        return false;

    hashAlg = alg->hashAlg;
    sigAlg = alg->sigAlg;
    return true;
}

class tsCertificateValidator : public ICertificateValidator, public tsmod::IObject, public tsmod::IInitializableObject
{
public:
    tsCertificateValidator()
    {
    }
    virtual ~tsCertificateValidator()
    {
    }

protected:
    std::shared_ptr<IPropertyMap> _parameters;
    std::shared_ptr<ICertificateValidatorOptions> _options;
    std::shared_ptr<ICertificateRevocationChecker> _revoker;

    virtual bool GetCertificateSignatureInfo(const tscrypto::tsCertificateParser& cert, SSL_HashAlgorithm& hashAlg, SSL_SignatureAlgorithm& sigAlg) override
    {
        tsCryptoString oid = cert.SignatureAlgorithmOID().ToOIDString();

        return tscrypto::GetCertificateSignatureInfo(oid, hashAlg, sigAlg);
    }
    virtual bool GetCertificateSignatureInfo(const tscrypto::tsCryptoData& cert, SSL_HashAlgorithm& hashAlg, SSL_SignatureAlgorithm& sigAlg) override
    {
        tsCertificateParser parser;

        if (!parser.LoadCertificate(cert))
            return false;
        return GetCertificateSignatureInfo(parser, hashAlg, sigAlg);
    }

    virtual SSL_AlertDescription basicCertValidation(const tscrypto::tsCertificateParser& cert, std::shared_ptr<tscrypto::AsymmetricKey> priorKey, bool sslCert, SSL_CIPHER cipher) override
    {
        SSL_HashAlgorithm hashAlg;
        SSL_SignatureAlgorithm sigAlg;

        if (!GetCertificateSignatureInfo(cert, hashAlg, sigAlg))
            return sslalert_bad_certificate;
        if (cipher != tsTLS_NULL_WITH_NULL_NULL && !_options->CertSignatureTypeOkForCipher(hashAlg, sigAlg, sslCert, cipher))
            return sslalert_bad_certificate;
        if (!!priorKey && !cert.VerifySignature(priorKey))
            return sslalert_bad_certificate;
        if (cert.ValidFrom() > tsCryptoDate::Now() || cert.ValidTo() < tsCryptoDate::Now())
            return sslalert_certificate_expired;

        if (!!_revoker)
        {
            if (_revoker->IsRevoked(cert))
                return sslalert_certificate_revoked;
        }
        return sslalert_no_error;
    }
    virtual SSL_AlertDescription basicCertValidation(const tscrypto::tsCryptoData& cert, std::shared_ptr<tscrypto::AsymmetricKey> priorKey, bool sslCert, SSL_CIPHER cipher) override
    {
        tsCertificateParser parser;

        if (!parser.LoadCertificate(cert))
            return sslalert_bad_certificate;
        return basicCertValidation(parser, priorKey, sslCert, cipher);
    }

    virtual bool IsSelfSigned(const tscrypto::tsCertificateParser& cert) override
    {
        return cert.IssuerName() == cert.SubjectName();
    }
    virtual bool IsSelfSigned(const tscrypto::tsCryptoData& cert) override
    {
        tsCertificateParser parser;

        if (!parser.LoadCertificate(cert))
            return false;

        return IsSelfSigned(parser);
    }

    virtual SSL_AlertDescription ValidateCertificate(const tscrypto::tsCryptoDataList& certificates, SSL_CIPHER cipher) override
    {
        bool first = true;
        //int32_t startAt;
        SSL_AlertDescription retVal;
        std::shared_ptr<tscrypto::AsymmetricKey> priorKey;
        std::shared_ptr<ICertificateRetriever> retriever;
        std::vector<tsCertificateParser> certs;
        bool trustedRootFound = false;
        bool trustedIntermediaryFound = false;

        if (!certificates || certificates->size() == 0)
            return sslalert_bad_certificate;

        retriever = _options->getCertificateRetriever();

        if (!retriever)
            return sslalert_certificate_unknown;

        for (const auto& it : *certificates)
        {
            tsCertificateParser certParser;

            if (!certParser.LoadCertificate(it))
            {
                return sslalert_bad_certificate;
            }
            certs.push_back(certParser);
            if (certs.size() > 1)
            {
                if (retriever->isTrustedRootCert(certParser))
                {
                    trustedRootFound = true;
                    break;
                }
                else if (retriever->isTrustedIntermediaryCert(certParser))
                {
                    trustedIntermediaryFound = true;
                    break;
                }
            }
        }

        if (!trustedRootFound && !trustedIntermediaryFound && !IsSelfSigned(certs.back()))
        {
            tscrypto::tsCryptoDataList trustList = CreateTsCryptoDataList();
            // If the trusted root/intermediary is not found then retrieve the chain.
            if (!retriever->getTrustChainForCert(certs.back(), trustList, trustedRootFound, trustedIntermediaryFound))
            {
                return sslalert_bad_certificate;
            }
            for (const auto& it : *trustList)
            {
                tsCertificateParser certParser;

                if (!certParser.LoadCertificate(it))
                {
                    return sslalert_bad_certificate;
                }
                certs.push_back(certParser);
            }
        }
        if (!trustedRootFound && !trustedIntermediaryFound)
        {
            if (!IsSelfSigned(certs.back()))
            {
                return sslalert_bad_certificate;
            }
            else
            {
                if (!_options->allowSelfSignedCerts())
                    return sslalert_bad_certificate;
                else
                {
                    priorKey = certs.back().getPublicKeyObject();
                    trustedRootFound = true;
                }
            }
        }

        // At this point we have the cert chain and it terminates on a trusted entity.
        for (int i = (int)certs.size() - 1; i >= 0; i--)
        {
            const tsCertificateParser& cert = certs.at(i);

            retVal = basicCertValidation(cert, priorKey, (i == 0), cipher);
            if (retVal != sslalert_no_error)
                return retVal;

            if (i > 0)
            {
                SSL_HashAlgorithm hashAlg;
                SSL_SignatureAlgorithm sigAlg;

                if ((cert.GetKeyUsage() & CA_Certificate_Request::digitialSignature) == 0 || (cert.GetKeyUsage() & CA_Certificate_Request::keyCertSign) == 0)
                    return sslalert_bad_certificate;

                if (!GetCertificateSignatureInfo(cert, hashAlg, sigAlg))
                    return sslalert_bad_certificate;

                if (!_options->CertSignatureTypeOkForCipher(hashAlg, sigAlg, false, cipher))
                    return sslalert_bad_certificate;

                 priorKey = cert.getPublicKeyObject();
                 if (!_options->KeySizeOkForCipher(hashAlg, sigAlg, (uint32_t)priorKey->KeySize(), false, cipher))
                     return sslalert_bad_certificate;
            }
            if (trustedRootFound)
            {
                bool isCA = false;
                int maxIntermediaries = 1000000;

                if (!cert.getBasicConstraintInfo(isCA, maxIntermediaries) || !isCA)
                    return sslalert_bad_certificate;

                if (maxIntermediaries + 2 > certs.size())
                    return sslalert_bad_certificate;
                trustedRootFound = false; // clear the flag for the reset of the testing
            }
            else if (i > 0)
            {
                bool isCA = false;
                int maxIntermediaries = 1000000;

                if (cert.getBasicConstraintInfo(isCA, maxIntermediaries) && isCA)
                    return sslalert_unsupported_certificate;
            }
        }

        // Validate the flags for the sslCert
        const TlsSupport_Descriptor* supDesc = (const TlsSupport_Descriptor*)findCkmAlgorithm("TLS-SUPPORT");

        if (supDesc == nullptr)
            return sslalert_certificate_unknown;

        const char* info = supDesc->keyExchange(cipher);

        if (info != nullptr)
        {
            if (TsStriCmp(info, "RSA") == 0)
            {
                if ((certs.front().GetKeyUsage() & CA_Certificate_Request::digitialSignature) == 0 && (certs.front().GetKeyUsage() & CA_Certificate_Request::dataEncipherment) == 0)
                    return sslalert_bad_certificate;
            }
            else if (TsStriCmp(info, "DSA") == 0)
            {
                if ((certs.front().GetKeyUsage() & CA_Certificate_Request::digitialSignature) == 0)
                    return sslalert_bad_certificate;
            }
            else if (TsStriCmp(info, "ECDSA") == 0)
            {
                if ((certs.front().GetKeyUsage() & CA_Certificate_Request::digitialSignature) == 0)
                    return sslalert_bad_certificate;
            }
            else if (TsStriCmp(info, "DH") == 0)
            {
                if ((certs.front().GetKeyUsage() & CA_Certificate_Request::dataEncipherment) == 0)
                    return sslalert_bad_certificate;
            }
            else if (TsStriCmp(info, "ECDH") == 0)
            {
                if ((certs.front().GetKeyUsage() & CA_Certificate_Request::dataEncipherment) == 0)
                    return sslalert_bad_certificate;
            }
            else 
            {
                return sslalert_bad_certificate;
            }
        }
        else
        {
            return sslalert_bad_certificate;
        }

        return sslalert_no_error;

        //sslalert_no_error = 1024,
        //sslalert_no_certificate_RESERVED = 41,
        //sslalert_bad_certificate = 42,
        //sslalert_unsupported_certificate = 43,
        //sslalert_certificate_revoked = 44,
        //sslalert_certificate_expired = 45,
        //sslalert_certificate_unknown = 46,

    }

    // Inherited via IInitializableObject
    virtual bool InitializeWithFullName(const tscrypto::tsCryptoStringBase & fullName) override
    {
        _parameters = ServiceLocator()->get_instance<IPropertyMap>("PropertyMap");
        _parameters->parseUrlQueryString(fullName);

        // Parameters are all upper case
        if (!_parameters->hasItem("OPTIONS"))
            _parameters->AddItem("OPTIONS", "/CERTIFICATEOPTIONS");

        tsCryptoString optionsName = _parameters->item("OPTIONS");

        optionsName.Replace("^", "?").Replace("$", "&");

        if (!(_options = ServiceLocator()->try_get_instance<ICertificateValidatorOptions>(optionsName)))
            return false;

        _revoker = _options->getRevocationChecker();

        return true;
    }
};

tsmod::IObject* CreateCertificateValidator()
{
    return dynamic_cast<tsmod::IObject*>(new tsCertificateValidator);
}

class BasicCertOptions : public tscrypto::ICertificateValidatorOptions, public tsmod::IObject, public tsmod::IInitializableObject
{
public:
    BasicCertOptions() : _selfSigned(false)
    {
    }
    virtual ~BasicCertOptions()
    {
    }

protected:
    std::shared_ptr<ICertificateRetriever> _retriever;
    std::shared_ptr<ICertificateRevocationChecker> _checker;
    bool _selfSigned;

    // Inherited via IInitializableObject
    virtual bool InitializeWithFullName(const tscrypto::tsCryptoStringBase & fullName) override
    {
        std::shared_ptr<IPropertyMap> _parameters = ServiceLocator()->get_instance<IPropertyMap>("PropertyMap");
        _parameters->parseUrlQueryString(fullName);

        // Parameters are all upper case
        if (!_parameters->hasItem("RETRIEVER"))
            _parameters->AddItem("RETRIEVER", "/CERTIFICATE_RETRIEVER");

        if (_parameters->hasItem("CHECKER"))
        {
            _checker = ServiceLocator()->try_get_instance<ICertificateRevocationChecker>(_parameters->item("CHECKER"));
            if (!_checker)
                return false;
        }

        _selfSigned = _parameters->itemAsBoolean("SELFSIGNED", false);

        _retriever = ServiceLocator()->try_get_instance<ICertificateRetriever>(_parameters->item("RETRIEVER"));
        return !!_retriever;
    }

    // Inherited via ICertificateValidatorOptions
    virtual bool allowSelfSignedCerts() const override
    {
        return _selfSigned;
    }

    virtual std::shared_ptr<ICertificateRevocationChecker> getRevocationChecker() override
    {
        return _checker;
    }

    virtual std::shared_ptr<ICertificateRetriever> getCertificateRetriever() override
    {
        return _retriever;
    }

    virtual bool CertSignatureTypeOkForCipher(SSL_HashAlgorithm hashAlg, SSL_SignatureAlgorithm sigAlg, bool sslCert, SSL_CIPHER cipher) override
    {
        return true;
    }

    virtual bool KeySizeOkForCipher(SSL_HashAlgorithm hashAlg, SSL_SignatureAlgorithm sigAlg, uint32_t keySize, bool sslCert, SSL_CIPHER cipher) override
    {
        return true;
    }
};

tsmod::IObject* CreateBasicCertificateOptions()
{
    return dynamic_cast<tsmod::IObject*>(new tsCertificateValidator);
}
