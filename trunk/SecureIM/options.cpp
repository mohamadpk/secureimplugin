#include "commonheaders.h"

#define PSKSIZE 4096
BOOL bChangeSortOrder = false;


void TC_InsertItem(HWND hwnd, WPARAM wparam, TCITEM *tci) {
	if( bCoreUnicode ) {
		LPWSTR tmp = a2u(tci->pszText);
		tci->pszText = (LPSTR)TranslateW(tmp);
		SNDMSG(hwnd, TCM_INSERTITEMW, wparam, (LPARAM)tci);
		mir_free(tmp);
	}
	else {
		tci->pszText = Translate(tci->pszText);
		SNDMSG(hwnd, TCM_INSERTITEMA, wparam, (LPARAM)tci);
	}
}


void LV_InsertColumn(HWND hwnd, WPARAM wparam, LVCOLUMN *lvc) {
	if( bCoreUnicode ) {
		LPWSTR tmp = a2u(lvc->pszText);
		lvc->pszText = (LPSTR)TranslateW(tmp);
		SNDMSG(hwnd, LVM_INSERTCOLUMNW, wparam, (LPARAM)lvc);
		mir_free(tmp);
	}
	else {
		lvc->pszText = Translate(lvc->pszText);
		SNDMSG(hwnd, LVM_INSERTCOLUMNA, wparam, (LPARAM)lvc);
	}
}


int LV_InsertItem(HWND hwnd, LVITEM *lvi) {
	return SNDMSG(hwnd, bCoreUnicode ? LVM_INSERTITEMW : LVM_INSERTITEMA, 0, (LPARAM)lvi);
}


int LV_InsertItemA(HWND hwnd, LVITEM *lvi) {
	if( bCoreUnicode ) lvi->pszText = (LPSTR) a2u(lvi->pszText);
	int ret = LV_InsertItem(hwnd, lvi);
	if( bCoreUnicode ) mir_free(lvi->pszText);
	return ret;
}


void LV_SetItemText(HWND hwnd, WPARAM wparam, int subitem, LPSTR text) {
	LV_ITEM lvi = {0};
	lvi.iSubItem = subitem;
	lvi.pszText = text;
	SNDMSG(hwnd, bCoreUnicode ? LVM_SETITEMTEXTW : LVM_SETITEMTEXTA, wparam, (LPARAM)&lvi);
}


void LV_SetItemTextA(HWND hwnd, WPARAM wparam, int subitem, LPSTR text) {
	if( bCoreUnicode ) text = (LPSTR) a2u(text);
	LV_SetItemText(hwnd, wparam, subitem, text);
	if( bCoreUnicode ) mir_free(text);
}


void LV_GetItemTextA(HWND hwnd, WPARAM wparam, int iSubItem, LPSTR text, int cchTextMax) {
	LV_ITEM lvi = {0};
	lvi.iSubItem = iSubItem;
	lvi.cchTextMax = cchTextMax;
	lvi.pszText = text;
	SNDMSG(hwnd, bCoreUnicode ? LVM_GETITEMTEXTW : LVM_GETITEMTEXTA, wparam, (LPARAM)&lvi);
	if( bCoreUnicode ) {
		lvi.pszText = u2a((LPWSTR)text);
		strcpy(text, lvi.pszText);
		mir_free(lvi.pszText);
	}
}

/*
 * tabbed options dialog
 */

BOOL CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

   static int iInit = TRUE;
   TCITEM tci;

   switch(msg) {
      case WM_INITDIALOG: {
         RECT rcClient;
         GetClientRect(hwnd, &rcClient);

         iInit = TRUE;
         tci.mask = TCIF_PARAM|TCIF_TEXT;

         tci.lParam = (LPARAM)CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_TAB_GENERAL),hwnd,DlgProcOptionsGeneral);
		 tci.pszText = (LPSTR)sim201;
         TC_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,5,26,rcClient.right-8,rcClient.bottom-29,1);

         tci.lParam = (LPARAM)CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_TAB_PROTO),hwnd,DlgProcOptionsProto);
		 tci.pszText = (LPSTR)sim202;
         TC_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 2, &tci);
         MoveWindow((HWND)tci.lParam,5,26,rcClient.right-8,rcClient.bottom-29,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);

         if(bPGP && bPGPloaded) {
         tci.lParam = (LPARAM)CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_TAB_PGP),hwnd,DlgProcOptionsPGP);
		 tci.pszText = (LPSTR)sim214;
         TC_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 3, &tci);
         MoveWindow((HWND)tci.lParam,5,26,rcClient.right-8,rcClient.bottom-29,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
         }

         if(bGPG && bGPGloaded) {
         tci.lParam = (LPARAM)CreateDialog(g_hInst,MAKEINTRESOURCE(IDD_TAB_GPG),hwnd,DlgProcOptionsGPG);
		 tci.pszText = (LPSTR)sim226;
         TC_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 4, &tci);
         MoveWindow((HWND)tci.lParam,5,26,rcClient.right-8,rcClient.bottom-29,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
         }

         // add more tabs here if needed
         // activate the final tab
         iInit = FALSE;
		 return TRUE;
      }
	  break;

      case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
         if(!iInit)
			SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
         break;

	  case WM_COMMAND: {
		  switch(LOWORD(wParam)) {
		  	case ID_UPDATE_CLIST: {
                tci.mask = TCIF_PARAM;
                TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),0,&tci);
                SendMessage((HWND)tci.lParam,WM_COMMAND,ID_UPDATE_CLIST,0);
			}
			break;
		  }	
	  }
	  break;
	  
	  case WM_NOTIFY: {
         switch(((LPNMHDR)lParam)->idFrom) {
            case 0: {
               switch (((LPNMHDR)lParam)->code) {
                  case PSN_APPLY: {
                        tci.mask = TCIF_PARAM;
                        int count = TabCtrl_GetItemCount(GetDlgItem(hwnd,IDC_OPTIONSTAB));
                        for (int i=0;i<count;i++) {
                           TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),i,&tci);
                           SendMessage((HWND)tci.lParam,WM_NOTIFY,0,lParam);
                        }
                  }
                  break;
               }
		    } // case 0
            break;

			case IDC_OPTIONSTAB: {
               switch (((LPNMHDR)lParam)->code) {
                  case TCN_SELCHANGING: {
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);
                  }
                  break;
                  case TCN_SELCHANGE: {
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_SHOW);
                  }
                  break;
               }
			} // case IDC_OPTIONSTAB
            break;
         }
	  } // case WM_NOTIFY
      break;
   }
   return FALSE;
}


BOOL CALLBACK DlgProcOptionsGeneral(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    static int iInit = TRUE;
	static HIMAGELIST hLarge, hSmall;
	int i;

	HWND hLV = GetDlgItem(hDlg,IDC_STD_USERLIST);

	switch (wMsg) {
		case WM_INITDIALOG: {

		  TranslateDialogDefault(hDlg);

  		  iInit = TRUE;
//		  SendMessage(hLV, WM_SETREDRAW, FALSE, 0);
		  ListView_SetExtendedListViewStyle(hLV, ListView_GetExtendedListViewStyle(hLV) | LVS_EX_FULLROWSELECT);

		  hLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), iBmpDepth, 1, 1);
		  hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), iBmpDepth, 1, 1);
