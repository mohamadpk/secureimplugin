#include "commonheaders.h"


// generate KeyA pair and return public key
LPSTR InitKeyA(pUinKey ptr,int features) {

    if(!ptr->cntx)
		ptr->cntx = cpp_create_context(isProtoSmallPackets(ptr->hContact)?MODE_BASE64:0);

	char *tmp = DBGetString(ptr->hContact,szModuleName,"PSK");
	if(tmp) {
		cpp_init_keyp(ptr->cntx,tmp);	// make pre-shared key from password
	    mir_free(tmp);
    }

	LPSTR pub_text = cpp_init_keya(ptr->cntx,features);	// calculate public and private key & fill KeyA

	LPSTR keysig;
	if(features&FEATURES_NEWPG) {
		if(features&KEY_B_SIG)
			keysig = (LPSTR)SIG_KEYB;
		else
			keysig = (LPSTR)SIG_KEYA;
	}
	else
	if(isProtoSmallPackets(ptr->hContact))
		keysig = (LPSTR)SIG_KEY4;
	else
		keysig = (LPSTR)SIG_KEY3;

	int tlen = strlen(pub_text);
	int slen = strlen(keysig);

	LPSTR keyToSend = (LPSTR) mir_alloc(slen+tlen+1);

	memcpy(keyToSend,keysig,slen);
	memcpy(keyToSend+slen,pub_text,tlen);

	return keyToSend;
}

// store KeyB into context
int InitKeyB(pUinKey ptr,LPCSTR key) {

    if(!ptr->cntx)
		ptr->cntx = cpp_create_context(isProtoSmallPackets(ptr->hContact)?MODE_BASE64:0);

	if(!cpp_keyp(ptr->cntx)) {
		char *tmp = DBGetString(ptr->hContact,szModuleName,"PSK");
		if(tmp) {
			cpp_init_keyp(ptr->cntx,tmp);	// make pre-shared key from password
		    mir_free(tmp);
	    }
	}
	ptr->features = cpp_get_features(ptr->cntx);
	cpp_init_keyb(ptr->cntx,key);

	return cpp_get_error(ptr->cntx);
}


// store KeyX into context
void InitKeyX(pUinKey ptr,BYTE *key) {

    if(!ptr->cntx)
		ptr->cntx = cpp_create_context(isProtoSmallPackets(ptr->hContact)?MODE_BASE64:0);

	cpp_set_keyx(ptr->cntx,key);
}


// calculate secret key
BOOL CalculateKeyX(pUinKey ptr,HANDLE hContact) {

	int agr = cpp_calc_keyx(ptr->cntx);
	if (agr) {
		// do this only if key exchanged is ok
		// we use a 192bit key
		int keysize = cpp_size_keyx();
		BYTE *buffer = (BYTE*)mir_alloc(keysize); // buffer for hash

		// store key
		cpp_get_keyx(ptr->cntx,buffer);

		DBCONTACTWRITESETTING cws;
		cws.szModule = szModuleName;

		// store key in database
		cws.szSetting = "offlineKey";
		cws.value.type = DBVT_BLOB;
		cws.value.cpbVal = keysize;
		cws.value.pbVal = buffer;
		CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);

		// store timeout of key in database (2 days)
		cws.szSetting = "offlineKeyTimeout";
		cws.value.type = DBVT_DWORD;
		cws.value.dVal = gettime()+(60*60*24*DBGetContactSettingWord(0,szModuleName,"okt",2));
		CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)hContact, (LPARAM)&cws);

		mir_free(buffer);
		// key exchange is finished released flag
		ptr->waitForExchange = false;
		showPopUpEC(hContact);
	}
	else {
		// agree value problem
		showPopUp(sim002,hContact,g_hPOP[POP_SECDIS],0);
	}
	return agr!=0;
}


// encrypt message
LPSTR encrypt(pUinKey ptr, LPCSTR szEncMsg) {

	LPSTR szSig = (LPSTR) (ptr->offlineKey?SIG_ENOF:SIG_ENON);

	int slen = strlen(szSig);
	int clen = strlen(szEncMsg);

	LPSTR szMsg = (LPSTR) mir_alloc(clen+slen+1);
	memcpy(szMsg, szSig, slen);
	memcpy(szMsg+slen, szEncMsg, clen);

	return szMsg;
}


// encode message
LPSTR encodeMsg(pUinKey ptr, LPARAM lParam) {

	CCSDATA *pccsd = (CCSDATA *)lParam;
	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = (LPSTR) pccsd->lParam;

	if(pccsd->wParam & PREF_UNICODE)
		szNewMsg = encrypt(ptr,cpp_encodeW(ptr->cntx,(LPWSTR)(szOldMsg+strlen(szOldMsg)+1)));
	else
		szNewMsg = encrypt(ptr,cpp_encodeA(ptr->cntx,szOldMsg));

	pccsd->wParam &= ~PREF_UNICODE;

	return szNewMsg;
}


// decode message
LPSTR decodeMsg(pUinKey ptr, LPARAM lParam, LPSTR szEncMsg) {

	CCSDATA *pccsd = (CCSDATA *)lParam;
	PROTORECVEVENT *ppre = (PROTORECVEVENT *)pccsd->lParam;

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = cpp_decode(ptr->cntx, szEncMsg);

	if(szOldMsg == NULL) {
		ptr->decoded=false;
		switch(cpp_get_error(ptr->cntx)) {
		case ERROR_BAD_LEN:
			szNewMsg = mir_strdup(Translate(sim102));
			break;
		case ERROR_BAD_CRC:
			szNewMsg = mir_strdup(Translate(sim103));
			break;
		default: {
			ptr->decoded=true;
    		szNewMsg = mir_strdup(Translate(sim101));
    		}
			break;
		}
		ppre->flags &= ~PREF_UNICODE;
		pccsd->wParam &= ~PREF_UNICODE;
	}
	else {
		ptr->decoded=true;
	    int olen = (strlen(szOldMsg)+1)*(sizeof(WCHAR)+1);
		szNewMsg = (LPSTR) mir_alloc(olen);
		memcpy(szNewMsg,szOldMsg,olen);

		ppre->flags |= PREF_UNICODE;
		pccsd->wParam |= PREF_UNICODE;
	}
	ppre->szMessage = szNewMsg;
	return szNewMsg;
}


BOOL LoadKeyPGP(pUinKey ptr) {
   	int mode = DBGetContactSettingByte(ptr->hContact,szModuleName,"pgp_mode",255);
   	if(mode==0) {
   		DBVARIANT dbv;
   		DBGetContactSetting(ptr->hContact,szModuleName,"pgp",&dbv);
		BOOL r=(dbv.type==DBVT_BLOB);
		if(r) pgp_set_keyid(ptr->cntx,(PVOID)dbv.pbVal);
		DBFreeVariant(&dbv);
		return r;
   	}
   	else
   	if(mode==1) {
   		LPSTR key = DBGetStringDecode(ptr->hContact,szModuleName,"pgp");
		if( key ) {
   			pgp_set_key(ptr->cntx,key);
   			mir_free(key);
	   		return 1;
   		}
   	}
	return 0;
}


BOOL LoadKeyGPG(pUinKey ptr) {

	LPSTR key = DBGetString(ptr->hContact,szModuleName,"gpg");
	if( key ) {
		gpg_set_keyid(ptr->cntx,key);
		mir_free(key);
	   	return 2;
	}
   	return 0;
}

// EOF
