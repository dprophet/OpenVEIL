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

//////////////////////////////////////////////////////////////////////////////////
/// \file CkmReadWriteFile.h
/// \brief Contains the implementation of the CKM file reader and writer in memory.
//////////////////////////////////////////////////////////////////////////////////

#ifndef __CKMREADWRITEFILE_H__
#define __CKMREADWRITEFILE_H__

#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>Creates a CkmReadWriteFile object instance.</summary>
///
/// <param name="filename">Filename of the file.</param>
/// <param name="pVal">	   [in,out] The CkmReadWriteFile object.</param>
///
/// <returns>S_OK for success or a standard COM error code for failure.</returns>
////////////////////////////////////////////////////////////////////////////////////////////////////
VEILCORE_API std::shared_ptr<IDataIOBase> CreateReadWriteFile(const tscrypto::tsCryptoString& filename);

#endif // __CKMREADWRITEFILE_H__

