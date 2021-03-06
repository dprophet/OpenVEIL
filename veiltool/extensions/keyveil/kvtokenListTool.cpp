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

enum options { OPT_HELP, OPT_URL, OPT_PASSWORD, OPT_USERNAME };

static const struct tsmod::OptionList Options[] = {
	{ "", "VEIL KEYVEIL TOKEN LIST options" },
	{ "", "=================================" },
	{ "--help, -h, -?", "This help information." },
	{ "-k, --keyveil=<url>", "The url to KeyVEIL" },
	{ "-u, --username=<name>", "The username to use to connect to KeyVEIL." },
	{ "-p, --password", "The password used to authenticate to KeyVEIL." },
	{ "", "" },
};

static const CSimpleOptA::SOption OptionList[] =
{
	{ OPT_HELP,              "-?",                  SO_NONE },
	{ OPT_HELP,              "-h",                  SO_NONE },
	{ OPT_HELP,              "--help",              SO_NONE },
	{ OPT_URL,               "-k",                  SO_REQ_SEP },
	{ OPT_URL,               "--keyveil",           SO_REQ_SEP },
	{ OPT_PASSWORD,          "-p",                  SO_REQ_SEP },
	{ OPT_PASSWORD,          "--password",          SO_REQ_SEP },
	{ OPT_USERNAME,          "-u",                  SO_REQ_SEP },
	{ OPT_USERNAME,          "--username",          SO_REQ_SEP },

	SO_END_OF_OPTIONS
};

class KeyVEILTokenListTool : public tsmod::IVeilToolCommand, public tsmod::IObject
{
public:
	KeyVEILTokenListTool()
	{}
	~KeyVEILTokenListTool()
	{}

	// tsmod::IObject
	virtual void OnConstructionFinished() override
	{
		utils = ::TopServiceLocator()->get_instance<tsmod::IVeilUtilities>("VeilUtilities");
	}

	// Inherited via tsmod::IVeilToolCommand
	virtual tscrypto::tsCryptoString getDescription() const override
	{
		return "List tokens";
	}
	virtual int RunCommand(CSimpleOptA & opts) override
	{
		int retVal = 1;
		tscrypto::tsCryptoString username, password, url;

		opts.Init(opts.FileCount(), opts.Files(), OptionList, SO_O_NOERR | SO_O_USEALL | SO_O_ICASE);
		while (opts.Next())
		{
			if (opts.LastError() == SO_SUCCESS)
			{
				if (opts.OptionId() == OPT_HELP)
				{
					Usage();
					return 0;
				}
				else if (opts.OptionId() == OPT_URL)
				{
					url = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_PASSWORD)
				{
					password = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_USERNAME)
				{
					username = opts.OptionArg();
				}
				else
				{
					Usage();
					return 1;
				}
			}
			else
			{
				Usage();
				return 1;
			}
		}

		std::shared_ptr<IKeyVEILConnector> connector = GetConnector(url, username, password);

		if (!connector)
		{
			printf("An error occurred while creating the KeyVEIL connector.\n");
			return 0;
		}

		size_t count = connector->tokenCount();
		for (size_t i = 0; i < count; i++)
		{
			std::shared_ptr<IToken> token = connector->token(i);

			printf("Token:  %s\n", token->tokenName().c_str());
			printf("    serial number:  %s\n         Token ID:  %s\n", token->serialNumber().ToHexString().c_str(), TSGuidToString(token->id()).c_str());
			printf("       Enterprise:  %s\n    Enterprise ID:  %s\n", token->enterpriseName().c_str(), TSGuidToString(token->enterpriseId()).c_str());
			printf("           Member:  %s\n        Member ID:  %s\n", token->memberName().c_str(), TSGuidToString(token->memberId()).c_str());

			std::shared_ptr<IKeyVEILSession> session = token->openSession();
			if (!!session)
			{
				printf("        Is Locked:  %s\n      Retry count:  %d\n", session->IsLocked() ? "true" : "false", (int)session->retriesLeft());
			}
			printf("\n");
		}


		return retVal;
	}
	virtual tscrypto::tsCryptoString getCommandName() const override
	{
		return "list";
	}
protected:
	void Usage()
	{
		utils->Usage(Options, sizeof(Options) / sizeof(Options[0]));
	}
protected:
	std::shared_ptr<tsmod::IVeilUtilities> utils;
};

tsmod::IObject* CreateKeyVEILTokenListTool()
{
	return dynamic_cast<tsmod::IObject*>(new KeyVEILTokenListTool());
}

