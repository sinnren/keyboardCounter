#pragma once

#include <cassert>
#include <string>
#include <vector>
#include <map>
#include <future>
#include <functional>
#include <tuple>
#include "wke.h"
#include "RenderGDI.h"

#define		UI_DATA_CMD_DEFAULT		-1

class CWebView
{
	typedef std::function<jsValue(jsExecState)>  funcjsCallBack;

public:
	CWebView();
	~CWebView();

	//��ʼ��һ�μ���
	static int InitWebView();
	//����note.dll·��  Ĭ����note.dll
	static void SetWkeDllPath(LPCTSTR lpDllPath);
	//�޴�����Ϣѭ��������¿����ֶ�����
	static void DoMessageLoop();

	//��JS������ֵ���������ڴ��������֮ǰ���ã�������Ч
	//��һ��js����
	void BindJSFunction(const char *name, int nArgCount, funcjsCallBack cb = nullptr);
	//��һ����js window��һ�������������ص�
	//��bug������ͬʱ����setter��getter 
	//https://github.com/weolar/miniblink49/issues/537 �����߸���...
	void BindJSValue(const char *valname, funcjsCallBack getter, funcjsCallBack setter);

	//������Ƿ��Ѿ�����/����
	bool IsAvailable();

	//�������������
	bool CreateWebViewWindow(wkeWindowType type, HWND parent, int x, int y, int width, int height);
	//��ȡ��������ڵľ��
	HWND GetWebViewWindowHWND();
	//�����ƶ�����Ļ����
	void MoveToCenter();
	//��ʾ���� Ĭ�ϴ�������ʾ
	void ShowWebViewWindow(bool show);
	//��С������
	void MinimizeWindow();
	//��󻯴���
	void MaximizeWindow();
	//�رմ���
	void CloseWebViewWindow();
	//��������� ��������
	void DestroyWebViewWindow();

	//���ô��ڱ���͸�� Ĭ�Ϲر�
	void SetTransparent(bool b);
	//�����Ƿ�֧����ק�ļ���ҳ�棬Ĭ�Ϲر�
	void SetDragEnable(bool b);
	//���ô��ڱ���
	void SetWindowTitle(const utf8* title);
	void SetWindowTitle(const WCHAR *title);
	//���ô���ͼ��
	void SetWindowIcon(HICON hIcon);
	void SetWindowIcon(int nIconID, HINSTANCE hInstance = NULL);

	//����cookies
	void SetCookieEnabled(bool bEnable);
	//����local storage��ȫ·������c:\mb\LocalStorage\��������ֻ����Ŀ¼��Ĭ���ǵ�ǰĿ¼��
	void SetLocalStorageFullPath(const WCHAR *lpPath);
	//���ñ���cookiesĿ¼·�� path���ļ�Ŀ¼·�����������ļ����������Զ��½������ڵ�Ŀ¼��
	void SetCookieJarPath(const WCHAR* path);


	//����URL
	void LoadURL(const utf8* url);
	void LoadURL(const wchar_t* url);
	//�����ļ�
	void LoadFile(const wchar_t* filename);
	//����HTML
	void LoadHtml(const utf8* html);
	//�����ڴ��е��ļ�ϵͳ
	void LoadMemoryFS(const std::map<std::string, std::string> mapFsBuf, const char *indexpath);
	//��ȡ��frame��URL
	const utf8 *GetURL();

	//����js���� ���ÿ��̴߳�����
	jsValue RunJS(const utf8 *script);
	
	//�򿪵��Թ��� path��c:/miniblink-release/front_end/inspector.html ȫ·�������Ҳ���������
	void ShowDevTools(const char *path);

