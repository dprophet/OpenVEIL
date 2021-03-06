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

using namespace tscrypto;

#define MAX_KEY_SIZE_BYTES 512

class DhParametersImpl : public DhParameters, public TSName, public tscrypto::ICryptoObject, public tscrypto::IInitializableObject, public AlgorithmInfo, public TSALG_Access
{
public:
	DhParametersImpl(const tsCryptoStringBase& algorithm) :
		desc(nullptr),
		reason(kvf_NoFailure)
	{
		desc = findDhParamAlgorithm("DHPARAMETERS");
		if (desc != nullptr)
		{
			dhParams = desc->createParametersetStructure(desc);
		}
		SetName(algorithm);
	}
	virtual ~DhParametersImpl(void)
	{
	}

	// AlgorithmInfo
	virtual tsCryptoString AlgorithmName() const override
	{
		return GetName();
	}
	virtual tsCryptoString AlgorithmOID() const override
	{
		return LookUpAlgOID(GetName());
	}
	virtual TS_ALG_ID AlgorithmID() const override
	{
		return LookUpAlgID(GetName());
	}

	// DhParameters
	virtual tsCryptoData get_prime() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getPrime(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_prime(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;
		if (setTo.size() == 0 || (setTo[0] & 0x80) == 0 || (setTo[setTo.size() - 1] & 1) == 0)
			return false;

		switch (setTo.size())
		{
		case 64:
		case 128:
		case 256:
		case 384:
			break;
		default:
			return false;
		}
		return desc->setPrime(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());
	}
	virtual tsCryptoData get_subprime() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getSubprime(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_subprime(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;
		if (setTo.size() == 0 || (setTo[0] & 0x80) == 0 || (setTo[setTo.size() - 1] & 1) == 0)
			return false;

		switch (setTo.size())
		{
		case 20:
		case 28:
		case 32:
			break;
		default:
			return false;
		}
		return desc->setSubprime(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());
	}
	virtual tsCryptoData get_generator() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getGenerator(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_generator(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		if (setTo.size() == 0)
			return false;

		switch (setTo.size())
		{
		case 64:
		case 128:
		case 256:
		case 384:
			break;
		default:
			return false;
		}
		return desc->setGenerator(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());
	}
	virtual tsCryptoData get_firstSeed() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getFirstSeed(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_firstSeed(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		if (setTo.size() == 0)
			return false;
		return desc->setFirstSeed(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());;
	}
	virtual tsCryptoData get_pSeed() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getPSeed(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_pSeed(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		if (setTo.size() == 0)
			return false;
		return desc->setPSeed(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());;
	}
	virtual tsCryptoData get_qSeed() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getQSeed(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_qSeed(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		if (setTo.size() == 0)
			return false;
		return desc->setQSeed(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());;
	}
	virtual tsCryptoData get_generatorFactor() const override
	{
		tsCryptoData tmp;
		uint32_t len = MAX_KEY_SIZE_BYTES;

		if (desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->getGeneratorFactor(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool set_generatorFactor(const tsCryptoData &setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		if (setTo.size() == 0)
			return false;
		return desc->setGeneratorFactor(desc, dhParams, setTo.c_str(), (uint32_t)setTo.size());;
	}
	virtual size_t get_pgen_counter() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->getPgenCounter(desc, dhParams);
	}
	virtual bool set_pgen_counter(size_t setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		return desc->setPgenCounter(desc, dhParams, (uint32_t)setTo);
	}
	virtual size_t get_qgen_counter() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->getQgenCounter(desc, dhParams);
	}
	virtual bool set_qgen_counter(size_t setTo) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		return desc->setQgenCounter(desc, dhParams, (uint32_t)setTo);
	}
	virtual bool generateProbablePrimeParameters(const tsCryptoStringBase& hashAlgName, size_t primeBitLength, size_t subprimeBitLength, size_t seedLen, const tsCryptoData &optionalFirstSeed, uint8_t index) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		return desc->generateProbablePrimeParameters(desc, dhParams, hashAlgName.c_str(), (uint32_t)primeBitLength, (uint32_t)subprimeBitLength, (uint32_t)seedLen, optionalFirstSeed.c_str(), (uint32_t)optionalFirstSeed.size(), index);
	}
	virtual bool generateProvablePrimeParameters(const tsCryptoStringBase& hashAlgName, size_t primeBitLength, size_t subprimeBitLength, size_t seedLen, const tsCryptoData &optionalFirstSeed, uint8_t index) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		return desc->generateProvablePrimeParameters(desc, dhParams, hashAlgName.c_str(), (uint32_t)primeBitLength, (uint32_t)subprimeBitLength, (uint32_t)seedLen, optionalFirstSeed.c_str(), (uint32_t)optionalFirstSeed.size(), index);
	}
	virtual bool validateParametersAndGenerator(const tsCryptoStringBase& hashAlgName, DH_Param_Gen_Type primeType, bool verifiableGenerator, uint8_t index) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;
		return desc->validateParametersAndGenerator(desc, dhParams, hashAlgName.c_str(), (DH_Param_Generator_Type)primeType, verifiableGenerator, index, &reason);
	}
	virtual bool validateParameters(const tsCryptoStringBase& hashAlgName, DH_Param_Gen_Type primeType) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;
		return desc->validateParameters(desc, dhParams, hashAlgName.c_str(), (DH_Param_Generator_Type)primeType, &reason);
	}
	virtual bool validateGenerator(const tsCryptoStringBase& hashAlgName, bool verifiableGenerator, uint8_t index) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;
		return desc->validateGenerator(desc, dhParams, hashAlgName.c_str(), verifiableGenerator, index);
	}
	virtual void Clear() override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return;

		dhParams = desc->createParametersetStructure(desc);
		reason = kvf_NoFailure;
	}
	virtual size_t ParameterSize() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->parameterSize(desc, dhParams);
	}
	virtual bool PrimesLoaded() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->primesLoaded(desc, dhParams);
	}
	virtual bool GeneratorLoaded() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->generatorLoaded(desc, dhParams);
	}
	virtual bool PrimesVerified() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->primesVerified(desc, dhParams);
	}
	virtual bool GeneratorVerified() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return 0;

		return desc->generatorVerified(desc, dhParams);
	}
	virtual bool ParametersAreCompatible(std::shared_ptr<DhParameters> obj) const override
	{
		std::shared_ptr<TSALG_Access> ts = std::dynamic_pointer_cast<TSALG_Access>(obj);

		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr || !ts)
			return false;
		if (!obj || !PrimesLoaded() || !obj->PrimesLoaded())
			return false;

		return desc->parametersAreCompatible(desc, dhParams, (CRYPTO_DH_PARAMS)ts->getKeyPair());
	}
	virtual bool computeGenerator(const tsCryptoStringBase& hashAlgName, const tsCryptoData &seed, uint8_t index) override
	{
		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return false;

		return desc->computeGenerator(desc, dhParams, hashAlgName.c_str(), seed.c_str(), (uint32_t)seed.size(), index);
	}
	virtual tsCryptoData toByteArray() const override
	{
		tsCryptoData tmp;
		uint32_t len = 20000;

		if (!gFipsState.operational() || desc == nullptr || dhParams == nullptr)
			return tsCryptoData();

		tmp.resize(len);
		if (!desc->exportParameterSet(desc, dhParams, tmp.rawData(), &len))
			return tsCryptoData();
		tmp.resize(len);
		return tmp;
	}
	virtual bool fromByteArray(const tsCryptoData &data) override
	{
		if (!gFipsState.operational() || desc == nullptr)
			return false;

		reason = kvf_NoFailure;
		SmartCryptoDhParams newParams = desc->createParametersetStructureFromBlob(desc, data.c_str(), (uint32_t)data.size());
		if (newParams != nullptr)
		{
			dhParams = std::move(newParams);
		}
		return newParams != nullptr;
	}
	virtual size_t minimumKeySizeInBits() const override
	{
		if (desc == nullptr)
			return 0;

		return desc->minimumKeySizeInBits(desc);
	}
	virtual size_t maximumKeySizeInBits() const override
	{
		if (desc == nullptr)
			return 0;

		return desc->maximumKeySizeInBits(desc);
	}
	virtual size_t keySizeIncrementInBits() const override
	{
		if (desc == nullptr)
			return 0;

		return desc->keySizeIncrementInBits(desc);
	}

	virtual std::shared_ptr<AsymmetricKey> generateKeyPair(bool forSignature)  override
	{
		std::shared_ptr<AsymmetricKey> key = std::dynamic_pointer_cast<AsymmetricKey>(CryptoFactory("KEY-DH"));
		std::shared_ptr<DhKey> dhkey = std::dynamic_pointer_cast<DhKey>(key);

		if (!!dhkey)
		{
			if (!dhkey->set_DomainParameters(std::dynamic_pointer_cast<DhParameters>(_me.lock())) || !key->generateKeyPair(forSignature))
			{
				key.reset();
			}
		}
		return key;
	}

	// tscrypto::IInitializableObject
	virtual bool InitializeWithFullName(const tscrypto::tsCryptoStringBase& fullName) override
	{
		SetName(fullName);

		return true;
	}

	// Inherited via TSALG_Access
	virtual const TSALG_Base_Descriptor * Descriptor() const override
	{
		return desc;
	}
	virtual CRYPTO_ASYMKEY getKeyPair() const override
	{
		return (CRYPTO_ASYMKEY)(CRYPTO_DH_PARAMS)dhParams;
	}
	virtual CRYPTO_WORKSPACE getWorkspace() const override
	{
		return nullptr;
	}
	virtual CRYPTO_ASYMKEY detachFromKeyPair() override
	{
        return (CRYPTO_ASYMKEY)dhParams.detach();
	}
	virtual CRYPTO_ASYMKEY cloneKeyPair() const override
	{
		if (desc == nullptr || dhParams == nullptr)
			return nullptr;
		return (CRYPTO_ASYMKEY)desc->cloneKey(desc, dhParams);
	}

