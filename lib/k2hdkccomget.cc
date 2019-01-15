/*
 * 
 * K2HDKC
 * 
 * Copyright 2016 Yahoo Japan Corporation.
 * 
 * K2HDKC is k2hash based distributed KVS cluster.
 * K2HDKC uses K2HASH, CHMPX, FULLOCK libraries. K2HDKC supports
 * distributed KVS cluster server program and client libraries.
 * 
 * For the full copyright and license information, please view
 * the license file that was distributed with this source code.
 *
 * AUTHOR:   Takeshi Nakatani
 * CREATE:   Wed Jul 20 2016
 * REVISION:
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "k2hdkccomget.h"
#include "k2hdkcutil.h"
#include "k2hdkcdbg.h"

using namespace	std;

//---------------------------------------------------------
// Constructor/Destructor
//---------------------------------------------------------
K2hdkcComGet::K2hdkcComGet(K2HShm* pk2hash, ChmCntrl* pchmcntrl, uint64_t comnum, bool without_self, bool is_routing_on_server, bool is_wait_on_server) : K2hdkcCommand(pk2hash, pchmcntrl, comnum, without_self, is_routing_on_server, is_wait_on_server, DKC_COM_GET)
{
	strClassName = "K2hdkcComGet";
}

K2hdkcComGet::~K2hdkcComGet(void)
{
}

//---------------------------------------------------------
// Methods
//---------------------------------------------------------
PDKCCOM_ALL K2hdkcComGet::MakeResponseOnlyHeadData(dkcres_type_t subcode, dkcres_type_t errcode)
{
	PDKCCOM_ALL	pComErr;
	if(NULL == (pComErr = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET))))){
		ERR_DKCPRN("Could not allocate memory.");
		return NULL;
	}
	PDKCRES_GET	pComRes			= CVT_DKCRES_GET(pComErr);
	pComRes->head.comtype		= DKC_COM_GET;
	pComRes->head.restype		= COMPOSE_DKC_RES(errcode, subcode);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET);
	pComRes->val_offset			= sizeof(DKCRES_GET);
	pComRes->val_length			= 0;

	return pComErr;
}

bool K2hdkcComGet::SetResponseData(const unsigned char* pValue, size_t ValLen)
{
	if(!pValue || 0 == ValLen){
		ERR_DKCPRN("Parameter are wrong.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCRES_GET) + ValLen)))){
		ERR_DKCPRN("Could not allocate memory.");
		SetErrorResponseData(DKC_RES_SUBCODE_NOMEM);		// could this set?
		return false;
	}
	PDKCRES_GET	pComRes			= CVT_DKCRES_GET(pComAll);
	pComRes->head.comtype		= DKC_COM_GET;
	pComRes->head.restype		= COMPOSE_DKC_RES(DKC_RES_SUCCESS, DKC_RES_SUBCODE_NOTHING);
	pComRes->head.comnumber		= GetComNumber();
	pComRes->head.dispcomnumber	= GetDispComNumber();
	pComRes->head.length		= sizeof(DKCRES_GET) + ValLen;
	pComRes->val_offset			= sizeof(DKCRES_GET);
	pComRes->val_length			= ValLen;

	unsigned char*	pdata		= CVT_BIN_DKC_COM_ALL(pComAll, pComRes->val_offset, pComRes->val_length);
	if(!pdata){
		ERR_DKCPRN("Could not convert respond data area.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	memcpy(pdata, pValue, ValLen);

	if(!SetResponseData(pComAll)){
		ERR_DKCPRN("Failed to set send data.");
		DKC_FREE(pComAll);
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}
	return true;
}

bool K2hdkcComGet::CommandProcessing(void)
{
	if(!pChmObj || !pRcvComPkt || !pRcvComAll || 0 == RcvComLength){
		ERR_DKCPRN("This object does not initialize yet.");
		SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
		return false;
	}

	if(DKC_NORESTYPE == pRcvComAll->com_head.restype){
		//
		// receive data is command
		//
		if(!pK2hObj || !IsServerNode){
			ERR_DKCPRN("This object does not initialize yet.");
			SetErrorResponseData(DKC_RES_SUBCODE_INTERNAL);
			return false;
		}

		// get command data
		const PDKCCOM_GET		pCom	= CVT_DKCCOM_GET(pRcvComAll);
		const unsigned char*	pKey	= GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->key_offset, pCom->key_length);
		char*					pEncPass= NULL;
		if(0 < pCom->encpass_length){
			pEncPass					= reinterpret_cast<char*>(k2hbindup(GET_BIN_DKC_COM_ALL(pRcvComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length));
		}
		if(!pKey){
			ERR_DKCPRN("Could not get safe key data(%zd offset, %zu byte) in DKCCOM_GET(%p).", pCom->key_offset, pCom->key_length, pRcvComAll);
			SetErrorResponseData(DKC_RES_SUBCODE_INVAL);
			DKC_FREE(pEncPass);
			return false;
		}

		// do command
		unsigned char*	pValue = NULL;
		ssize_t			ValLen;
		if(0 >= (ValLen = pK2hObj->Get(pKey, pCom->key_length, &pValue, pCom->check_attr, pEncPass))){
			MSG_DKCPRN("Error or No data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			SetErrorResponseData(DKC_RES_SUBCODE_NODATA, DKC_RES_SUCCESS);
			DKC_FREE(pEncPass);
			return true;															// [NOTE] result is success
		}
		DKC_FREE(pEncPass);

		// set response data
		if(!SetResponseData(pValue, static_cast<size_t>(ValLen))){
			MSG_DKCPRN("Failed to make response data for key(%s)", bin_to_string(pKey, pCom->key_length).c_str());
			// continue for responding
		}
		DKC_FREE(pValue);

	}else{
		//
		// receive data is response
		//
		// nothing to do
		MSG_DKCPRN("Received data is response data, so nothing to do here.");
	}
	return true;
}

bool K2hdkcComGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass)
{
	if(!pkey || 0 == keylength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}

	// make send data
	PDKCCOM_ALL	pComAll;
	if(NULL == (pComAll = reinterpret_cast<PDKCCOM_ALL>(malloc(sizeof(DKCCOM_GET) + keylength + (encpass ? (strlen(encpass) + 1) : 0))))){
		ERR_DKCPRN("Could not allocate memory.");
		return false;
	}
	PDKCCOM_GET	pComGet			= CVT_DKCCOM_GET(pComAll);
	pComGet->head.comtype		= DKC_COM_GET;
	pComGet->head.restype		= DKC_NORESTYPE;
	pComGet->head.comnumber		= GetComNumber();
	pComGet->head.dispcomnumber	= GetDispComNumber();
	pComGet->head.length		= sizeof(DKCCOM_GET) + keylength + (encpass ? (strlen(encpass) + 1) : 0);
	pComGet->key_offset			= sizeof(DKCCOM_GET);
	pComGet->key_length			= keylength;
	pComGet->encpass_offset		= sizeof(DKCCOM_GET) + keylength;
	pComGet->encpass_length		= encpass ? (strlen(encpass) + 1) : 0;
	pComGet->check_attr			= checkattr;

	unsigned char*	pdata		= reinterpret_cast<unsigned char*>(pComGet) + pComGet->key_offset;
	memcpy(pdata, pkey, keylength);
	if(encpass){
		pdata					= reinterpret_cast<unsigned char*>(pComGet) + pComGet->encpass_offset;
		memcpy(pdata, encpass, strlen(encpass) + 1);
	}

	if(!SetSendData(pComAll, K2hdkcCommand::MakeChmpxHash((reinterpret_cast<unsigned char*>(pComGet) + pComGet->key_offset), keylength))){
		ERR_DKCPRN("Failed to set command data to internal buffer.");
		DKC_FREE(pComAll);
		return false;
	}

	// do command & receive response(on slave node)
	if(!CommandSend()){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return true;
}

bool K2hdkcComGet::CommandSend(const unsigned char* pkey, size_t keylength, bool checkattr, const char* encpass, const unsigned char** ppval, size_t* pvallength, dkcres_type_t* prescode)
{
	if(!pChmObj){
		ERR_DKCPRN("Chmpx object is NULL, so could not send command.");
		return false;
	}

	if(!CommandSend(pkey, keylength, checkattr, encpass)){
		ERR_DKCPRN("Failed to send command.");
		return false;
	}
	return GetResponseData(ppval, pvallength, prescode);
}

bool K2hdkcComGet::GetResponseData(const unsigned char** ppdata, size_t* plength, dkcres_type_t* prescode) const
{
	if(!ppdata || !plength){
		ERR_DKCPRN("Parameter are wrong.");
		return false;
	}
	PDKCCOM_ALL	pcomall = NULL;
	if(!K2hdkcCommand::GetResponseData(&pcomall, NULL, prescode) || !pcomall){
		ERR_DKCPRN("Failed to get result.");
		return false;
	}
	if(DKC_NORESTYPE == pcomall->com_head.restype){
		ERR_DKCPRN("Response(received) data is not received data type.");
		return false;
	}
	PDKCRES_GET	pResGet = CVT_DKCRES_GET(pcomall);
	if(DKC_COM_GET != pResGet->head.comtype || DKC_NORESTYPE == pResGet->head.restype || RcvComLength != pResGet->head.length){
		ERR_DKCPRN("Response(received) data is something wrong(internal error: data is invalid).");
		return false;
	}
	if(ppdata && plength){
		*ppdata		= reinterpret_cast<const unsigned char*>(pResGet) + pResGet->val_offset;
		*plength	= pResGet->val_length;
	}
	return true;
}

//---------------------------------------------------------
// Methods - Dump
//---------------------------------------------------------
void K2hdkcComGet::RawDumpComAll(const PDKCCOM_ALL pComAll) const
{
	assert(DKC_COM_GET == pComAll->com_head.comtype);

	if(DKC_NORESTYPE == pComAll->com_head.restype){
		PDKCCOM_GET	pCom = CVT_DKCCOM_GET(pComAll);

		RAW_DKCPRN("  DKCCOM_GET = {");
		RAW_DKCPRN("    key_offset      = (%016" PRIx64 ") %s",	pCom->key_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->key_offset, pCom->key_length), pCom->key_length).c_str());
		RAW_DKCPRN("    key_length      = %zu",					pCom->key_length);
		RAW_DKCPRN("    encpass_offset  = (%016" PRIx64 ") %s",	pCom->encpass_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->encpass_offset, pCom->encpass_length), pCom->encpass_length).c_str());
		RAW_DKCPRN("    encpass_length  = %zu",					pCom->encpass_length);
		RAW_DKCPRN("    check_attr      = %s",					pCom->check_attr ? "yes" : "no");
		RAW_DKCPRN("  }");

	}else{
		PDKCRES_GET	pCom = CVT_DKCRES_GET(pComAll);

		RAW_DKCPRN("  DKCRES_GET = {");
		RAW_DKCPRN("    val_offset      = (%016" PRIx64 ") %s",	pCom->val_offset, bin_to_string(GET_BIN_DKC_COM_ALL(pComAll, pCom->val_offset, pCom->val_length), pCom->val_length).c_str());
		RAW_DKCPRN("    val_length      = %zu",					pCom->val_length);
		RAW_DKCPRN("  }");
	}
}

/*
 * VIM modelines
 *
 * vim:set ts=4 fenc=utf-8:
 */