//		  COLORREF rgbTransparentColor;
		  for (i = 0; i < 3; i++) {
//			  ImageList_AddMasked(himgl, hbmp, rgbTransparentColor);
			  ImageList_AddIcon(hSmall, g_hICO[i]);
			  ImageList_AddIcon(hLarge, g_hICO[i]);
		  }

		  ListView_SetImageList(hLV, hSmall, LVSIL_SMALL);
		  ListView_SetImageList(hLV, hLarge, LVSIL_NORMAL);

		  static const char *szColHdr[] = { sim203, sim204, sim205, sim206 };
		  static int iColWidth[] = { 150, 160, 65, 35 };
		  LVCOLUMN lvc;
		  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		  lvc.fmt = LVCFMT_LEFT;
		  for (i = 0; i < 4; i++) {
			  lvc.iSubItem = i;
			  lvc.pszText = (LPSTR)szColHdr[i];
			  lvc.cx = iColWidth[i];
			  LV_InsertColumn(hLV, i, &lvc);
		  }

		  RefreshGeneralDlg(hDlg);
		  EnableWindow(hLV, true);
//		  SendMessage(hLV, WM_SETREDRAW, TRUE, 0);

  		  iInit = FALSE;
	      return TRUE;
		} // WM_INITDIALOG
		break;

	 	case WM_DESTROY: {
			ImageList_Destroy(hSmall);
			ImageList_Destroy(hLarge);
		} // WM_DESTROY
		break;

	 	case WM_PAINT: {
			if(!iInit)
	 			InvalidateRect(hDlg,NULL,FALSE);
	 	} // WM_PAINT
		break;

		case WM_COMMAND: {
		  switch(LOWORD(wParam)) {
		  	case ID_ALWAYS:
		  	case ID_ENABLED:
		  	case ID_DISABLED: {
		  		int idx = ListView_GetSelectionMark(hLV);
				if (getListViewContact(hLV,idx))
					setListViewStatus(hLV,idx,(LOWORD(wParam) == ID_ALWAYS)?2:((LOWORD(wParam) == ID_DISABLED)?0:1));
			}
			break;

		  	case ID_SETPSK: {
		  		int idx = ListView_GetSelectionMark(hLV);
		  		HANDLE hContact = getListViewContact(hLV,idx);
				if (hContact) {
					char *buffer = (char*)mir_alloc(PSKSIZE+1);
					getContactName(hContact, buffer);
					int res = DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_PSK),NULL,(DLGPROC)DlgProcSetPSK,(LPARAM)buffer);
					if(res == IDOK) {
						setListViewPSK(hLV,idx,1);
					    DBWriteContactSettingString(hContact,szModuleName,"tPSK",buffer);
					}
					mir_free(buffer);
				}
			}
			break;

		  	case ID_DELPSK: {
		  		int idx = ListView_GetSelectionMark(hLV);
		  		HANDLE hContact = getListViewContact(hLV,idx);
				if (hContact) {
					setListViewPSK(hLV,idx,0);
					DBDeleteContactSetting(hContact, szModuleName, "tPSK");
				}
		  	}
		  	break;

		  	case ID_UPDATE_CLIST: {
				iInit = TRUE;
				RefreshGeneralDlg(hDlg);
				iInit = FALSE;
				return TRUE;
			}
			break;

		  	case IDC_RESET: {
				if(!iInit)
					ResetGeneralDlg(hDlg);
			}
			break;

			case IDC_ADV8:
			case IDC_ADV7:
			case IDC_ADV6:
			case IDC_ADV5:
			case IDC_ADV4:
			case IDC_ADV3:
			case IDC_ADV2:
			case IDC_ADV1:
			case IDC_ADV0:
		  	case IDC_GPG:
		  	case IDC_PGP:
		  	case IDC_NO_PGP:
		  	case IDC_AIP:
		  	case IDC_SOM:
		  	case IDC_SFT:
		  	case IDC_ASI:
		  	case IDC_MCD:
		  	case IDC_KET:
		  	case IDC_SCM:
		  	case IDC_DGP:
		  	case IDC_OKT:
				break;

			default:
				return FALSE;
		  }
		  if(!iInit)
			SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
		} // WM_COMMAND
		break;	

		case WM_NOTIFY: {
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0: {
                    if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
						iInit = TRUE;
						ApplyGeneralSettings(hDlg);
						RefreshContactListIcons();
						iInit = FALSE;
					}
				}
				break;

				case IDC_STD_USERLIST: {
                    switch(((LPNMHDR)lParam)->code) {
                    case NM_DBLCLK: {
						if(LPNMLISTVIEW(lParam)->iSubItem == 2) {
							int s = getListViewStatus(hLV,LPNMLISTVIEW(lParam)->iItem)+1;
							if(s>2) s=0;
							setListViewStatus(hLV,LPNMLISTVIEW(lParam)->iItem,s);
							SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
						}
                    } break;
                    case NM_RCLICK: {
						POINT p;
						GetCursorPos(&p);
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_CLIST));
						CheckMenuItem(hMenu, ID_ENCRYPTION, MF_BYCOMMAND );
						CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu, 0);
						TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hDlg, 0);
						DestroyMenu(hMenu);
					} break;
                    case LVN_COLUMNCLICK: {
                    	bChangeSortOrder = true;
						ListView_Sort(hLV,(LPARAM)(LPNMLISTVIEW(lParam)->iSubItem+0x01));
						bChangeSortOrder = false;
					}
					}
				}
				break;
		  }
		} // WM_NOTIFY
		break;
	}
    return FALSE;
}


BOOL CALLBACK DlgProcOptionsProto(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    static int iInit = TRUE;

    HWND hLV = GetDlgItem(hDlg,IDC_PROTO);

	switch (wMsg) {
		case WM_INITDIALOG: {

		  TranslateDialogDefault(hDlg);

  		  iInit = TRUE;
		  ListView_SetExtendedListViewStyle(hLV, ListView_GetExtendedListViewStyle(hLV) | LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

		  LVCOLUMN lvc;
		  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		  lvc.fmt = LVCFMT_LEFT;
		  lvc.pszText = (LPSTR)sim210;
		  lvc.cx = 150;
		  LV_InsertColumn(hLV, 0, &lvc);

		  RefreshProtoDlg(hDlg);
		  EnableWindow(hLV, true);

  		  iInit = FALSE;
		  return TRUE;
		} // WM_INITDIALOG
		break;

		case WM_PAINT: {
			if(!iInit)
	 			InvalidateRect(hDlg,NULL,FALSE);
	 	} // WM_PAINT
		break;

		case WM_COMMAND: {
			if(!iInit)
				SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
		}
		break;

		case WM_NOTIFY: {
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0: {
                    if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
						iInit = TRUE;
						ApplyProtoSettings(hDlg);
						RefreshProtoDlg(hDlg);
						RefreshContactListIcons();
		                SendMessage(GetParent(hDlg),WM_COMMAND,ID_UPDATE_CLIST,0);
						iInit = FALSE;
					}
				}
				break;

				case IDC_PROTO: {
                    if (((LPNMHDR)lParam)->code == (UINT)NM_CLICK) {
						SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
					}
				}
				break;
		  }
		} // WM_NOTIFY
		break;
	}
	return FALSE;
}


