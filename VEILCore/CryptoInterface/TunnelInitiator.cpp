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
#include "CryptoAsn1.h"

#define MAX_DATA_SIZE 64000

using namespace tscrypto;

class TunnelInitiatorImpl : public IClientTunnel, public TSName, public Selftest, public tscrypto::ICryptoObject, public tscrypto::IInitializableObject, public AlgorithmInfo
{
public:
	TunnelInitiatorImpl(const tsCryptoStringBase& algorithm) : _keyHandler(nullptr), _ctrlChannel(nullptr)
	{
		desc = (const CkmTunnelInitiator_Descriptor*)findCkmAlgorithm("CKMTUNNEL-INITIATOR");
	}
	virtual ~TunnelInitiatorImpl(void)
	{
		StopTunnel();
		if (desc != nullptr && !workspace.empty())
			desc->finish(desc, workspace.rawData());
		workspace.clear();
	}

	// Selftests
	virtual bool runTests(bool runDetailedTests) override
	{
		if (desc != nullptr)
		{
			return desc->selftest(desc, runDetailedTests);
		}
		return true;
	}

	// AlgorithmInfo
	virtual tsCryptoString  AlgorithmName() const override
	{
		return GetName();
	}
	virtual tsCryptoString  AlgorithmOID() const override
	{
		return LookUpAlgOID(GetName());
	}
	virtual TS_ALG_ID AlgorithmID() const override
	{
		return LookUpAlgID(GetName());
	}

	// IClientTunnel
	virtual bool GetMessageAuthBitSize(int &pVal) override
	{
		pVal = 0;
		if (desc == nullptr)
			return false;
		pVal = desc->getMessageAuthBitSize(desc, workspace.rawData());
		return true;
	}
	virtual bool GetMessageAuth(tsCryptoData& pVal) override
	{
		uint32_t len;

		if (desc == nullptr || workspace.empty())
			return false;
		if (!desc->getMessageAuth(desc, workspace.rawData(), nullptr, &len) || len == 0)
			return false;

		pVal.resize(len);
		if (!desc->getMessageAuth(desc, workspace.rawData(), pVal.rawData(), &len) || len == 0)
			return false;
		pVal.resize(len);
		return true;
	}
	virtual bool TunnelActive() override
	{
		if (desc == nullptr || workspace.empty())
			return false;
		return desc->tunnelActive(desc, workspace.rawData());
	}
	virtual bool StartTunnel(const char* username, authenticationInitiatorTunnelKeyHandler* authHandler, authenticationControlDataCommunications* ctrlChannel) override
	{
		if (desc == nullptr || authHandler == NULL || ctrlChannel == NULL)
			return false;

		if (workspace.empty())
		{
			workspace.resize(desc->getWorkspaceSize(desc));
			if (!desc->configure(desc, workspace.rawData(), _encAlg.c_str(), _tagAlg.c_str(), _kdfAlg.c_str(), _kdfMacAlg.c_str()))
				return false;
			// Establish callbacks here

			if (!desc->set_sendControlDataFunction(desc, workspace.rawData(), this, &ctrl_sendControlData) ||
				!desc->set_sendReceivedDataFunction(desc, workspace.rawData(), this, &ctrl_sendReceivedData) ||
				!desc->set_stateChangedFunction(desc, workspace.rawData(), this, &event_stateChanged) ||
				!desc->set_failedEventFunction(desc, workspace.rawData(), this, &event_failed) ||
				!desc->set_loggedOutEventFunction(desc, workspace.rawData(), this, &event_loggedOut) ||
				!desc->set_AuthInfoFunction(desc, workspace.rawData(), this, &getAuthenticationInformationFn) ||
				!desc->set_onPacketReceived(desc, workspace.rawData(), this, &onPacketReceived) ||
				!desc->set_onPacketSent(desc, workspace.rawData(), this, &onPacketSent)
				)
			{
				return false;
			}
		}

		_keyHandler = authHandler;
		_ctrlChannel = ctrlChannel;

		if (!desc->startTunnel(desc, workspace.rawData(), username))
			return false;
		return true;
	}
	virtual bool StopTunnel() override
	{
		if (desc == nullptr || workspace.empty())
			return false;

		_keyHandler = nullptr;
		_ctrlChannel = nullptr;
		return desc->stopTunnel(desc, workspace.rawData());
	}
	virtual bool Logout() override
	{
		if (desc == nullptr || workspace.empty())
			return false;

		return desc->logout(desc, workspace.rawData());
	}
	virtual bool ReceiveData(const tsCryptoData& src) override
	{
		if (desc == nullptr || workspace.empty())
			return false;

		if (!desc->receiveData(desc, workspace.rawData(), src.c_str(), (uint32_t)src.size()))
			return false;
		
		return true;
	}
	virtual bool SendData(const tsCryptoData& src) override
	{
		if (desc == nullptr || workspace.empty())
			return false;

		if (!desc->sendData(desc, workspace.rawData(), src.c_str(), (uint32_t)src.size()))
			return false;

		return true;
	}
	virtual bool GetMessageEncryptionAlg(_POD_AlgorithmIdentifier & alg) override
	{
		if (desc == nullptr || workspace.empty())
			return false;

		tsCryptoData OID(desc->getMessageEncryptionOID(desc, workspace.rawData()), tsCryptoData::OID);
		if (OID.empty())
			return false;

		alg.clear();
		alg.set_oid(OID);
		return true;
	}
	virtual bool GetMessageHashAlg(_POD_AlgorithmIdentifier & alg) override
	{
		if (desc == nullptr || workspace.empty())
			return false;

		tsCryptoData OID(desc->getMessageHashOID(desc, workspace.rawData()), tsCryptoData::OID);
		if (OID.empty())
			return false;

		alg.clear();
		alg.set_oid(OID);
		return true;
	}
	virtual bool SetOnPacketRecievedCallback(std::function<void(uint8_t packetType, const uint8_t* data, uint32_t dataLen)> func)
	{
		_packetReceivedFn = func;
		return true;
	}
	virtual bool SetOnPacketSentCallback(std::function<void(uint8_t packetType, const uint8_t* data, uint32_t dataLen)> func)
	{
		_packetSentFn = func;
		return true;
	}
	virtual bool useCompression() override
	{
		return false;
	}
	virtual void useCompression(bool setTo) override
	{
	}