private:
	const DH_Parameters_Descriptor* desc;
	SmartCryptoDhParams dhParams;
	tsalg_keyValidationFailureType reason;
};

tscrypto::ICryptoObject* CreateDhParameters(const tsCryptoStringBase& algorithm)
{
	return dynamic_cast<tscrypto::ICryptoObject*>(new DhParametersImpl(algorithm));
}


//bool DhParametersImpl::runTests(bool runDetailedTests)
//{
//    if (!gFipsState.operational())
//        return false;
//
//	if (runDetailedTests)
//	{
//		tsCryptoData p("a4aea10f0dd61125dc1b32c12b307c0996ff358c324930ff923a855b70b6ae98cced302b1761ebef15bd433b58ccbfd825285c005781887f7a2e7f0b1a4c6c062f610fc2b49aea2ca02a8e34ce59b1b87061a9fbab59dc0d6ca964e7bd1e8f2a0d9df08244de01645101bba904f86c1b0c6237dec0a0fa0b6407fd762a153a13", tsCryptoData::HEX);
//		tsCryptoData q("c5ec2bc8c38d06fbef07bd4b7ff879912360dce1", tsCryptoData::HEX);
//		tsCryptoData g("60d0441807e2b4b8b96d8ca1daae9baeed9e373fedb3fa4aaf7373980943e32ccf3c50c212230fefabe6d0f2491672f78c451c2b25bf08a67452a2dd594c4efd86497ce6adb7bfef3e91fd70ca13a228d206dc310942666d6132f8a217fe2b971ef14f884f5af7bf69e59a0481e5e6cf09b43270368459d20b726e5ffdce6ef5", tsCryptoData::HEX);
//		tsCryptoData seed("04be9c70dd8973a3de899a54a907064fd30a1e2d", tsCryptoData::HEX);
//		tsCryptoData h("00000002", tsCryptoData::HEX);
//		int c = 272;
//
//        if (!set_firstSeed(seed) || !set_pgen_counter(c) || !set_prime(p) || !set_subprime(q) || !set_generator(g) ||
//            !PrimesLoaded() || !validateParameters("SHA1", _DH_Param_Gen_Type::dhpg_Probable) || !set_generatorFactor(h) ||
//			!validateGenerator("SHA1", true, 1))
//        {
//            gFipsState.testFailed();
//			return false;
//        }
//		return true;
//	}
//    return true;
//}