static BOOL bPGP9;

BOOL CALLBACK DlgProcOptionsPGP(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    static int iInit = TRUE;
	static HIMAGELIST hLarge, hSmall;
	int i;

    HWND hLV = GetDlgItem(hDlg,IDC_PGP_USERLIST);

	switch (wMsg) {
		case WM_INITDIALOG: {

		  TranslateDialogDefault(hDlg);
  		  iInit = TRUE;

		  ListView_SetExtendedListViewStyle(hLV, ListView_GetExtendedListViewStyle(hLV) | LVS_EX_FULLROWSELECT);

		  hLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), iBmpDepth, 1, 1);
		  hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), iBmpDepth, 1, 1);
		  for (i = ICO_DISKEY; i < ICO_TRYKEY; i++) {
			  ImageList_AddIcon(hSmall, g_hICO[i]);
			  ImageList_AddIcon(hLarge, g_hICO[i]);
		  }

		  ListView_SetImageList(hLV, hSmall, LVSIL_SMALL);
		  ListView_SetImageList(hLV, hLarge, LVSIL_NORMAL);

		  static const char *szColHdr[] = { sim203, sim204, sim215 };
		  static int iColWidth[] = { 160, 150, 80 };
		  LVCOLUMN lvc;
		  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		  lvc.fmt = LVCFMT_LEFT;
		  for (i = 0; i < 3; i++) {
			  lvc.iSubItem = i;
			  lvc.pszText = (LPSTR)szColHdr[i];
			  lvc.cx = iColWidth[i];
			  LV_InsertColumn(hLV, i, &lvc);
		  }

		  RefreshPGPDlg(hDlg,TRUE);
//		  EnableWindow(hLV, bPGPkeyrings);

  		  iInit = FALSE;
		  return TRUE;
		} // WM_INITDIALOG
		break;

	 	case WM_DESTROY: {
			ImageList_Destroy(hSmall);
			ImageList_Destroy(hLarge);
		} // WM_DESTROY
		break;

		case WM_PAINT: {
			if(!iInit)
	 			InvalidateRect(hDlg,NULL,FALSE);
	 	} // WM_PAINT
		break;

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
		  	case IDC_SET_KEYRINGS: {
	  			char PubRingPath[MAX_PATH] = {0}, SecRingPath[MAX_PATH] = {0};
			    bPGPkeyrings = pgp_open_keyrings(PubRingPath,SecRingPath);
				if(bPGPkeyrings && PubRingPath[0] && SecRingPath[0]) {
					DBWriteContactSettingString(0,szModuleName,"pgpPubRing",PubRingPath);
					DBWriteContactSettingString(0,szModuleName,"pgpSecRing",SecRingPath);
				}
			    SetDlgItemText(hDlg, IDC_KEYRING_STATUS, bPGPkeyrings?Translate(sim216):Translate(sim217));
//				EnableWindow(hLV, bPGPkeyrings);
//			    RefreshPGPDlg(hDlg);
				return FALSE;
		  	}
		  	break;
		  	case IDC_NO_KEYRINGS: {
		  		BOOL bNoKR = (SendMessage(GetDlgItem(hDlg, IDC_NO_KEYRINGS),BM_GETCHECK,0L,0L)==BST_CHECKED);
				EnableWindow(GetDlgItem(hDlg, IDC_SET_KEYRINGS), !bNoKR);
				EnableWindow(GetDlgItem(hDlg, IDC_LOAD_PRIVKEY), bNoKR);
				SetDlgItemText(hDlg, IDC_KEYRING_STATUS, bNoKR?Translate(sim225):((bPGP9)?Translate(sim220):(bPGPkeyrings?Translate(sim216):Translate(sim217))));
		  	}
		  	break;
		  	case IDC_LOAD_PRIVKEY: {
				char KeyPath[MAX_PATH] = {0};
			  	if(ShowSelectKeyDlg(hDlg,KeyPath)){
			  		char *priv = LoadKeys(KeyPath,true);
			  		if(priv) {
				  		DBWriteContactSettingString(0,szModuleName,"tpgpPrivKey",priv);
				  		mir_free(priv);
			  		}
			  		else {
				  		DBDeleteContactSetting(0,szModuleName,"tpgpPrivKey");
			  		}
			  	}
		  	}
		  	break;
			default:
				return FALSE;
		  	}
			if(!iInit)
				SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
		}
		break;

		case WM_NOTIFY: {
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0: {
                    if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
						iInit = TRUE;
						ApplyPGPSettings(hDlg);
						RefreshPGPDlg(hDlg,FALSE);
//						RefreshContactListIcons();
		                SendMessage(GetParent(hDlg),WM_COMMAND,ID_UPDATE_CLIST,0);
						iInit = FALSE;
					}
				}
				break;
				case IDC_PGP_USERLIST: {
                    switch(((LPNMHDR)lParam)->code) {
/*                    case NM_RCLICK: {
						POINT p;
						GetCursorPos(&p);
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_CLIST));
						CheckMenuItem(hMenu, ID_ENCRYPTION, MF_BYCOMMAND );
						CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu, 0);
						TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hDlg, 0);
						DestroyMenu(hMenu);
					} break;*/
                    case LVN_COLUMNCLICK: {
                    	bChangeSortOrder = true;
						ListView_Sort(hLV,(LPARAM)(LPNMLISTVIEW(lParam)->iSubItem+0x11));
						bChangeSortOrder = false;
					}
					}
				}
				break;
		  }
		} // WM_NOTIFY
		break;
	}
	return FALSE;
}


BOOL CALLBACK DlgProcOptionsGPG(HWND hDlg, UINT wMsg, WPARAM wParam, LPARAM lParam) {

    static int iInit = TRUE;
	static HIMAGELIST hLarge, hSmall;
	int i;

    HWND hLV = GetDlgItem(hDlg,IDC_GPG_USERLIST);

	switch (wMsg) {
		case WM_INITDIALOG: {

		  TranslateDialogDefault(hDlg);
  		  iInit = TRUE;

		  ListView_SetExtendedListViewStyle(hLV, ListView_GetExtendedListViewStyle(hLV) | LVS_EX_FULLROWSELECT);

		  hLarge = ImageList_Create(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), iBmpDepth, 1, 1);
		  hSmall = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), iBmpDepth, 1, 1);
		  for (i = ICO_DISKEY; i < ICO_TRYKEY; i++) {
			  ImageList_AddIcon(hSmall, g_hICO[i]);
			  ImageList_AddIcon(hLarge, g_hICO[i]);
		  }

		  ListView_SetImageList(hLV, hSmall, LVSIL_SMALL);
		  ListView_SetImageList(hLV, hLarge, LVSIL_NORMAL);

		  static const char *szColHdr[] = { sim203, sim204, sim215, sim227 };
		  static int iColWidth[] = { 140, 120, 120, 40 };
		  LVCOLUMN lvc;
		  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
		  lvc.fmt = LVCFMT_LEFT;
		  for (i = 0; i < 4; i++) {
			  lvc.iSubItem = i;
			  lvc.pszText = (LPSTR)szColHdr[i];
			  lvc.cx = iColWidth[i];
			  LV_InsertColumn(hLV, i, &lvc);
		  }

		  RefreshGPGDlg(hDlg,TRUE);
