#include "commonheaders.h"


HMODULE hpgpsdk;
UINT	pgpVer;
HRSRC	hRS_pgp;
PBYTE	pRS_pgp;


int   __cdecl _pgp_init(void);
int   __cdecl _pgp_done(void);
int   __cdecl _pgp_open_keyrings(LPSTR,LPSTR);
int   __cdecl _pgp_close_keyrings(void);
int   __cdecl _pgp_get_version(void);
LPSTR __cdecl _pgp_get_error(void);
int   __cdecl _pgp_size_keyid(void);
PVOID __cdecl _pgp_select_keyid(HWND,LPSTR);
LPSTR __cdecl _pgp_encrypt_keydb(LPCSTR,PVOID);
LPSTR __cdecl _pgp_decrypt_keydb(LPCSTR);
//int   __cdecl _pgp_check_key(LPCSTR);
LPSTR __cdecl _pgp_encrypt_key(LPCSTR,LPCSTR);
LPSTR __cdecl _pgp_decrypt_key(LPCSTR,LPCSTR);

int   (__cdecl *p_pgp_init)(void);
int   (__cdecl *p_pgp_done)(void);
int   (__cdecl *p_pgp_open_keyrings)(LPSTR,LPSTR);
int   (__cdecl *p_pgp_close_keyrings)(void);
int   (__cdecl *p_pgp_get_version)(void);
LPSTR (__cdecl *p_pgp_get_error)(void);
int   (__cdecl *p_pgp_size_keyid)(void);
PVOID (__cdecl *p_pgp_select_keyid)(HWND,LPSTR);
LPSTR (__cdecl *p_pgp_encrypt_keydb)(LPCSTR,PVOID);
LPSTR (__cdecl *p_pgp_decrypt_keydb)(LPCSTR);
//int   (__cdecl *p_pgp_check_key)(LPCSTR);
LPSTR (__cdecl *p_pgp_encrypt_key)(LPCSTR,LPCSTR);
LPSTR (__cdecl *p_pgp_decrypt_key)(LPCSTR,LPCSTR);


