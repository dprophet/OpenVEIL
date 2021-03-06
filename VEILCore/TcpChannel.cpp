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

TcpChannel::TcpChannel()
{
}
TcpChannel::~TcpChannel(void)
{
}

void TcpChannel::SendLogout()
{
	if (!!m_processor)
		m_processor->Logout();
}

bool TcpChannel::Send(const tscrypto::tsCryptoData& _data)
{
	tscrypto::tsCryptoData data(_data);
	bool retVal = true;

	if (!WrapTransport(data))
		return false;

	if (!data.empty())
		retVal = RawSend(data);

	if (m_processor != nullptr && m_processor->shouldCloseAfterTransmit())
	{
		Disconnect();
	}
	return retVal;
}

bool TcpChannel::Receive(tscrypto::tsCryptoData& _data, size_t size)
{
	int len;
	tscrypto::tsCryptoData buff;
	//int requiredDataLength = 0;
	int targetLength = (int)size;

	_data.clear();
	while (true)
	{
		buff.resize(size);
#ifdef _WIN32
		len = recv(m_socket, (char*)buff.rawData(), targetLength, MSG_PEEK);
#else
		len = recv((int)m_socket, (char*)buff.rawData(), targetLength, MSG_PEEK);
#endif

		//
		// Is there data in the buffer?
		//
		if (len > 0)
		{
			//
			// Get it
			//
#ifdef _WIN32
			len = recv(m_socket, (char*)buff.rawData(), len, 0);
#else
			len = recv((int)m_socket, (char*)buff.rawData(), len, 0);
#endif
			if (len > 0)
			{
				buff.resize(len);

				LOG(httpData, "recv'd" << tscrypto::endl << buff.ToHexDump());

				if (m_processor != nullptr)
				{
					if (!m_processor->UnwrapTransport(buff))
					{
						m_errors += "Malformed response - Transport failed\n";
						_errorSignals.Fire(this, m_errors);
						return false;
					}
				}

				if (buff.size() > 0)
				{
					_data += buff;
				}
				else
				{
					continue;
				}
				return true;
			}
			else if (len == SOCKET_ERROR)
				return false;
			else
			{
				m_errors += "Unable to read the data from the socket\n";
				_errorSignals.Fire(this, m_errors);
				//
				// Data retrieval error (should never happen)
				//
				return false;
			}
		}
		else
		{
			return false;
		}
	}
}

bool TcpChannel::isAuthenticated() const
{
	if (!m_processor)
		return false;
	return m_processor->isAuthenticated();
}

bool TcpChannel::processAuthenticationMessages()
{
	if (!!m_processor)
	{
		do
		{
			if (!m_processor)
				return false;
			switch (m_processor->GetTransportState())
			{
			case IHttpChannelProcessor::inactive:
			case IHttpChannelProcessor::active:
			case IHttpChannelProcessor::logout:
				break;
			case IHttpChannelProcessor::login:
			{
				int len;
				tscrypto::tsCryptoData buff;

				buff.resize(1024);
#ifdef _WIN32
				len = recv(m_socket, (char*)buff.rawData(), 1023, MSG_PEEK);
#else
				len = recv((int)m_socket, (char*)buff.rawData(), 1023, MSG_PEEK);
#endif
				if (len > 0)
				{
#ifdef _WIN32
					len = recv(m_socket, (char*)buff.rawData(), len, 0);
#else
					len = recv((int)m_socket, (char*)buff.rawData(), len, 0);
#endif
					if (len > 0)
					{
						buff.resize(len);

						LOG(httpData, "recv'd" << tscrypto::endl << buff.ToHexDump());

						if (!m_processor || !m_processor->UnwrapTransport(buff))
							return false;
						if (buff.size() > 0)
							m_bufferedData << buff;

						if (!!m_processor && m_processor->shouldCloseAfterTransmit())
						{
							Disconnect();
							return false;
						}
					}
					else if (len == SOCKET_ERROR)
						return false;
				}
				else if (len == SOCKET_ERROR)
					return false;
				else
				{
					XP_Sleep(100);
				}
			}
			break;
			}
		} while (!!m_processor && m_processor->GetTransportState() == IHttpChannelProcessor::login);
	}
	return !!m_processor;
}

std::shared_ptr<IChannelProcessor> TcpChannel::getChannelProcessor() const
{
	return m_processor;
}
void TcpChannel::setChannelProcessor(std::shared_ptr<IChannelProcessor> setTo)
{
	m_processor.reset();
	m_processor = setTo;
}

bool TcpChannel::WrapTransport(tscrypto::tsCryptoData& content)
{
	if (!!m_processor)
		return m_processor->WrapTransport(content);

	return true;
}
bool TcpChannel::UnwrapTransport(tscrypto::tsCryptoData& content)
{
	if (!!m_processor)
		return m_processor->UnwrapTransport(content);

	return true;
}



std::shared_ptr<ITcpChannel> CreateTcpChannel()
{
	return ::TopServiceLocator()->Finish<ITcpChannel>(new TcpChannel());
}