//		  EnableWindow(hLV, bPGPkeyrings);

  		  iInit = FALSE;
		  return TRUE;
		} // WM_INITDIALOG
		break;

	 	case WM_DESTROY: {
			ImageList_Destroy(hSmall);
			ImageList_Destroy(hLarge);
		} // WM_DESTROY
		break;

		case WM_PAINT: {
			if(!iInit)
	 			InvalidateRect(hDlg,NULL,FALSE);
	 	} // WM_PAINT
		break;

		case WM_COMMAND: {
			switch(LOWORD(wParam)) {
/*		  	case IDC_LOAD_PRIVKEY: {
				char KeyPath[MAX_PATH] = {0};
			  	if(ShowSelectKeyDlg(hDlg,KeyPath)){
			  		char *priv = LoadKeys(KeyPath,true);
			  		if(priv) {
				  		DBWriteContactSettingString(0,szModuleName,"tpgpPrivKey",priv);
				  		mir_free(priv);
			  		}
			  		else {
				  		DBDeleteContactSetting(0,szModuleName,"tpgpPrivKey");
			  		}
			  	}
		  	}
		  	break;*/
            case IDC_BROWSEEXECUTABLE_BTN: {
				char gpgexe[256];
				char filter[128];
				OPENFILENAME ofn;

    			GetDlgItemText(hDlg, IDC_GPGEXECUTABLE_EDIT, gpgexe, sizeof(gpgexe));
    			
				char *txtexecutablefiles="Executable Files"; /*lang*/
				char *txtselectexecutable="Select GnuPG Executable"; /*lang*/

    			// filter zusammensetzen
    			ZeroMemory(&filter, sizeof(filter));
    			strcpy(filter, Translate(txtexecutablefiles));
    			strcat(filter, " (*.exe)");
    			strcpy(filter+strlen(filter)+1, "*.exe");
    			
    			// OPENFILENAME initialisieren
    			ZeroMemory(&ofn, sizeof(ofn));
    			ofn.lStructSize=sizeof(ofn);
    			ofn.hwndOwner=hDlg;
    			ofn.lpstrFilter=filter;
    			ofn.lpstrFile=gpgexe;
    			ofn.nMaxFile=sizeof(gpgexe);
    			ofn.lpstrTitle=Translate(txtselectexecutable);
    			ofn.Flags=OFN_FILEMUSTEXIST|OFN_LONGNAMES|OFN_HIDEREADONLY;
    			
    			if (GetOpenFileName(&ofn))
    			{
    				SetDlgItemText(hDlg, IDC_GPGEXECUTABLE_EDIT, ofn.lpstrFile);
    			}
    		}
    		break;
			default:
			break;
//				return FALSE;
		  	}
			if(!iInit)
				SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
		}
		break;

		case WM_NOTIFY: {
			switch(((LPNMHDR)lParam)->idFrom) {
				case 0: {
                    if (((LPNMHDR)lParam)->code == (UINT)PSN_APPLY) {
						iInit = TRUE;
						ApplyGPGSettings(hDlg);
//						RefreshGPGDlg(hDlg,FALSE);
		                SendMessage(GetParent(hDlg),WM_COMMAND,ID_UPDATE_CLIST,0);
						iInit = FALSE;
					}
				}
				break;
				case IDC_GPG_USERLIST: {
                    switch(((LPNMHDR)lParam)->code) {
                    case NM_DBLCLK: {
						if(LPNMLISTVIEW(lParam)->iSubItem == 3) {
							char tmp[128];
							LV_GetItemTextA(hLV, LPNMLISTVIEW(lParam)->iItem, LPNMLISTVIEW(lParam)->iSubItem, tmp, sizeof(tmp));
							if( strncmp(tmp, Translate(sim228), sizeof(tmp))==0 ) {
								LV_SetItemTextA(hLV, LPNMLISTVIEW(lParam)->iItem, LPNMLISTVIEW(lParam)->iSubItem, Translate(sim229));
							}
							else {
								LV_SetItemTextA(hLV, LPNMLISTVIEW(lParam)->iItem, LPNMLISTVIEW(lParam)->iSubItem, Translate(sim228));
							}
							SendMessage(GetParent(hDlg), PSM_CHANGED, 0, 0);
						}
                    } break;
/*                    case NM_RCLICK: {
						POINT p;
						GetCursorPos(&p);
						HMENU hMenu = LoadMenu(g_hInst, MAKEINTRESOURCE(IDM_CLIST));
						CheckMenuItem(hMenu, ID_ENCRYPTION, MF_BYCOMMAND );
						CallService(MS_LANGPACK_TRANSLATEMENU, (WPARAM)hMenu, 0);
						TrackPopupMenu(GetSubMenu(hMenu, 0), TPM_LEFTALIGN | TPM_TOPALIGN, p.x, p.y, 0, hDlg, 0);
						DestroyMenu(hMenu);
					} break;*/
                    case LVN_COLUMNCLICK: {
                    	bChangeSortOrder = true;
						ListView_Sort(hLV,(LPARAM)(LPNMLISTVIEW(lParam)->iSubItem+0x21));
						bChangeSortOrder = false;
					}
					}
				}
				break;
		  }
		} // WM_NOTIFY
		break;
	}
	return FALSE;
}


BOOL CALLBACK DlgProcSetPSK(HWND hDlg,UINT uMsg,WPARAM wParam,LPARAM lParam) {
    static char *buffer;
	switch(uMsg) {
	case WM_INITDIALOG: {
		SendDlgItemMessage(hDlg,IDC_EDIT1,EM_LIMITTEXT,PSKSIZE,0);
		SetDlgItemText(hDlg,IDC_EDIT2,(LPCSTR)lParam);
		buffer = (LPSTR)lParam;
        return (TRUE);
	}
	case WM_COMMAND: {
		switch(LOWORD(wParam)) {
		case IDOK: {
			int len = GetDlgItemText(hDlg,IDC_EDIT1,buffer,PSKSIZE+1);
			if(len<8) {
				MessageBox(hDlg,Translate(sim211),Translate(szModuleName),MB_OK|MB_ICONEXCLAMATION);
				return TRUE;
			}
			else {
				EndDialog(hDlg,IDOK);
			}
		}
		break;
		case IDCANCEL: {
			EndDialog(hDlg,IDCANCEL);
		}
		break;
		}
	}
	break;
	default:
		return (FALSE);
	}
	return (TRUE);
}