#define GPA(x)                                              \
  {                                                         \
    *((PVOID*)&p##x) = (PVOID)GetProcAddress(mod, TEXT(#x)); \
    if (!p##x) {                                            \
      return 0;                                             \
    }                                                       \
  }

int load_pgpsdk_dll(HMODULE mod) {

	GPA(_pgp_init);
	GPA(_pgp_done);
	GPA(_pgp_open_keyrings);
	GPA(_pgp_close_keyrings);
	GPA(_pgp_get_version);
	GPA(_pgp_get_error);
	GPA(_pgp_size_keyid);
	GPA(_pgp_select_keyid);
	GPA(_pgp_encrypt_keydb);
	GPA(_pgp_decrypt_keydb);
//	GPA(_pgp_check_key);
	GPA(_pgp_encrypt_key);
	GPA(_pgp_decrypt_key);

	return 1;
}

#undef GPA


#define GPA(x)                                              \
  {                                                         \
    *((PVOID*)&p##x) = (PVOID)MemGetProcAddress(mod, TEXT(#x)); \
    if (!p##x) {                                            \
      return 0;                                             \
    }                                                       \
  }

int load_pgpsdk_mem(HMODULE mod) {

	GPA(_pgp_init);
	GPA(_pgp_done);
	GPA(_pgp_open_keyrings);
	GPA(_pgp_close_keyrings);
	GPA(_pgp_get_version);
	GPA(_pgp_get_error);
	GPA(_pgp_size_keyid);
	GPA(_pgp_select_keyid);
	GPA(_pgp_encrypt_keydb);
	GPA(_pgp_decrypt_keydb);
//	GPA(_pgp_check_key);
	GPA(_pgp_encrypt_key);
	GPA(_pgp_decrypt_key);

	return 1;
}

#undef GPA


BOOL load_pgp_sdk(int type, int id) 
{
	int r; char t[MAX_PATH];
	pgpVer = 0;

	if( isVista ){
		sprintf(t,"%s\\pgpsdkw.dll",TEMP);
		ExtractFile(t,type,id);
		hpgpsdk = LoadLibraryA(t);
	}
	else {
		hRS_pgp = FindResource( g_hInst, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type) );
		pRS_pgp = (PBYTE) LoadResource( g_hInst, hRS_pgp ); LockResource( pRS_pgp );
		hpgpsdk = MemLoadLibrary( pRS_pgp );
	}
	if (hpgpsdk) {
		if( isVista )	load_pgpsdk_dll(hpgpsdk);
		else			load_pgpsdk_mem(hpgpsdk);
		r = p_pgp_init();
		if(r) {
			pgpVer = p_pgp_get_version();
			return r;
		}
		if( isVista ){
			FreeLibrary(hpgpsdk);
		}
		else {
			MemFreeLibrary(hpgpsdk);
			UnlockResource( pRS_pgp );
			FreeResource( pRS_pgp );
		}
	}
	return 0;
}


int __cdecl pgp_init()
{
	int r;

	if( r = load_pgp_sdk(666,6) ) return r;
	if( r = load_pgp_sdk(666,8) ) return r;

    hpgpsdk = 0;

	return 0;
}


int __cdecl pgp_done()
{
    int r = 0;
    pgpVer = 0;
    if(hpgpsdk) {
    	r = p_pgp_done();
		if( isVista ){
			FreeLibrary(hpgpsdk);
		}
		else {
			MemFreeLibrary(hpgpsdk);
			UnlockResource( pRS_pgp );
			FreeResource( pRS_pgp );
		}
		hpgpsdk = 0;
    }
	return r;
}


int __cdecl pgp_open_keyrings(LPSTR PubRingPath, LPSTR SecRingPath)
{
	return p_pgp_open_keyrings(PubRingPath,SecRingPath);
}


int __cdecl pgp_close_keyrings()
{
	return p_pgp_close_keyrings();
}


int __cdecl pgp_get_version()
{
	return pgpVer;
}


LPSTR __cdecl pgp_get_error()
{
	return p_pgp_get_error();
}


LPSTR __cdecl pgp_encrypt(pCNTX ptr, LPCSTR szPlainMsg)
{
   	ptr->error = ERROR_NONE;
	pPGPDATA p = (pPGPDATA) ptr->pdata;
	SAFE_FREE(ptr->tmp);

	LPSTR szEncMsg;
	if(p->pgpKey) 
		szEncMsg = p_pgp_encrypt_key(szPlainMsg,(LPCSTR)p->pgpKey);
	else
		szEncMsg = p_pgp_encrypt_keydb(szPlainMsg,p->pgpKeyID);
	if(!szEncMsg) return 0;

	DWORD dwEncMsgLen = strlen(szEncMsg);

    ptr->tmp = (LPSTR) mir_alloc(dwEncMsgLen+1);
    memcpy(ptr->tmp, szEncMsg, dwEncMsgLen+1);
    LocalFree((LPVOID)szEncMsg);

    return ptr->tmp;
}


LPSTR __cdecl pgp_decrypt(pCNTX ptr, LPCSTR szEncMsg)
{
  	ptr->error = ERROR_NONE;
	SAFE_FREE(ptr->tmp);

    LPSTR szPlainMsg = p_pgp_decrypt_keydb(szEncMsg);
	if(!szPlainMsg) {
	    ptr = get_context_on_id(-1); // find private pgp keys
    	if(ptr) {
			pPGPDATA p = (pPGPDATA) ptr->pdata;
    		if(p->pgpKey)
			szPlainMsg = p_pgp_decrypt_key(szEncMsg,(LPCSTR)p->pgpKey);
		}
		if(!szPlainMsg) return NULL;
    }

    DWORD dwPlainMsgLen = strlen(szPlainMsg);

    ptr->tmp = (LPSTR) mir_alloc(dwPlainMsgLen+1);
    memcpy(ptr->tmp, szPlainMsg, dwPlainMsgLen+1);
    LocalFree((LPVOID)szPlainMsg);

    return ptr->tmp;
}


LPSTR __cdecl pgp_encodeA(int context, LPCSTR szPlainMsg)
{
	pCNTX ptr = get_context_on_id(context);
	if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pPGPDATA p = (pPGPDATA) ptr->pdata;
	if(!p->pgpKeyID && !p->pgpKey) { ptr->error = ERROR_NO_PGP_KEY; return NULL; }

	// ansi message: convert to unicode->utf-8 and encrypt.
	int slen = strlen(szPlainMsg)+1;
	LPWSTR wstring = (LPWSTR) alloca(slen*sizeof(WCHAR));
	MultiByteToWideChar(CP_ACP, 0, szPlainMsg, -1, wstring, slen*sizeof(WCHAR));
	LPSTR szUtfMsg = utf8encode(wstring);
	// encrypt
	LPSTR szNewMsg = pgp_encrypt(ptr, szUtfMsg);
	mir_free(szUtfMsg);

	return szNewMsg;
}


LPSTR __cdecl pgp_encodeW(int context, LPCWSTR szPlainMsg)
{
    pCNTX ptr = get_context_on_id(context);
    if(!ptr) return NULL;
	cpp_alloc_pdata(ptr); pPGPDATA p = (pPGPDATA) ptr->pdata;
    if(!p->pgpKeyID && !p->pgpKey) { ptr->error = ERROR_NO_PGP_KEY; return NULL; }

	// unicode message: convert to utf-8 and encrypt.
	LPSTR szUtfMsg = utf8encode(szPlainMsg);
	LPSTR szNewMsg = pgp_encrypt(ptr, szUtfMsg);
	mir_free(szUtfMsg);

	return szNewMsg;
}


LPSTR __cdecl pgp_decode(int context, LPCSTR szEncMsg)
{
    pCNTX ptr = get_context_on_id(context);
    if(!ptr) return NULL;

	LPSTR szNewMsg = NULL;
	LPSTR szOldMsg = pgp_decrypt(ptr, szEncMsg);

	if(szOldMsg) {
		// utf8 message: convert to unicode -> ansii
		WCHAR *wstring = utf8decode(szOldMsg);
		int wlen = wcslen(wstring)+1;
		szNewMsg = (LPSTR) mir_alloc(wlen*(sizeof(WCHAR)+1));
		WideCharToMultiByte(CP_ACP, 0, wstring, -1, szNewMsg, wlen, 0, 0);
		memcpy(szNewMsg+wlen, wstring, wlen*sizeof(WCHAR));
		mir_free(wstring);
	}
	SAFE_FREE(ptr->tmp);
	ptr->tmp = szNewMsg;
	return szNewMsg;
}


int __cdecl pgp_set_key(int context, LPCSTR RemoteKey)
{
    pCNTX ptr = get_context_on_id(context);
    if(!ptr) return 0;
	cpp_alloc_pdata(ptr); pPGPDATA p = (pPGPDATA) ptr->pdata;
   	ptr->error = ERROR_NONE;

//   	if(!p_pgp_check_key(RemoteKey)) return 0;

   	SAFE_FREE(p->pgpKey);
	p->pgpKey = (BYTE *) mir_alloc(strlen(RemoteKey)+1);
	strcpy((LPSTR)p->pgpKey,RemoteKey);

   	return 1;
}


int __cdecl pgp_set_keyid(int context, PVOID RemoteKeyID)
{
    pCNTX ptr = get_context_on_id(context);
    if(!ptr) return 0;
	cpp_alloc_pdata(ptr); pPGPDATA p = (pPGPDATA) ptr->pdata;
   	ptr->error = ERROR_NONE;

   	SAFE_FREE(p->pgpKeyID);
	p->pgpKeyID = (BYTE *) mir_alloc(p_pgp_size_keyid());
	memcpy(p->pgpKeyID,RemoteKeyID,p_pgp_size_keyid());

   	return 1;
}


int __cdecl pgp_size_keyid()
{
	return p_pgp_size_keyid();
}


PVOID __cdecl pgp_select_keyid(HWND hDlg,LPSTR szKeyID)
{
	return p_pgp_select_keyid(hDlg,szKeyID);
}

