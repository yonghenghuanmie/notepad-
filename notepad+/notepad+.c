#include <stdbool.h>
#include <tchar.h>
#include <windows.h>
#define idm_new                         1
#define idm_open                        2
#define idm_save                        3
#define idm_saveas                      4
#define idm_undo                        5
#define idm_cut                         6
#define idm_copy                        7
#define idm_paste                       8
#define idm_selectall                   9
#define idm_find                        10
#define idm_findnext                    11
#define idm_replace                     12
#define idm_font                        13
#define idm_return						14
#define idm_hex							15
#define idm_ansi						16
#define idm_unicode						17
#define ask								0
#define open							1
#define save							2
#define findfail						3
#define nopath							4
char hex,unicode;HWND hwndA,edit,hdialog;HINSTANCE hinstance;WNDPROC procA;
int findstring(char buffer[],char string[])
{
	int i,j,k,start,end=-1;
	k=lstrlen(buffer);
	for(i=lstrlen(string)-k+1;i>=0;i--)
		for(j=0;j<k&&string[i+j]==buffer[j];j++)
			if(j==k-1)
			{
				start=i;end=i+j+1;
			}
			return end;
}
LRESULT CALLBACK editprocA(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	switch(message)
	{
	case WM_KEYUP:
		if(wparam==VK_UP||wparam==VK_DOWN||wparam==VK_LEFT||wparam==VK_RIGHT||
			wparam==VK_PRIOR||wparam==VK_NEXT||wparam==VK_HOME||wparam==VK_END)
			SendMessage(hwndA,WM_USER+1,0,0);
		break;
	case WM_LBUTTONUP:
		SendMessage(hwndA,WM_USER+1,0,0);
		break;
	case WM_CHAR:
		if(hex)
		{
			if((wparam==122||wparam==90||wparam==120||wparam==88||wparam==99||wparam==67||wparam==118||wparam==86)&&
				GetKeyState(VK_CONTROL)>=0)
			{
			if((wparam<48&&wparam!=8&&wparam!=32)||(wparam>57&&wparam<65)||(wparam>70&&wparam<97)||wparam>102)
			{
				MessageBeep(0);
				return 0;
			}
			if(wparam>=97&&wparam<=102)
				wparam-=32;
			}
		}
		SetTimer(hwnd,1,50,0);
		break;
	case WM_TIMER:
		SendMessage(hwndA,WM_USER+1,0,0);
		KillTimer(hwnd,1);
		return 0;
	}
	return CallWindowProc(procA,hwnd,message,wparam,lparam);
}
convert(unsigned char *string,int count,char *buffer)
{
	int i;
	if(hex)
		for(i=0;i<count;i++)
			wsprintf(&buffer[3*i],string[i]>15?"%X ":"0%X ",string[i]);
		else
			for (i=0;i<count/3;i++)
				wsprintf(&buffer[i],"%c",(string[i*3]-(string[i*3]<=57?48:55))*16+string[i*3+1]-(string[i*3+1]<=57?48:55));
			
}

inline void IsKnown(TCHAR* FileName)
{
	bool known=false;
	TCHAR* extension=_tcsrchr(FileName,'.')+1;
	if(_tcscmp(extension,_T("txt"))==0||_tcscmp(extension,_T("TXT"))==0)
		known=true;
	if(hex==known)
		SendMessage(GetActiveWindow(),WM_COMMAND,MAKELONG(idm_hex,0),0);
}