///////////////
// A P P L Y //
///////////////


void ApplyGeneralSettings(HWND hDlg) {

	char timeout[5];
	int tmp,i;

	// Key Exchange Timeout
	GetDlgItemText(hDlg,IDC_KET,timeout,5);
	tmp = atoi(timeout); if(tmp > 65535) tmp = 65535;
	DBWriteContactSettingWord(0,szModuleName,"ket",tmp);
	itoa(tmp,timeout,10);
	SetDlgItemText(hDlg,IDC_KET,timeout);

	// Offline Key Timeout
	GetDlgItemText(hDlg,IDC_OKT,timeout,5);
	tmp = atoi(timeout); if(tmp > 65535) tmp = 65535;
	DBWriteContactSettingWord(0,szModuleName,"okt",tmp);
	itoa(tmp,timeout,10);
	SetDlgItemText(hDlg,IDC_OKT,timeout);

	bSFT = (SendMessage(GetDlgItem(hDlg, IDC_SFT),BM_GETCHECK,0L,0L)==BST_CHECKED);
	bSOM = (SendMessage(GetDlgItem(hDlg, IDC_SOM),BM_GETCHECK,0L,0L)==BST_CHECKED);
	bASI = (SendMessage(GetDlgItem(hDlg, IDC_ASI),BM_GETCHECK,0L,0L)==BST_CHECKED);
	bMCD = (SendMessage(GetDlgItem(hDlg, IDC_MCD),BM_GETCHECK,0L,0L)==BST_CHECKED);
	bSCM = (SendMessage(GetDlgItem(hDlg, IDC_SCM),BM_GETCHECK,0L,0L)==BST_CHECKED);
	bDGP = (SendMessage(GetDlgItem(hDlg, IDC_DGP),BM_GETCHECK,0L,0L)==BST_CHECKED);
	bAIP = (SendMessage(GetDlgItem(hDlg, IDC_AIP),BM_GETCHECK,0L,0L)==BST_CHECKED);

	// Advanced
	for(i=0;i<ADV_CNT;i++)
	   if(SendMessage(GetDlgItem(hDlg, IDC_ADV0+i),BM_GETCHECK,0L,0L)==BST_CHECKED) {
			bADV = i;
			break;
	   }

	// update extraicon position
	g_IEC[0].ColumnType = EXTRA_ICON_ADV1 + bADV - 1;
	for(i=0;i<IEC_CNT;i++){
		if(i) g_IEC[i].ColumnType = g_IEC[0].ColumnType;
	}

	SetFlags();

	// PGP &| GPG flags
	{
	tmp = 0;
	i = SendMessage(GetDlgItem(hDlg, IDC_PGP),BM_GETCHECK,0L,0L)==BST_CHECKED;
	if(i!=bPGP) {
		bPGP = i; tmp++;
		DBWriteContactSettingByte(0, szModuleName, "pgp", bPGP);
	}
	i = SendMessage(GetDlgItem(hDlg, IDC_GPG),BM_GETCHECK,0L,0L)==BST_CHECKED;
	if(i!=bGPG) {
		bGPG = i; tmp++;
		DBWriteContactSettingByte(0, szModuleName, "gpg", bGPG);
	}
	if(tmp) MessageBox(hDlg, Translate(sim224), Translate(szModuleName), MB_OK|MB_ICONINFORMATION);
	}

	HWND hLV = GetDlgItem(hDlg,IDC_STD_USERLIST);
	i = ListView_GetNextItem(hLV,(UINT)-1,LVNI_ALL);
	while(i!=-1) {
		HANDLE hcontact = getListViewContact(hLV,i);
		int status = getListViewStatus(hLV,i);
		if(status==1)	DBDeleteContactSetting(hcontact, szModuleName, "StatusID");
		else 			DBWriteContactSettingByte(hcontact, szModuleName, "StatusID", status);
		if(getListViewPSK(hLV,i)) {
			char *tmp = DBGetString(hcontact,szModuleName,"tPSK");
		    DBWriteContactSettingString(hcontact, szModuleName, "PSK", tmp);
		    mir_free(tmp);
		}
		else {
			DBDeleteContactSetting(hcontact, szModuleName, "PSK");
		}
		DBDeleteContactSetting(hcontact, szModuleName, "tPSK");
		i = ListView_GetNextItem(hLV,i,LVNI_ALL);
	}
}


void ApplyProtoSettings(HWND hDlg) {

	char szNames[512] = {0};

	HWND hLV = GetDlgItem(hDlg,IDC_PROTO);
	int i = ListView_GetNextItem(hLV,(UINT)-1,LVNI_ALL);
	while(i!=-1) {
		if(ListView_GetCheckState(hLV,i)) {
			char *proto = getListViewProto(hLV,i);
			strcat(szNames,proto);
			strcat(szNames,";");
		}
		i = ListView_GetNextItem(hLV,i,LVNI_ALL);
	}

    DBWriteContactSettingString(0,szModuleName,"protos",szNames);
	loadSupportedProtocols();
}


void ApplyPGPSettings(HWND hDlg) {

	bUseKeyrings = !(SendMessage(GetDlgItem(hDlg, IDC_NO_KEYRINGS),BM_GETCHECK,0L,0L)==BST_CHECKED);
	DBWriteContactSettingByte(0,szModuleName,"ukr",bUseKeyrings);

	char *priv = DBGetString(0,szModuleName,"tpgpPrivKey");
	if(priv) {
   	    bPGPprivkey = true;
	    pgp_set_key(-1,priv);
		DBWriteStringEncode(0,szModuleName,"pgpPrivKey",priv);
		mir_free(priv);
  		DBDeleteContactSetting(0,szModuleName,"tpgpPrivKey");
	}
}


void ApplyGPGSettings(HWND hDlg) {

	char tmp[256];

	BOOL bgpgLogFlag = (SendMessage(GetDlgItem(hDlg, IDC_LOGGINGON_CBOX),BM_GETCHECK,0L,0L)==BST_CHECKED);
	DBWriteContactSettingByte(0,szModuleName,"gpgLogFlag",bgpgLogFlag);

	GetDlgItemText(hDlg, IDC_GPGEXECUTABLE_EDIT, tmp, sizeof(tmp));
	DBWriteContactSettingString(0,szModuleName,"gpgExec",tmp);
	GetDlgItemText(hDlg, IDC_GPGHOME_EDIT, tmp, sizeof(tmp));
	DBWriteContactSettingString(0,szModuleName,"gpgHome",tmp);

	bSavePass = (SendMessage(GetDlgItem(hDlg, IDC_SAVEPASS_CBOX),BM_GETCHECK,0L,0L)==BST_CHECKED);
	DBWriteContactSettingByte(0,szModuleName,"gpgSaveFlag",bSavePass);

	GetDlgItemText(hDlg, IDC_GPGLOGFILE_EDIT, tmp, sizeof(tmp));
	DBWriteContactSettingString(0,szModuleName,"gpgLog",tmp);
	if(bgpgLogFlag)	gpg_set_log(tmp);
	else gpg_set_log(0);

    HWND hLV = GetDlgItem(hDlg,IDC_GPG_USERLIST);
	int i = ListView_GetNextItem(hLV,(UINT)-1,LVNI_ALL);
	while(i!=-1) {
		ListView_GetItemText(hLV, i, 3, tmp, sizeof(tmp));
		if( strncmp(tmp, Translate(sim228), sizeof(tmp))==0 )
			DBWriteContactSettingByte((HANDLE)getListViewParam(hLV,i),szModuleName,"gpgANSI",1);
		else
			DBDeleteContactSetting((HANDLE)getListViewParam(hLV,i),szModuleName,"gpgANSI");

		i = ListView_GetNextItem(hLV,i,LVNI_ALL);
	}
}


