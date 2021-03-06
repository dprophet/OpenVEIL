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
#include "CkmWinscardConnectionImpl.h"

class CkmWinscardContextImpl : public tsmod::IObject, public ICkmWinscardContext, public ICkmWinscardHandle
{
public:
	CkmWinscardContextImpl(void) :
		m_context(0)
	{
		tsWinscardInit();
		tsSCardEstablishContext(SCARD_SCOPE_USER, nullptr, nullptr, &m_context);
	}
	virtual ~CkmWinscardContextImpl(void)
	{
		if (m_context != 0)
			tsSCardReleaseContext(m_context);
		m_context = 0;
		tsWinscardRelease();
	}

	virtual bool Connect(const tscrypto::tsCryptoString& readerName, uint32_t protocolsToAllow, std::shared_ptr<ICkmWinscardConnection>& pObj)
	{
		SCARDHANDLE card;
		DWORD protocol;

		if (m_context == 0)
			return false;

		if (tsSCardConnect(m_context, readerName.c_str(), SCARD_SHARE_SHARED, protocolsToAllow, &card, &protocol) != ERROR_SUCCESS)
			return false;
		pObj = CreateWinscardConnection(std::dynamic_pointer_cast<ICkmWinscardContext>(_me.lock()), readerName, card, protocol);

		if (!pObj)
			return false;
		return true;
	}
	virtual bool Cancel()
	{
		if (m_context == 0)
			return false;

		return tsSCardCancel(m_context) == ERROR_SUCCESS;
	}
	virtual bool IsValid()
	{
		if (m_context == 0)
			return false;

		return tsSCardIsValidContext(m_context) == ERROR_SUCCESS;
	}

	virtual INT_PTR GetHandle() const { return (INT_PTR)m_context; }

private:
	SCARDCONTEXT m_context;
};

std::shared_ptr<ICkmWinscardContext> CreateWinscardContext()
{
	return ::TopServiceLocator()->Finish<ICkmWinscardContext>(new CkmWinscardContextImpl());
}
