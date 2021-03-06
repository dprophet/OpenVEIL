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

enum {
	OPT_HELP = 0, OPT_SERVER, OPT_USERNAME, OPT_PASSWORD, OPT_PRIVATEKEY, OPT_PEM, OPT_KEYID, OPT_KEYNAME, OPT_KEYHASH, OPT_PRIVATEKEY_PASSWORD, OPT_FILENAME
};

//printf("USAGE: GetMyCTS <username> <ServerUrl>\n");

static const struct tsmod::OptionList options[] = {
	{ "", "VEIL tool KEYVEIL KEY EXPORT options" },
	{ "", "====================================" },
	{ "--help, -h, -?", "This help information." },
	{ "-s, --server=url", "The URL of KeyVEIL ." },
	{ "-u, --username=user", "Your user name within that KeyVEIL." },
	{ "-p, --password=pin", "The password to use (optional).  If the password is not specified here then you will be prompted for it when needed." },
	{ "-f, --filename=filenamen", "The output file name." },
	{ "--private-key", "Export the private key." },
	{ "--private-key-password=password", "The password to use to protect the private key." },
	{ "pem", "Export in PEM format" },
	{ "--key-id=id", "The ID of the key to be exported." },
	{ "--key-name=name", "The Name of the key to be exported." },
	{ "--key-hash=hash", "The SHA512 hash of the key to be exported." },
	{ "", "" },
};
static const CSimpleOptA::SOption g_rgOptions1[] =
{
	{ OPT_HELP, "-?", SO_NONE },
	{ OPT_HELP, "-h", SO_NONE },
	{ OPT_HELP, "--help", SO_NONE },
	{ OPT_SERVER, "-s", SO_REQ_SEP },
	{ OPT_SERVER, "--server", SO_REQ_SEP },
	{ OPT_USERNAME, "-u", SO_REQ_SEP },
	{ OPT_USERNAME, "--username", SO_REQ_SEP },
	{ OPT_PASSWORD, "-p", SO_REQ_SEP },
	{ OPT_PASSWORD, "--password", SO_REQ_SEP },
	{ OPT_PRIVATEKEY, "--private-key", SO_NONE },
	{ OPT_PEM, "pem", SO_NONE },
	{ OPT_PRIVATEKEY_PASSWORD, "--private-key-password", SO_REQ_SEP },
	{ OPT_FILENAME, "-f", SO_REQ_SEP },
	{ OPT_FILENAME, "--filename", SO_REQ_SEP },

	{ OPT_KEYID, "--key-id", SO_REQ_SEP },
	{ OPT_KEYNAME, "--key-name", SO_REQ_SEP },
	{ OPT_KEYHASH, "--key-hash", SO_REQ_SEP },

	SO_END_OF_OPTIONS
};

class ExportKeyTool : public tsmod::IVeilToolCommand, public tsmod::IObject
{
public:
	ExportKeyTool()
	{}
	~ExportKeyTool()
	{}

	// tsmod::IObject
	virtual void OnConstructionFinished() override
	{
		utils = ::TopServiceLocator()->get_instance<tsmod::IVeilUtilities>("VeilUtilities");
	}

	// Inherited via tsmod::IVeilToolCommand
	virtual tscrypto::tsCryptoString getDescription() const override
	{
		return "Export the specified key.";
	}
	virtual int RunCommand(CSimpleOptA & opts) override
	{
//		int64_t start;
		tscrypto::tsCryptoString username, url, filename;
		tscrypto::tsCryptoData outData;
		JSONObject response;
//		int status;
		tscrypto::tsCryptoString password;
		bool privateKeyAlso = false;
		bool asPEM = false;
		JSONObject cmdData;
		GUID keyId;
		tscrypto::tsCryptoData keyHash;
		tscrypto::tsCryptoString keyName;
		tscrypto::tsCryptoString pkPassword;

		opts.Init(opts.FileCount(), opts.Files(), g_rgOptions1, SO_O_NOERR | SO_O_USEALL | SO_O_ICASE);

		while (opts.Next())
		{
			if (opts.LastError() == SO_SUCCESS)
			{
				if (opts.OptionId() == OPT_SERVER)
				{
					url = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_USERNAME)
				{
					username = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_PASSWORD)
				{
					password = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_HELP)
				{
					Usage();
					return 0;
				}
				else if (opts.OptionId() == OPT_PRIVATEKEY)
				{
					privateKeyAlso = true;
				}
				else if (opts.OptionId() == OPT_PEM)
				{
					asPEM = true;
				}
				else if (opts.OptionId() == OPT_KEYID)
				{
					keyId = ToGuid()(opts.OptionArg());
				}
				else if (opts.OptionId() == OPT_KEYNAME)
				{
					keyName = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_KEYHASH)
				{
					keyHash = tscrypto::tsCryptoString(opts.OptionArg()).HexToData();
				}
				else if (opts.OptionId() == OPT_PRIVATEKEY_PASSWORD)
				{
					pkPassword = opts.OptionArg();
				}
				else if (opts.OptionId() == OPT_FILENAME)
				{
					pkPassword = opts.OptionArg();
				}
				else 
				{
					ERROR("Unknown option: " << opts.OptionText());
					return 8;
				}
			}
			else
			{
				ERROR("Invalid arguments detected.");
				Usage();
				return 9;
			}
		}

		if (opts.FileCount() > 0)
		{
			ERROR("Unknown options were detected.");
			return 12;
		}

		// Connect to the KeyVEIL server
		std::shared_ptr<IKeyVEILConnector> connector = GetConnector(url, username, password);

		if (!connector)
		{
			printf("An error occurred while creating the KeyVEIL connector.\n");
			return 0;
		}

		//cmdData
		//	.add("action", "EXPORT")

		//start = GetTicks();
		//LOG(gHttpLog, "POST Key");
		//start = GetTicks();
		//if (!connector->sendJsonRequest("POST", "Key", cmdData, response, status))
		//{
		//	LOG(gHttpLog, "  Failed to retrieve the list of keys for this user");
		//	ERROR("Failed to retrieve the list of keys for this user");
		//	connector->disconnect();
		//	return false;
		//}
		//LOG(gHttpLog, "  Success - " << (GetTicks() - start) / 1000.0 << " ms  Status:  " << status);

		//if (status >= 400)
		//{
		//	connector->disconnect();
		//	return false;
		//}

		//printf("Name                                    ID                                     Type  Spec    Length\n");
		//printf("===================================================================================================\n");

		//if (response.hasField("KeyCollection"))
		//{
		//	for (auto it : *response.AsArray("KeyCollection"))
		//	{
		//		JSONObject obj = it.AsObject();

		//		printf("%-39s %-38s %-5s %-7s   %4d\n", obj.AsString("name").c_str(), obj.AsString("Id").c_str(), obj.AsString("type").c_str(), obj.AsString("spec").c_str(), (int)obj.AsNumber("length"));
		//	}
		//}
		//// Disconnect from the CTS server
		//connector->disconnect();
		//connector.reset();

		// Shutdown the system
		return 0;
	}
	virtual tscrypto::tsCryptoString getCommandName() const override
	{
		return "list";
	}
protected:
	void Usage()
	{
		utils->Usage(options, sizeof(options) / sizeof(options[0]));
	}
protected:
	std::shared_ptr<tsmod::IVeilUtilities> utils;
};

tsmod::IObject* HIDDEN CreateExportKeyTool()
{
	return dynamic_cast<tsmod::IObject*>(new ExportKeyTool());
}