	//�ڷ�UI�߳���Ҫ��webview�����ǷǷ��ģ�����Ҫ���߳̽��н����ص������Ա���UI�߳��е��������⡣����
	template<class F>
	auto RunInUIThread(const F& f) -> decltype(f())
	{
		assert(m_dwUIThreadID != GetCurrentThreadId());

		//����������ֵ
		std::packaged_task<decltype(f())()> task(f);
		std::future<decltype(f())> ret = task.get_future();

		//��function�ٴη�װ ͳһ������ʽ������մ���
		std::function<void()> func = [&task](){ task(); };

		//���͵�UI�߳���
		SendUIThreadMsg(UI_DATA_CMD_DEFAULT, &func);

		//�ȴ�ִ�в����ؽ��
		return ret.get();
	}


	const utf8* jsValue2String(jsValue val, jsExecState es = NULL);
	int jsValue2Int(jsValue val, jsExecState es = NULL);


protected:

	//�κ�����������ǰ�ᴥ���˻ص� ����true��ʾȡ��
	//LoadMemoryFS����ԭʼ����
	virtual bool OnLoadUrlBeginCallBack(const utf8* url, wkeNetJob job);

	//�����������ʱ�ᴥ���˻ص���ֻ��������wkeNetHookRequest�Żᴥ���ýӿڣ�wkeNetSetData���ᴥ�����ٷ��ĵ�����
	virtual void OnLoadUrlEndCallBack(const utf8 *url, void *job, void* buf, int len){};

	//��ҳ����ʧ��ʱ�����˻ص�
	virtual void OnLoadUrlFailCallback(const utf8* url, wkeNetJob job){};

	//��ҳDocument���ݽṹ�������ʱ�����˻ص�
	//����������ҳ������첽����ģʽ��Document���ݽṹ������� �� ��ҳ������ɣ�ÿ���첽ִ�еļ��ض������ʱ�����ܻᴥ��һ�α��ص���ע���ǿ��ܣ�����һ��������ҳ�������ж��frame��ÿ��frame�������ʱ���ᴥ��һ�α��ص���ʵ���п��Բ���wkeOnLoadingFinish�ӿڣ�wkeOnTitleChanged�ӿڣ�wkeOnURLChanged�ӿڣ��Լ����ҳ����ĳ��Ԫ�أ�����վlogo�ȣ��Ƿ���ڵȷ������ۺ��ж�ҳ������Ƿ���ɡ�
	virtual void OnDocumentReadyCallBack(){};
	virtual void OnDocumentReady2CallBack(wkeWebFrameHandle frameId){};

	//wkeWebView�������������ģʽ�������յ�WM_CLODE��Ϣʱ�����˻ص��ӿڣ��ɷ���false�ܾ��رմ���
	virtual bool OnWindowClosingCallBack(){ return true; };

	//���ڼ���������ʱ�����˻ص� ��ʱm_window�Ѿ������ٲ���ʼ��ΪNULL
	virtual void OnWindowDestroyCallBack(){};

	//��ҳ��js�����ص�
	virtual jsValue OnJsNativeFunctionCallBack(const std::string &funcname, jsExecState es){ return jsUndefined(); };
	//Ĭ�����õ�_excmd()����
	virtual jsValue OnJsExCmdCallBack(jsExecState es){ return jsUndefined(); };

	//��UI�߳� ���յ���Ҫ������û���Ϣ
	virtual void OnUserUIMsg(int nCmd, const void *data){};

private:
	wkeWebView m_window;
	HWND m_windowHwnd;
	DWORD m_dwUIThreadID;
	CRenderGDI *m_pRender;  //��ʱ�ò��� �ȶ����š�����
	std::map<std::string, std::string> m_mapFsBuf;

	void OnUIMsg(int nCmd, const void *data);
	void SendUIThreadMsg(int nCmd, const void *data);
	
	static LRESULT CALLBACK SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);


	std::vector<std::tuple<std::string, int, funcjsCallBack>> m_arrBindFunctions;
	std::vector<std::tuple<std::string, funcjsCallBack, funcjsCallBack>> m_arrBindValues;

};