char openorsave(OPENFILENAME *filename,char flag)
{
	int i,length;char *buffer,*string;HANDLE hfile;long bytes;WCHAR header=0xFEFF;
	if(flag&save)
	{
		if(!(flag&nopath))
		{
			filename->Flags=OFN_OVERWRITEPROMPT;
			if(!GetSaveFileName (filename))
				return 2;
		}
		if(INVALID_HANDLE_VALUE==(hfile=CreateFile(filename->lpstrFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,0,0)))
			return 0;
		length=GetWindowTextLength(edit);
		buffer=calloc(length+1,1);
		GetWindowText(edit,buffer,length+1);
		if (hex)
		{
			string=calloc(length/3+1,1);
			for (i=0;i<length/3;i++)
				wsprintf(&string[i],"%c",(buffer[i*3]-(buffer[i*3]<=57?48:55))*16+buffer[i*3+1]-(buffer[i*3+1]<=57?48:55));
			WriteFile(hfile,string,length/=3,&bytes,0);
			free(string);
		}
		else
		{
			if(unicode)
			{
				WriteFile(hfile,&header,2,&bytes,0);
				length=MultiByteToWideChar(CP_ACP,0,buffer,-1,0,0);
				string=calloc(length,2);
				MultiByteToWideChar(CP_ACP,0,buffer,-1,(WCHAR*)string,length);
				free(buffer);
				buffer=string;
				length=length*2-2;
			}
			WriteFile(hfile,buffer,length,&bytes,0);
		}
		if (bytes != length)
		{
			CloseHandle(hfile);
			free(buffer);
			return 0;
		}
	}
	else
	{
		if(!(flag&nopath))
		{
			filename->Flags=OFN_CREATEPROMPT;
			if(!GetOpenFileName(filename))
				return 2;
		}
		if(INVALID_HANDLE_VALUE==(hfile=CreateFile(filename->lpstrFile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0)))
			return 0;
		IsKnown(filename->lpstrFile);
		length=GetFileSize(hfile,0);
		buffer=calloc(length+2,1);
		ReadFile(hfile,buffer,length,&bytes,0);
		if(hex)
		{
			string=calloc(length*3+1,1);
			convert(buffer,length,string);
			SetWindowText(edit,string);
			free(string);
		}
		else
		{
			if(unicode)
			{
				string=calloc(length,1);
				CopyMemory(string,buffer+2,length);
				free(buffer);
				buffer=string;
				length=WideCharToMultiByte(CP_ACP,0,(WCHAR*)buffer,-1,0,0,0,0);
				string=calloc(length,1);
				WideCharToMultiByte(CP_ACP,0,(WCHAR*)buffer,-1,string,length,0,0);
				free(buffer);
				buffer=string;
			}
			SetWindowText(edit,buffer);
		}
	}
	CloseHandle(hfile);
	free(buffer);
	return 1;
}
char printmessage(HWND hwnd,char *text,int i)
{
	int flag[]={MB_ICONQUESTION|MB_YESNOCANCEL,MB_ICONERROR|MB_OK,MB_ICONERROR|MB_OK,MB_ICONINFORMATION|MB_OK};
	char buffer[MAX_PATH],*title[]={"退出","打开","保存","查找"},
		*tip[]={"是否保存-%s?","打开-%s失败!","保存-%s失败!","未找到\"%s\"!"};
	wsprintf(buffer,tip[i],text[0]?text:"无标题");
	return MessageBox(hwnd,buffer,title[i],flag[i]);
}
int findnext(int end,char string[],int replaceall)
{
	int count,offset,length;char *buffer;
	length=GetWindowTextLength(edit);
	buffer=calloc(length+1,1);
	GetWindowText(edit,buffer,length+1);
	count=findstring(string,&buffer[end]);
	if(count==-1)
	{
		if(!end)
			printmessage(hwndA,string,findfail);
		else
			if(!replaceall)
				MessageBox(hwndA,"已到文件尾!","查找",MB_ICONINFORMATION|MB_OK);
			end=0;
	}
	else
	{
		offset=end+count;
		SendMessage(edit,EM_SETSEL,offset-lstrlen(string),offset);
								end+=count;
	}
	return end;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam)
{
	int count,offset,line,length,i=0,j=0;RECT rect;
	static OPENFILENAME filename;static unsigned int stringmsg,end;
	unsigned char buffer[50],*text,*textA,titlewnd[MAX_PATH];static HWND hwndedit,hwndstatic;
	static char flag,rt=1,ffile,lastcode=idm_ansi,path[MAX_PATH],title[MAX_PATH],findtext[MAX_PATH],replace[MAX_PATH];
	static FINDREPLACE find;static CHOOSEFONT font;static HFONT hfont,hfontA,temp;static LOGFONT logfont;HANDLE hfile;
	switch(message)
	{
	case WM_CREATE:
		hwndedit=CreateWindow("edit",0,WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|
			ES_NOHIDESEL,0,0,0,0,hwnd,(HMENU)1,hinstance,0);
		edit=hwndedit;
		SendMessage(hwndedit,EM_LIMITTEXT,0xFFFFFFFF,0);
		procA=(WNDPROC)SetWindowLongPtr(hwndedit,GWLP_WNDPROC,(size_t)editprocA);
		hwndstatic=CreateWindow("static","1行 1列",WS_CHILD|WS_VISIBLE|ES_LEFT,0,0,0,0,hwnd,(HMENU)2,hinstance,0);
		DragAcceptFiles(hwnd,TRUE);
		filename.lStructSize=sizeof(OPENFILENAME);
		filename.hwndOwner=hwnd;
		filename.lpstrFilter="文本文档(*.txt)\0*.txt\0所有文件(*.*)\0*.*\0\0";
		filename.lpstrFile=path;
		filename.nMaxFile=MAX_PATH;
		filename.lpstrFileTitle=title;
		filename.nMaxFileTitle=MAX_PATH;
		filename.lpstrDefExt="txt";
		find.lStructSize=sizeof(FINDREPLACE);
		find.hwndOwner=hwnd;
		find.Flags=FR_HIDEUPDOWN|FR_HIDEMATCHCASE|FR_HIDEWHOLEWORD;
		find.lpstrFindWhat=findtext;
		find.lpstrReplaceWith=replace;
		find.wFindWhatLen=MAX_PATH;
		find.wReplaceWithLen=MAX_PATH;
		font.lStructSize=sizeof(CHOOSEFONT);
		font.hwndOwner=hwnd;
		font.lpLogFont=&logfont;
		font.Flags=CF_INITTOLOGFONTSTRUCT|CF_SCREENFONTS|CF_EFFECTS;
		logfont.lfHeight=16;
		logfont.lfCharSet=1;
		lstrcpy(logfont.lfFaceName,"楷体");
		hfont=CreateFontIndirect(&logfont);
		hfontA=hfont;
		SendMessage(hwndedit,WM_SETFONT,(size_t)hfont,0);
		SendMessage(hwndstatic,WM_SETFONT,(size_t)hfont,0);
		stringmsg=RegisterWindowMessage(FINDMSGSTRING);
		return 0;
	case WM_SIZE:
		MoveWindow(hwndedit,0,0,LOWORD(lparam),HIWORD(lparam)-HIWORD(GetDialogBaseUnits()),TRUE);
		MoveWindow(hwndstatic,0,HIWORD(lparam)-HIWORD(GetDialogBaseUnits()),LOWORD(lparam),HIWORD(GetDialogBaseUnits()),TRUE);
		return 0;
	case WM_SETFOCUS:
		SetFocus(hwndedit);
		return 0;
	case WM_INITMENUPOPUP:
		if(lparam==1)
		{
			EnableMenuItem((HMENU)wparam,idm_undo,SendMessage(hwndedit,EM_CANUNDO,0,0)?MF_ENABLED:MF_GRAYED);
			EnableMenuItem((HMENU)wparam,idm_paste,IsClipboardFormatAvailable(CF_TEXT)?MF_ENABLED:MF_GRAYED);
			length=(int)SendMessage(hwndedit,EM_LINELENGTH,0,0);
			EnableMenuItem((HMENU)wparam,idm_selectall,length?MF_ENABLED:MF_GRAYED);
			EnableMenuItem((HMENU)wparam,idm_find,length&&!hdialog?MF_ENABLED:MF_GRAYED);
			EnableMenuItem((HMENU)wparam,idm_findnext,length&&!hdialog?MF_ENABLED:MF_GRAYED);
			EnableMenuItem((HMENU)wparam,idm_replace,length&&!hdialog?MF_ENABLED:MF_GRAYED);
			count=(int)SendMessage(hwndedit,EM_GETSEL,0,0);
			EnableMenuItem((HMENU)wparam,idm_cut,LOWORD(count)!=HIWORD(count)?MF_ENABLED:MF_GRAYED);
			EnableMenuItem((HMENU)wparam,idm_copy,LOWORD(count)!=HIWORD(count)?MF_ENABLED:MF_GRAYED);
		}
		return 0;
	case WM_DROPFILES:
		length=DragQueryFile((HDROP)wparam,0,0,0);
		text=calloc(length+1,1);
		DragQueryFile((HDROP)wparam,0,text,(int)length+1);
 		DragFinish((HDROP)wparam);
		lstrcpy(path,text);
		for(i=length;text[i]!='\\';i--);
		for(j=0;j<length-i;j++)
			title[j]=text[i+j+1];
		if(!openorsave(&filename,open|nopath))
		printmessage(hwnd,title,open);
		free(text);
		return 0;
	case WM_COMMAND:
		if(lparam)
		{
			if(HIWORD(wparam)==EN_UPDATE)
				flag=1;
		}
		else
			switch(LOWORD(wparam))
		{
	case idm_new:
		if(!flag||(flag&&(!title[0]&&!GetWindowTextLength(hwndedit))))
		{
			SetWindowText(hwnd,"无标题 - 记事本");
			SetWindowText(hwndedit,"\0");
			title[0]=0;
			ffile=0;
		}
		else
		{
			if((count=printmessage(hwnd,title,ask))==IDYES)
			{
				if(!(flag=(char)SendMessage(hwnd,WM_COMMAND,idm_save,0)))
				{
					SetWindowText(hwnd,"无标题 - 记事本");
					SetWindowText(hwndedit,"\0");
					title[0]=0;
					ffile=0;
				}
			}
			else if(count==IDNO)
			{
				SetWindowText(hwnd,"无标题 - 记事本");
				SetWindowText(hwndedit,"\0");
				title[0]=0;
				ffile=0;
				flag=0;
			}
		}
		
		return 0;
	case idm_open:
		if(!flag||(flag&&(!title[0]&&!GetWindowTextLength(hwndedit))))
		{
			if(!(count=openorsave(&filename,open)))
			{
				printmessage(hwnd,title,open);
				return 1;
			}
			else if(count==1)
			{
				wsprintf(titlewnd,"%s - 记事本",title);
				SetWindowText(hwnd,titlewnd);
				ffile=1;
			}
			else if(count==2) return 1;
		}
		else
		{
			if((count=printmessage(hwnd,title,ask))==IDYES)
			{
				if(!(flag=(char)SendMessage(hwnd,WM_COMMAND,idm_save,0)))
					PostMessage(hwnd,WM_COMMAND,idm_open,0);
			}
			else if(count==IDNO)
			{
				flag=0;
				flag=(char)SendMessage(hwnd,WM_COMMAND,idm_open,0);
			}
		}
		return 0;
	case idm_save:
		if(path[0])
		{
			if(openorsave(&filename,save|nopath)==1)
			{
				flag=0;
				return 0;
			}
			else
			{
				printmessage(hwnd,title,save);
				return 1;
			}
		}
	case idm_saveas:
		if((count=openorsave(&filename,save))==1)
		{
			wsprintf(titlewnd,"%s - 记事本",title);
			SetWindowText(hwnd,titlewnd);
			flag=0;
			return 0;
		}
		else if(!count)
			printmessage(hwnd,title,save);
		return 1;
	case idm_undo:
		SendMessage(hwndedit,WM_UNDO,0,0);
		break;
	case idm_cut:
		SendMessage(hwndedit,WM_CUT,0,0);
		break;
	case idm_copy:
		SendMessage(hwndedit,WM_COPY,0,0);
		break;
	case idm_paste:
		SendMessage(hwndedit,WM_PASTE,0,0);
		break;
	case idm_selectall:
		SendMessage(edit,EM_SETSEL,0,-1);
		break;
	case idm_find:
		hdialog=FindText(&find);
		return 0;
	case idm_findnext:
		if(!find.lpstrFindWhat[0])
			SendMessage(hwnd,WM_COMMAND,idm_find,0);
		else
			end=findnext(end,find.lpstrFindWhat,0);
		return 0;
	case idm_replace:
		hdialog=ReplaceText(&find);
		return 0;
	case idm_font:
		if(ChooseFont(&font))
		{
			temp=CreateFontIndirect(&logfont);
			SendMessage(hwndedit,WM_SETFONT,(size_t)temp,0);
			if(hfont!=hfontA)
				DeleteObject(hfont);
			hfont=temp;
			GetClientRect(hwndedit,&rect);
			InvalidateRect(hwndedit,&rect,TRUE);
		}
		return 0;
	case idm_return:
		rt=!rt;
		count=GetWindowTextLength(hwndedit);
		text=calloc(count+1,1);
		GetWindowText(hwndedit,text,(int)count+1);
		DestroyWindow(hwndedit);
		hwndedit=CreateWindow("edit",0,
			rt?WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_NOHIDESEL:
		WS_CHILD|WS_VISIBLE|WS_VSCROLL|WS_HSCROLL|WS_BORDER|ES_LEFT|ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL|ES_NOHIDESEL,
			0,0,0,0,hwnd,(HMENU)1,hinstance,0);
		SendMessage(hwndedit,EM_LIMITTEXT,0xFFFFFFFF,0);
		SetWindowText(hwndedit,text);
		free(text);
		edit=hwndedit;
		procA=(WNDPROC)SetWindowLongPtr(hwndedit,GWLP_WNDPROC,(size_t)editprocA);
		SendMessage(hwndedit,WM_SETFONT,(size_t)hfont,0);
		GetClientRect(hwnd,&rect);
		MoveWindow(hwndedit,0,0,rect.right,rect.bottom-HIWORD(GetDialogBaseUnits()),TRUE);
		CheckMenuItem(GetMenu(hwnd),idm_return,rt?MF_CHECKED:MF_UNCHECKED);
		SendMessage(hwnd,WM_USER+1,0,0);
		SetFocus(hwndedit);
		return 0;
	case idm_hex:
		hex=!hex;
		CheckMenuItem(GetMenu(hwnd),idm_hex,hex?MF_CHECKED:MF_UNCHECKED);
		filename.lpstrDefExt=hex?0:"txt";
		filename.lpstrFilter=hex?"所有文件(*.*)\0*.*\0\0":"文本文档(*.txt)\0*.txt\0所有文件(*.*)\0*.*\0\0";
		count=GetWindowTextLength(hwndedit);
		if(count)
		{
			text=calloc(count+1,1);
			GetWindowText(hwndedit,text,count+1);
			if(hex)
			{
				if(unicode)
				{
					count=MultiByteToWideChar(CP_ACP,0,text,-1,0,0);
					textA=calloc(count,2);
					MultiByteToWideChar(CP_ACP,0,text,-1,(WCHAR*)textA,count);
					free(text);
					text=textA;
					textA=calloc(count*6+1,1);
					if(!ffile)
						for(i=0;i<(count-1)*2;i+=2)
						{
							wsprintf(&textA[3*i],text[i+1]>15?"%X ":"0%X ",text[i+1]);
							wsprintf(&textA[3*(i+1)],text[i]>15?"%X ":"0%X ",text[i]);
						}
						else
							for(i=0;i<(count-1)*2;i+=2)
							{
								wsprintf(&textA[3*i],text[i]>15?"%X ":"0%X ",text[i]);
								wsprintf(&textA[3*(i+1)],text[i+1]>15?"%X ":"0%X ",text[i+1]);
							}
				}
				else
				{
					textA=calloc(count*3+1,1);
					convert(text,count,textA);
				}
			}
			else
			{
				textA=calloc(count/3+2,1);
				convert(text,count,textA);
				if(unicode)
				{
					free(text);
					text=textA;
					if(!ffile)
						for(i=0;i<count/3;i+=2)
						{
							j=text[i];
							text[i]=text[i+1];
							text[i+1]=j;
						}
						length=WideCharToMultiByte(CP_ACP,0,(WCHAR*)text,-1,0,0,0,0);
						textA=calloc(length,1);
						WideCharToMultiByte(CP_ACP,0,(WCHAR*)text,-1,textA,length,0,0);
				}
			}
			SetWindowText(hwndedit,textA);
			free(textA);
			free(text);
			SendMessage(hwnd,WM_USER+1,0,0);
		}
		return 0;
	case idm_ansi:
		CheckMenuItem(GetMenu(hwnd),lastcode,MF_UNCHECKED);
		CheckMenuItem(GetMenu(hwnd),idm_ansi,MF_CHECKED);
		lastcode=idm_ansi;
		unicode=0;
		return 0;
	case idm_unicode:
		CheckMenuItem(GetMenu(hwnd),lastcode,MF_UNCHECKED);
		CheckMenuItem(GetMenu(hwnd),idm_unicode,MF_CHECKED);
		lastcode=idm_unicode;
		unicode=1;
		return 0;
}
return 0;
	case WM_USER+1:
		SendMessage(hwndedit,EM_GETSEL,0,(LPARAM)&count);
		offset=(int)SendMessage(hwndedit,EM_LINEINDEX,-1,0);
		for(line=0;SendMessage(hwndedit,EM_LINEINDEX,line,0)<offset;line++);
		if(!hex)
		{
			length=(int)SendMessage(hwndedit,EM_GETSEL,0,0);
			if(LOWORD(length)!=HIWORD(length))
				return 0;
			length=(int)SendMessage(hwndedit,EM_LINELENGTH,-1,0);
			//返回值为当前行未被选中的长度
			text=malloc(length+1);
			text[length]=0;
			SendMessage(hwndedit,EM_GETLINE,line,(LPARAM)text);
			//内存长度为指针到第一个/0的长度
			while(i<count-offset)
			{
				if(*text>0x7F)
				{
					i++;j++;text++;
				}
				i++;text++;
			}
			text-=i;
			free(text);
			wsprintf(buffer,"%d行 %d列",line+1,count-offset+1-j);
		}
		else
		{
			length=(int)SendMessage(hwndedit,EM_LINELENGTH,0,0);
			i=(length/3)*line+(count-offset)/3;
			wsprintf(buffer,"%X",i);
			j=lstrlen(buffer);
			wsprintf(buffer,j%2?"%d字节 0x0%X":"%d字节 0x%X",i,i);
		}
		SetWindowText(hwndstatic,buffer);
		return 0;
	case WM_USER+2:
		ffile=1;
		length=lstrlen(((char*)wparam))-1;
		text=calloc(length,1);
		for(i=0;i<length-1;i++)
			text[i]=((char*)wparam)[i+1];
		text[length-1]=0;
		lstrcpy(path,text);
		for(i=length;text[i]!='\\';i--);
		for(j=0;j<length-i;j++)
			title[j]=text[i+j+1];
		free(text);
		length=lstrlen(title);
		if(findstring(&title[length-4],".txt")!=-1||findstring(&title[length-4],".TXT")!=-1)
		{
			if(INVALID_HANDLE_VALUE==(hfile=CreateFile(filename.lpstrFile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0)))
				printmessage(hwnd,title,open);
			count=0;
			ReadFile(hfile,&count,2,&offset,0);
			if((WCHAR)count==0xFEFF)
				SendMessage(hwnd,WM_COMMAND,idm_unicode,0);
			else
				SendMessage(hwnd,WM_COMMAND,idm_ansi,0);
			CloseHandle(hfile);
		}
		else
			SendMessage(hwnd,WM_COMMAND,idm_hex,0);
		openorsave(&filename,open|nopath);
		wsprintf(titlewnd,"%s - 记事本",title);
		SetWindowText(hwnd,titlewnd);
		SendMessage(hwnd,WM_USER+1,0,0);
		return 0;
	case WM_CTLCOLOREDIT:
		SetTextColor((HDC)wparam,font.rgbColors);
		return 0;
	case WM_CLOSE:
		if(title[0]?flag:flag&&GetWindowTextLength(hwndedit))
		{
			if(IDYES==(count=printmessage(hwnd,title,ask)))
			{
				if(!SendMessage(hwnd,WM_COMMAND,idm_save,0))
					break;
				else return 0;
			}
			else if(IDNO==count) break;
			else return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
			}
			if(message==stringmsg)
			{
				if(find.Flags&FR_DIALOGTERM) hdialog=0;
				if(find.Flags&FR_FINDNEXT)
					end=findnext(end,find.lpstrFindWhat,0);
				if(find.Flags&FR_REPLACE)
				{
					count=(int)SendMessage(hwndedit,EM_GETSEL,0,0);
					if(LOWORD(count)==HIWORD(count))
						end=findnext(end,find.lpstrFindWhat,0);
					else
					{
						SendMessage(edit,EM_REPLACESEL,0,(size_t)find.lpstrReplaceWith);
						end+=lstrlen(find.lpstrReplaceWith)-lstrlen(find.lpstrFindWhat);
					}
				}
				if(find.Flags&FR_REPLACEALL)
				{
					end=0;
					while(end=findnext(end,find.lpstrFindWhat,1))
					{
						end+=lstrlen(find.lpstrReplaceWith)-lstrlen(find.lpstrFindWhat);
						SendMessage(edit,EM_REPLACESEL,0,(size_t)find.lpstrReplaceWith);
					}
				}
				return 0;
			}
			return DefWindowProc(hwnd,message,wparam,lparam);
}
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	HWND hwnd;WNDCLASS wndclass;MSG msg;HACCEL haccel;int x,y;
	memset(&wndclass,0,sizeof(WNDCLASS));
	wndclass.hbrBackground=GetStockObject(WHITE_BRUSH);
	wndclass.hCursor=LoadCursor(0,IDC_ARROW);
	wndclass.hIcon=LoadIcon(hInstance,"icon");
	wndclass.hInstance=hInstance;
	wndclass.lpfnWndProc=WndProc;
	wndclass.lpszClassName="notepad+";
	wndclass.lpszMenuName="menu";
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	RegisterClass(&wndclass);
	x=GetSystemMetrics(SM_CXSCREEN);
	y=GetSystemMetrics(SM_CYSCREEN);
	hwnd=CreateWindow("notepad+","无标题 - 记事本",WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,5*x/8,3*y/4,0,0,hInstance,0);
	//SetWindowText(hwnd,lpCmdLine);
	hinstance=hInstance;
	hwndA=hwnd;
	ShowWindow(hwnd,nCmdShow);
	haccel=LoadAccelerators(hInstance,"accelerator");
	if(lpCmdLine[0]&&strchr(lpCmdLine,'\"'))
		PostMessage(hwnd,WM_USER+2,(size_t)strchr(lpCmdLine,'\"'),0);
	while(GetMessage(&msg,0,0,0))
	{
		if(!TranslateAccelerator(hwnd,haccel,&msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;
} 