///////////////////
// R E F R E S H //
///////////////////


void RefreshGeneralDlg(HWND hDlg) {

	char timeout[5];
	UINT data,i;

	// Key Exchange Timeout
	data = DBGetContactSettingWord(0, szModuleName, "ket", 10);
	itoa(data,timeout,10);
	SetDlgItemText(hDlg,IDC_KET,timeout);

	// Offline Key Timeout
	data = DBGetContactSettingWord(0, szModuleName, "okt", 2);
	itoa(data,timeout,10);
	SetDlgItemText(hDlg,IDC_OKT,timeout);

	GetFlags();

	SendMessage(GetDlgItem(hDlg,IDC_SFT),BM_SETCHECK,(bSFT)?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_SOM),BM_SETCHECK,(bSOM)?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_ASI),BM_SETCHECK,(bASI)?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_MCD),BM_SETCHECK,(bMCD)?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_SCM),BM_SETCHECK,(bSCM)?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_DGP),BM_SETCHECK,(bDGP)?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_AIP),BM_SETCHECK,(bAIP)?BST_CHECKED:BST_UNCHECKED,0L);

/*	// CList_classic
	if(!ServiceExists(MS_CLIST_ADDSUBGROUPMENUITEM)) {
		EnableWindow(GetDlgItem(hDlg,IDC_ASI),FALSE);
		for(i=2;i<ADV_CNT;i++)
			EnableWindow(GetDlgItem(hDlg,IDC_ADV1+i),FALSE);
	}	
*/
	// Advanced
	for(i=0;i<ADV_CNT;i++)
		SendMessage(GetDlgItem(hDlg,IDC_ADV0+i),BM_SETCHECK,(i==bADV)?BST_CHECKED:BST_UNCHECKED,0L);

	// Select {OFF,PGP,GPG}
	SendMessage(GetDlgItem(hDlg,IDC_PGP),BM_SETCHECK,bPGP?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_GPG),BM_SETCHECK,bGPG?BST_CHECKED:BST_UNCHECKED,0L);

	// rebuild list of contacts
	HWND hLV = GetDlgItem(hDlg,IDC_STD_USERLIST);
	ListView_DeleteAllItems(hLV);

	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	char tmp[NAMSIZE];

	while (hContact) {

//		if (isSecureProtocol(hContact) && !isProtoMetaContacts(hContact) && !isChatRoom(hContact)) {
		if (isSecureProtocol(hContact) && !getMetaContact(hContact) && !isChatRoom(hContact)) {
			// add proto for new users ...
			if (!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)szModuleName))
				CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)szModuleName);

			int status = DBGetContactSettingByte(hContact, szModuleName, "StatusID",1);
			DBVARIANT dbv;
			DBGetContactSetting(hContact,szModuleName,"PSK",&dbv);
			int psk=(dbv.type==DBVT_ASCIIZ);
			DBFreeVariant(&dbv);

			lvi.iItem++;
			lvi.iImage = status;
			lvi.lParam = (LPARAM)hContact;

			getContactName(hContact, tmp);
			lvi.pszText = (LPSTR)&tmp;
			int itemNum = LV_InsertItem(hLV, &lvi);

			getContactUin(hContact, tmp);
			LV_SetItemText(hLV, itemNum, 1, tmp);

			setListViewStatus(hLV, itemNum, status);
			setListViewPSK(hLV, itemNum, psk);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	ListView_Sort(hLV,(LPARAM)0);
}


void RefreshProtoDlg(HWND hDlg) {

	HWND hLV = GetDlgItem(hDlg,IDC_PROTO);
	ListView_DeleteAllItems(hLV);

	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT | LVIF_PARAM;

	for(int i=0;i<proto_cnt;i++) {
		lvi.iItem = i+1;
		lvi.pszText = proto[i].uniqueName;
		lvi.lParam = (LPARAM)proto[i].uniqueName;
		int itemNum = LV_InsertItemA(hLV, &lvi);
		ListView_SetCheckState(hLV,itemNum,proto[i].inspecting);
	}
}


void RefreshPGPDlg(HWND hDlg, BOOL bRebuildLV) {

	int ver = pgp_get_version();
	bPGP9 = (ver>=0x03050000);

	EnableWindow(GetDlgItem(hDlg, IDC_SET_KEYRINGS), bUseKeyrings && !bPGP9);
	EnableWindow(GetDlgItem(hDlg, IDC_LOAD_PRIVKEY), !bUseKeyrings);
	SetDlgItemText(hDlg, IDC_PGP_PRIVKEY, bPGPprivkey?Translate(sim222):Translate(sim223));

	if(bPGPloaded && ver) {
		char pgpVerStr[64];
		sprintf(pgpVerStr, Translate(sim218), ver >> 24, (ver >> 16) & 255, (ver >> 8) & 255);
		SetDlgItemText(hDlg, IDC_PGP_SDK, pgpVerStr);
	}
	else {
		SetDlgItemText(hDlg, IDC_PGP_SDK, Translate(sim219));
	}
    SetDlgItemText(hDlg, IDC_KEYRING_STATUS, !bUseKeyrings?Translate(sim225):((bPGP9)?Translate(sim220):(bPGPkeyrings?Translate(sim216):Translate(sim217))));

	// Disable keyrings use
	SendMessage(GetDlgItem(hDlg,IDC_NO_KEYRINGS),BM_SETCHECK,(bUseKeyrings)?BST_UNCHECKED:BST_CHECKED,0L);

	if(!bRebuildLV) return;

	// rebuild list of contacts
	HWND hLV = GetDlgItem(hDlg,IDC_PGP_USERLIST);
	ListView_DeleteAllItems(hLV);

	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	char tmp[NAMSIZE];

	while (hContact) {

//		if (isSecureProtocol(hContact) && !isProtoMetaContacts(hContact) && !isChatRoom(hContact)) {
		if (isSecureProtocol(hContact) && !getMetaContact(hContact) && !isChatRoom(hContact)) {
			// add proto for new users ...
			if (!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)szModuleName))
				CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)szModuleName);

			LPSTR szKeyID = DBGetString(hContact,szModuleName,"pgp_abbr");

			lvi.iItem++;
			lvi.iImage = (szKeyID!=0);
			lvi.lParam = (LPARAM)hContact;

			getContactName(hContact, tmp);
			lvi.pszText = (LPSTR)&tmp;
			int itemNum = LV_InsertItem(hLV, &lvi);

			getContactUin(hContact, tmp);
			LV_SetItemText(hLV, itemNum, 1, tmp);

			LV_SetItemTextA(hLV, itemNum, 2, (szKeyID)?szKeyID:Translate(sim221));
			SAFE_FREE(szKeyID);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	ListView_Sort(hLV,(LPARAM)0x10);
}


