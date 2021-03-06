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

extern const char *resolveSocketError(DWORD error);

class TcpConnection : public ITcpConnection, public INetworkConnectionEvents, public tsmod::IObject
{
public:
	TcpConnection();
	virtual ~TcpConnection(void);

	virtual const tscrypto::tsCryptoString &Server() const override;
	virtual void Server(const tscrypto::tsCryptoString &setTo) override;

	virtual unsigned short Port() const override;
	virtual void Port(unsigned short setTo) override;

	virtual tscrypto::tsCryptoString Errors() const override;
	virtual void ClearErrors() override;
	virtual bool RawSend(const tscrypto::tsCryptoData& data) override;
	virtual bool RawReceive(tscrypto::tsCryptoData& _data, size_t size) override;

	virtual bool isConnected() const override;

	virtual bool Disconnect() override;
	virtual bool Connect() override;

protected:
	virtual bool isWSAInitialized() const;
	virtual bool WSAInitialize();
	virtual void flushBuffer();

	// Inherited via INetworkConnectionEvents
	virtual size_t AddOnConnect(std::function<void(const tsmod::IObject*)> func) override;
	virtual void RemoveOnConnect(size_t cookie) override;
	virtual size_t AddOnError(std::function<void(const tsmod::IObject*, const tscrypto::tsCryptoStringBase&)> func) override;
	virtual void RemoveOnError(size_t cookie) override;
	virtual size_t AddOnDisconnect(std::function<void(const tsmod::IObject*)> func) override;
	virtual void RemoveOnDisconnect(size_t cookie) override;

protected:
	bool                 m_WSAinitialized;
	mutable bool                 m_isConnected;
	struct sockaddr_storage      m_serverInfo;
	SOCKET               m_socket;
	tscrypto::tsCryptoString              m_server;
	unsigned short       m_port;
	tscrypto::tsCryptoString              m_errors;
	tscrypto::tsCryptoData               m_bufferedData;
	tsIObjectSignal _connectSignals;
	tsIObjStringSignal _errorSignals;
	tsIObjectSignal _disconnectSignals;

	std::shared_ptr<tsmod::IBaseProtocolStack> _stack;
	tscrypto::tsCryptoData m_receivedDataFromStack;
};