	// tscrypto::IInitializableObject
	virtual bool InitializeWithFullName(const tscrypto::tsCryptoStringBase& fullName) override
	{
		tsCryptoString algorithm(fullName);
		tsCryptoString alg(algorithm);
		tsCryptoStringList parts = algorithm.split(";");

		if (parts->size() < 2)
		{
			_encAlg = "GCM-AES";
			alg << ";" << _encAlg;
		}
		else
		{
			_encAlg = parts->at(1);
		}
		if (parts->size() < 3)
		{
			_tagAlg = "";
			alg << ";" << _tagAlg;
		}
		else
		{
			_tagAlg = parts->at(2);
		}
		if (parts->size() < 4)
		{
			_kdfAlg = "KDF-Sha512";
			alg << ";" << _kdfAlg;
			_kdfMacAlg = _kdfAlg.substr(4, 9999);
			if (_kdfMacAlg.empty())
				_kdfMacAlg = "SHA512";
			_kdfAlg.erase(3, 9999);
		}
		else
		{
			_kdfAlg = parts->at(3);
			_kdfMacAlg = _kdfAlg.substr(4, 9999);
			if (_kdfMacAlg.empty())
				_kdfMacAlg = "SHA512";
			_kdfAlg.erase(3, 9999);
		}

		SetName(alg);
		return true;
	}

private:
	const CkmTunnelInitiator_Descriptor* desc;
	tsCryptoData workspace;
	tsCryptoString _encAlg;
	tsCryptoString _tagAlg;
	tsCryptoString _kdfAlg;
	tsCryptoString _kdfMacAlg;
	authenticationInitiatorTunnelKeyHandler* _keyHandler;
	authenticationControlDataCommunications* _ctrlChannel;
	std::function<void(uint8_t packetType, const uint8_t* data, uint32_t dataLen)> _packetReceivedFn;
	std::function<void(uint8_t packetType, const uint8_t* data, uint32_t dataLen)> _packetSentFn;

	static ts_bool ctrl_sendControlData(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void *params, const uint8_t* dest, uint32_t dataLen)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		tsCryptoData tmp(dest, dataLen);
		if (This == nullptr || This->_ctrlChannel == nullptr)
			return ts_false;
		return This->_ctrlChannel->sendControlData(tmp);
	}
	static ts_bool ctrl_sendReceivedData(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void *params, const uint8_t* dest, uint32_t dataLen)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		tsCryptoData tmp(dest, dataLen);
		if (This == nullptr || This->_ctrlChannel == nullptr)
			return ts_false;
		return This->_ctrlChannel->sendReceivedData(tmp);
	}
	static void event_stateChanged(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void* params, ts_bool isActive, uint32_t currentState)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		if (This == nullptr || This->_ctrlChannel == nullptr)
			return;
		return This->_ctrlChannel->stateChanged(isActive, currentState);
	}
	static void event_failed(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void* params, const char *message)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		if (This == nullptr || This->_ctrlChannel == nullptr)
			return;
		return This->_ctrlChannel->failed(message);
	}
	static void event_loggedOut(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void* params)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		if (This == nullptr || This->_ctrlChannel == nullptr)
			return;
		return This->_ctrlChannel->loggedOut();
	}
	static ts_bool getAuthenticationInformationFn(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void* params, 
		const uint8_t* serverRequirements, uint32_t serverRequirementsLen, uint8_t* pVal, uint32_t* pValLen)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		tsCryptoData tmp(serverRequirements, serverRequirementsLen);
		tsCryptoData outTmp;
		if (This == nullptr || This->_keyHandler == NULL || pValLen == nullptr)
			return ts_false;

		outTmp = This->_keyHandler->getAuthenticationInformation(tmp);
		if (pVal == NULL)
		{
			*pValLen = (uint32_t)outTmp.size();
			return ts_true;
		}
		if (*pValLen < (uint32_t)outTmp.size())
		{
			*pValLen = (uint32_t)outTmp.size();
			return ts_false;
		}
		*pValLen = (uint32_t)outTmp.size();
		memcpy(pVal, outTmp.c_str(), *pValLen);
		return ts_true;
	}
	static void onPacketReceived(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void* params, uint8_t packetType, const uint8_t* packetData, uint32_t packetDataLen)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		if (This == nullptr || !This->_packetReceivedFn)
			return;
		This->_packetReceivedFn(packetType, packetData, packetDataLen);
	}
	static void onPacketSent(const struct CkmTunnelInitiator_Descriptor* desc, uint8_t* workspace, void* params, uint8_t packetType, const uint8_t* packetData, uint32_t packetDataLen)
	{
		TunnelInitiatorImpl* This = (TunnelInitiatorImpl*)params;
		if (This == nullptr || !This->_packetSentFn)
			return;
		This->_packetSentFn(packetType, packetData, packetDataLen);
	}
};

tscrypto::ICryptoObject* CreateTunnelInitiator(const tsCryptoStringBase& algorithm)
{
	return dynamic_cast<tscrypto::ICryptoObject*>(new TunnelInitiatorImpl(algorithm));
}