void RefreshGPGDlg(HWND hDlg, BOOL bRebuildLV) {

	LPSTR path;

	path = DBGetString(0,szModuleName,"gpgExec");
	if(path) {
		SetDlgItemText(hDlg, IDC_GPGEXECUTABLE_EDIT, path);
		mir_free(path);
	}
	path = DBGetString(0,szModuleName,"gpgHome");
	if(path) {
		SetDlgItemText(hDlg, IDC_GPGHOME_EDIT, path);
		mir_free(path);
	}
	path = DBGetString(0,szModuleName,"gpgLog");
	if(path) {
		SetDlgItemText(hDlg, IDC_GPGLOGFILE_EDIT, path);
		mir_free(path);
	}
	// Log to file checkbox
	SendMessage(GetDlgItem(hDlg,IDC_LOGGINGON_CBOX),BM_SETCHECK,(DBGetContactSettingByte(0, szModuleName, "gpgLogFlag",0))?BST_CHECKED:BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_SAVEPASS_CBOX),BM_SETCHECK,(bSavePass)?BST_CHECKED:BST_UNCHECKED,0L);

	if(!bRebuildLV) return;

	// rebuild list of contacts
	HWND hLV = GetDlgItem(hDlg,IDC_GPG_USERLIST);
	ListView_DeleteAllItems(hLV);

	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	char tmp[NAMSIZE];

	while (hContact) {

//		if (isSecureProtocol(hContact) && !isProtoMetaContacts(hContact) && !isChatRoom(hContact)) {
		if (isSecureProtocol(hContact) && !getMetaContact(hContact) && !isChatRoom(hContact)) {
			// add proto for new users ...
			if (!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)szModuleName))
				CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)szModuleName);

			LPSTR szKeyID = DBGetString(hContact,szModuleName,"gpg");

			lvi.iItem++;
			lvi.iImage = (szKeyID!=0);
			lvi.lParam = (LPARAM)hContact;

			getContactName(hContact, tmp);
			lvi.pszText = (LPSTR)&tmp;
			int itemNum = LV_InsertItem(hLV, &lvi);

			getContactUin(hContact, tmp);
			LV_SetItemText(hLV, itemNum, 1, tmp);

			LV_SetItemTextA(hLV, itemNum, 2, (szKeyID)?szKeyID:Translate(sim221));
			LV_SetItemTextA(hLV, itemNum, 3, (DBGetContactSettingByte(hContact,szModuleName,"gpgANSI",0))?Translate(sim228):Translate(sim229));
			SAFE_FREE(szKeyID);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
	ListView_Sort(hLV,(LPARAM)0x20);
}


///////////////
// R E S E T //
///////////////


void ResetGeneralDlg(HWND hDlg) {

	SetDlgItemText(hDlg,IDC_KET,"10");
	SetDlgItemText(hDlg,IDC_OKT,"2");

	SendMessage(GetDlgItem(hDlg,IDC_SFT),BM_SETCHECK,BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_SOM),BM_SETCHECK,BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_ASI),BM_SETCHECK,BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_MCD),BM_SETCHECK,BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_SCM),BM_SETCHECK,BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_DGP),BM_SETCHECK,BST_UNCHECKED,0L);
	SendMessage(GetDlgItem(hDlg,IDC_AIP),BM_SETCHECK,BST_UNCHECKED,0L);

	for(int i=0;i<ADV_CNT;i++)
		SendMessage(GetDlgItem(hDlg,IDC_ADV1+i),BM_SETCHECK,(i==0)?BST_CHECKED:BST_UNCHECKED,0L);

	// rebuild list of contacts
	HWND hLV = GetDlgItem(hDlg,IDC_STD_USERLIST);
	ListView_DeleteAllItems(hLV);

	LVITEM lvi = {0};
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;

	HANDLE hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST, 0, 0);
	char tmp[NAMSIZE];

	while (hContact) {

//		if (isSecureProtocol(hContact) && !isProtoMetaContacts(hContact) && !isChatRoom(hContact)) {
		if (isSecureProtocol(hContact) && !getMetaContact(hContact) && !isChatRoom(hContact)) {
			// add proto for new users ...
			if (!CallService(MS_PROTO_ISPROTOONCONTACT, (WPARAM)hContact, (LPARAM)szModuleName))
				CallService(MS_PROTO_ADDTOCONTACT, (WPARAM)hContact, (LPARAM)szModuleName);

			lvi.iItem++;
			lvi.iImage = 1;
			lvi.lParam = (LPARAM)hContact;

			getContactName(hContact, tmp);
			lvi.pszText = (LPSTR)&tmp;
			int itemNum = LV_InsertItem(hLV, &lvi);

			getContactUin(hContact, tmp);
			LV_SetItemText(hLV, itemNum, 1, tmp);

			setListViewStatus(hLV, itemNum, 1);
			setListViewPSK(hLV, itemNum, 0);
		}
		hContact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT, (WPARAM)hContact, 0);
	}
}


void ResetProtoDlg(HWND hDlg) {
}


///////////////
// O T H E R //
///////////////


LPARAM getListViewParam(HWND hLV, UINT iItem) {

	LVITEM lvi = {0};
	lvi.iItem = iItem;
	lvi.mask = LVIF_PARAM;
	ListView_GetItem(hLV, &lvi);
	return lvi.lParam;
}


UINT getListViewStatus(HWND hLV, UINT iItem) {

	LVITEM lvi = {0};
	lvi.iItem = iItem;
	lvi.mask = LVIF_IMAGE;
	ListView_GetItem(hLV, &lvi);
	return lvi.iImage;
}


void setListViewStatus(HWND hLV, UINT iItem, UINT iImage) {

	LVITEM lvi = {0};
	lvi.iItem = iItem;
	lvi.iImage = iImage;
	lvi.mask = LVIF_IMAGE;
	ListView_SetItem(hLV, &lvi);
	// change status text
	char szKeyID[128];
	if (iImage==2)		strncpy(szKeyID, Translate(sim207), sizeof(szKeyID));
	else if (iImage==1)	strncpy(szKeyID, Translate(sim208), sizeof(szKeyID));
	else if (iImage==0)	strncpy(szKeyID, Translate(sim209), sizeof(szKeyID));
	LV_SetItemTextA(hLV, iItem, 2, szKeyID);
}


UINT getListViewPSK(HWND hLV, UINT iItem) {

	char szPSK[128];
	LV_GetItemTextA(hLV, iItem, 3, szPSK, sizeof(szPSK));
	return strncmp(szPSK, Translate(sim212), sizeof(szPSK))==0;
}


void setListViewPSK(HWND hLV, UINT iItem, UINT iStatus) {

	// change status text
	char szPSK[128];
	if (iStatus)	strncpy(szPSK, Translate(sim212), sizeof(szPSK));
	else 			strncpy(szPSK, Translate(sim213), sizeof(szPSK));
	LV_SetItemTextA(hLV, iItem, 3, szPSK);
}


int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
    char t1[NAMSIZE], t2[NAMSIZE];
    int s,d,m=1;
	DBVARIANT dbv1,dbv2;

	if(lParamSort&0x100) {
		lParamSort&=0xFF;
		m=-1;
	}

	switch(lParamSort){
	case 0x01:
	case 0x11:
	case 0x21: {
		getContactNameA((HANDLE)lParam1, t1);
		getContactNameA((HANDLE)lParam2, t2);
		return strncmp(t1,t2,NAMSIZE)*m;
	} break;
	case 0x02:
	case 0x12:
	case 0x22: {
		getContactUinA((HANDLE)lParam1, t1);
		getContactUinA((HANDLE)lParam2, t2);
		return strncmp(t1,t2,NAMSIZE)*m;
	} break;
	case 0x03: {
		s = DBGetContactSettingByte((HANDLE)lParam1, szModuleName, "StatusID",1);
		d = DBGetContactSettingByte((HANDLE)lParam2, szModuleName, "StatusID",1);
		return (s-d)*m;
	} break;
	case 0x13: {
		DBGetContactSetting((HANDLE)lParam1,szModuleName,"pgp_abbr",&dbv1);
		DBGetContactSetting((HANDLE)lParam2,szModuleName,"pgp_abbr",&dbv2);
		s=(dbv1.type==DBVT_ASCIIZ);
		d=(dbv2.type==DBVT_ASCIIZ);
		if(s && d) {
			s=strcmp(dbv1.pszVal,dbv2.pszVal);
			d=0;
		}
		DBFreeVariant(&dbv1);
		DBFreeVariant(&dbv2);
		return (s-d)*m;
	} break;
	case 0x23: {
		DBGetContactSetting((HANDLE)lParam1,szModuleName,"gpg",&dbv1);
		DBGetContactSetting((HANDLE)lParam2,szModuleName,"gpg",&dbv2);
		s=(dbv1.type==DBVT_ASCIIZ);
		d=(dbv2.type==DBVT_ASCIIZ);
		if(s && d) {
			s=strcmp(dbv1.pszVal,dbv2.pszVal);
			d=0;
		}
		DBFreeVariant(&dbv1);
		DBFreeVariant(&dbv2);
		return (s-d)*m;
	} break;
	case 0x04: {
		DBGetContactSetting((HANDLE)lParam1,szModuleName,"PSK",&dbv1);
		s=(dbv1.type==DBVT_ASCIIZ);
		DBFreeVariant(&dbv1);
		DBGetContactSetting((HANDLE)lParam2,szModuleName,"PSK",&dbv2);
		d=(dbv2.type==DBVT_ASCIIZ);
		DBFreeVariant(&dbv2);
		return (s-d)*m;
	} break;
	}
	return 0;
}


void ListView_Sort(HWND hLV, LPARAM lParamSort) {
    char t[32];

	// restore sort column
	sprintf(t,"os%02x",(UINT)lParamSort&0xF0);
	if((lParamSort&0x0F)==0) {
		lParamSort=(int)DBGetContactSettingByte(0, szModuleName, t, lParamSort+1);
	}
	DBWriteContactSettingByte(0, szModuleName, t, (BYTE)lParamSort);

	// restore sort order
	sprintf(t,"os%02x",(UINT)lParamSort);
	int m=DBGetContactSettingByte(0, szModuleName, t, 0);
	if(bChangeSortOrder){ m=!m; DBWriteContactSettingByte(0, szModuleName, t, m); }

	ListView_SortItems(hLV,&CompareFunc,lParamSort|(m<<8));
}


BOOL ShowSelectKeyDlg(HWND hParent, LPSTR KeyPath)
{
   OPENFILENAME ofn;
   ZeroMemory(&ofn, sizeof(ofn));
   ofn.lStructSize = sizeof(ofn);
   ofn.hwndOwner = hParent;
   ofn.nMaxFile = MAX_PATH;
   ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

   ofn.lpstrFile = KeyPath;
   ofn.lpstrFilter = "ASC files\0*.asc\0All files (*.*)\0*.*\0";
   ofn.lpstrTitle = "Open Key File";
   if (!GetOpenFileName(&ofn)) return FALSE;

   return TRUE;
}


LPCSTR priv_beg = "-----BEGIN PGP PRIVATE KEY BLOCK-----";
LPCSTR priv_end = "-----END PGP PRIVATE KEY BLOCK-----";
LPCSTR publ_beg = "-----BEGIN PGP PUBLIC KEY BLOCK-----";
LPCSTR publ_end = "-----END PGP PUBLIC KEY BLOCK-----";

LPSTR LoadKeys(LPCSTR file,BOOL priv) {
	FILE *f=fopen(file,"r");
	if(!f) return NULL;

	fseek(f,0,SEEK_END);
	int flen = ftell(f);
	fseek(f,0,SEEK_SET);

	LPCSTR beg,end;
	if(priv) {
		beg = priv_beg;
		end = priv_end;
	}
	else {
		beg = publ_beg;
		end = publ_end;
	}

	LPSTR keys = (LPSTR)mir_alloc(flen+1);
	int i=0; BOOL b=false;
	while(fgets(keys+i,128,f)) {
		if(!b && strncmp(keys+i,beg,strlen(beg))==0) {
			b=true;
		}
		else
		if(b && strncmp(keys+i,end,strlen(end))==0) {
			i+=strlen(keys+i);
			b=false;
		}
		if(b) {
			i+=strlen(keys+i);
		}
	}
	*(keys+i)='\0';
/*	while(flen) {
		int block = (flen>32768)?32768:flen;
		fread(keys+i,block,1,f);
		i+=block;
		flen-=block;
	}*/
	fclose(f);
	return keys;
}


int onRegisterOptions(WPARAM wParam, LPARAM) {
	OPTIONSDIALOGPAGE odp = {0};
	odp.cbSize = sizeof(odp);
	odp.hInstance = g_hInst;
	odp.pszTemplate = MAKEINTRESOURCE(IDD_OPTIONSTAB);
	odp.pszTitle = (char*)szModuleName;
	odp.pszGroup = Translate("Services");
	odp.pfnDlgProc = OptionsDlgProc;
	CallService(MS_OPT_ADDPAGE, wParam, (LPARAM)&odp);
	return 0;
}

// EOF
