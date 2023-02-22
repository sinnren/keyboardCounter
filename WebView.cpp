#include "WebView.h"
#include <windows.h>
#include <commctrl.h>

#define		FAKE_WEBVIEW_DOMAIN		"http://hook_fake_webview_domain/"
#define		MSG_UI_DATA				WM_USER + 76452

#ifdef _WIN64
std::wstring g_strWkeDllPath = L"miniblink_x64.dll";
#else
std::wstring g_strWkeDllPath = L"node.dll";
#endif // _WIN64

CWebView::CWebView()
{
	m_window = NULL;
	m_windowHwnd = NULL;
	m_pRender = NULL;
	m_dwUIThreadID = 0;
}


CWebView::~CWebView()
{
	if (m_window)
		DestroyWebViewWindow();

	if (m_pRender)
		delete m_pRender;
}

int CWebView::InitWebView()
{
	return wkeInitialize();
}

void CWebView::SetWkeDllPath(LPCTSTR lpDllPath)
{
	g_strWkeDllPath = lpDllPath;
	wkeSetWkeDllPath(g_strWkeDllPath.c_str());
}


void CWebView::DoMessageLoop()
{
	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void CWebView::BindJSFunction(const char *name, int nArgCount, funcjsCallBack cb/* = nullptr*/)
{
	if (cb)
		m_arrBindFunctions.emplace_back(std::make_tuple(name, nArgCount, std::move(cb)));
	else
		m_arrBindFunctions.emplace_back(std::make_tuple(name, nArgCount, std::bind(&CWebView::OnJsNativeFunctionCallBack, this, name, std::placeholders::_1)));
}

void CWebView::BindJSValue(const char *valname, funcjsCallBack getter, funcjsCallBack setter)
{
	m_arrBindValues.emplace_back(std::make_tuple(valname, std::move(getter), std::move(setter)));
}

bool CWebView::IsAvailable()
{
	return (m_window != NULL);
}

bool CWebView::CreateWebViewWindow(wkeWindowType type, HWND parent, int x, int y, int width, int height)
{
	printf("webview threadID: %d\n", GetCurrentThreadId());

	//内置的js方法_excmd回调
	wkeJsBindFunction("_excmd", [](jsExecState es, void* param)->jsValue{ 
		CWebView *pWebview = (CWebView*)param; 
		return pWebview->OnJsExCmdCallBack(es); 
	}, this, 10);

	//用户设置的js方法回调
	for (size_t i = 0; i < m_arrBindFunctions.size(); i++)
	{
		wkeJsBindFunction(std::get<0>(m_arrBindFunctions[i]).c_str(), [](jsExecState es, void* param)->jsValue{
			std::tuple<std::string, int, funcjsCallBack> *cb = (std::tuple<std::string, int, funcjsCallBack> *)param;
			return std::get<2>(*cb)(es);
		}, &m_arrBindFunctions[i], std::get<1>(m_arrBindFunctions[i]));
	}

	//用户设置的对js window绑定一个属性设置器回调
	for (size_t i = 0; i < m_arrBindValues.size(); i++)
	{
		std::tuple<std::string, funcjsCallBack, funcjsCallBack> *cbData = &m_arrBindValues[i];

		//setter
		if (std::get<2>(m_arrBindValues[i]))
		{
			wkeJsBindSetter(std::get<0>(m_arrBindValues[i]).c_str(), [](jsExecState es, void* param)->jsValue{
				std::tuple<std::string, funcjsCallBack, funcjsCallBack> *cb = (std::tuple<std::string, funcjsCallBack, funcjsCallBack> *)param;
				return std::get<2>(*cb)(es);
			}, cbData);
		}

		//getter
		if (std::get<1>(m_arrBindValues[i]))
		{
			wkeJsBindGetter(std::get<0>(m_arrBindValues[i]).c_str(), [](jsExecState es, void* param)->jsValue{
				std::tuple<std::string, funcjsCallBack, funcjsCallBack> *cb = (std::tuple<std::string, funcjsCallBack, funcjsCallBack> *)param;
				return std::get<1>(*cb)(es);
			}, cbData);
		}
	}
	
	//创建webview窗口
	m_window = wkeCreateWebWindow(type, parent, x, y, width, height);
	if (!m_window)
		return false;

	m_windowHwnd = wkeGetWindowHandle(m_window);
	if (!m_windowHwnd)
		return false;

	m_dwUIThreadID = GetCurrentThreadId();

	//如果是无边框模式的，要增加以下系统窗体属性，否则在任务栏点击不会自动最小化。有需要再增加其他属性
	if (type == WKE_WINDOW_TYPE_TRANSPARENT)
	{
		//默认情况下 只有三个属性 WS_POPUP WS_CLIPSIBLNGS WS_CLIPCHILDREN   调用ShowWindow之后才会有WS_VISIBLE属性
		LONG lStyle = GetWindowLong(m_windowHwnd, GWL_STYLE);
		lStyle = lStyle | WS_SYSMENU | WS_MINIMIZEBOX;
		SetWindowLong(m_windowHwnd, GWL_STYLE, lStyle);
	}

	//窗口消息回调
	SetWindowSubclass(m_windowHwnd, &CWebView::SubClassProc, 0, (DWORD_PTR)this);

	//默认关闭透明效果。。。
	SetTransparent(false);
	//默认关闭拖拽
	SetDragEnable(false);

	//初始化一堆回调
	wkeOnLoadUrlBegin(m_window, [](wkeWebView webView, void* param, const utf8* url, wkeNetJob job)->bool{ CWebView *pWebview = (CWebView*)param; return pWebview->OnLoadUrlBeginCallBack(url, job); }, this);
	wkeOnLoadUrlEnd(m_window, [](wkeWebView webView, void* param, const utf8 *url, void *job, void* buf, int len){CWebView *pWebview = (CWebView*)param; return pWebview->OnLoadUrlEndCallBack(url, job, buf, len); }, this);
	wkeOnLoadUrlFail(m_window, [](wkeWebView webView, void* param, const utf8* url, wkeNetJob job){CWebView *pWebview = (CWebView*)param; return pWebview->OnLoadUrlFailCallback(url, job); }, this);
	wkeOnDocumentReady(m_window, [](wkeWebView webView, void* param){CWebView *pWebview = (CWebView*)param; return pWebview->OnDocumentReadyCallBack(); }, this);
	wkeOnDocumentReady2(m_window, [](wkeWebView webView, void* param, wkeWebFrameHandle frameId){CWebView *pWebview = (CWebView*)param; return pWebview->OnDocumentReady2CallBack(frameId); }, this);

	wkeOnWindowClosing(m_window, [](wkeWebView webView, void* param){CWebView *pWebview = (CWebView*)param; return pWebview->OnWindowClosingCallBack(); }, this);
	wkeOnWindowDestroy(m_window, [](wkeWebView webView, void* param){CWebView *pWebview = (CWebView*)param; pWebview->m_window = NULL; pWebview->OnWindowDestroyCallBack(); }, this);

	//控制台输出
	wkeOnConsole(m_window, [](wkeWebView webView,
		void* param,
		wkeConsoleLevel level,
		const wkeString message,
		const wkeString sourceName,
		unsigned sourceLine,
		const wkeString stackTrace){
		
		printf("[%d][%d:%ls]:%ls\n", level, sourceLine, wkeToStringW(sourceName), wkeToStringW(message));

	}, this);
	return true;
}

HWND CWebView::GetWebViewWindowHWND()
{
	return m_windowHwnd;
}

void CWebView::SetTransparent(bool b)
{
	if (!m_window)
		return; 

	wkeSetTransparent(m_window, b);
}

void CWebView::SetDragEnable(bool b)
{
	if (!m_window)
		return;

	wkeSetDragEnable(m_window, b);
}

void CWebView::SetWindowTitle(const utf8* title)
{
	if (!m_window)
		return;

	wkeSetWindowTitle(m_window, title);
}

void CWebView::SetWindowTitle(const WCHAR *title)
{
	if (!m_window)
		return;

	wkeSetWindowTitleW(m_window, title);
}

void CWebView::SetWindowIcon(int nIconID, HINSTANCE hInstance/* = NULL*/)
{
	if (!m_window)
		return;

	HICON hIcon = LoadIcon((hInstance ? hInstance : GetModuleHandle(NULL)), MAKEINTRESOURCE(nIconID));

	SetWindowIcon(hIcon);
}

void CWebView::SetWindowIcon(HICON hIcon)
{
	if (!m_window)
		return;

	::SendMessage(m_windowHwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
	::SendMessage(m_windowHwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
}


void CWebView::MoveToCenter()
{
	if (!m_window)
		return;

	RECT rect;
	GetWindowRect(m_windowHwnd, &rect);
	int width = rect.right - rect.left;
	int height = rect.bottom - rect.top;
	int cx = GetSystemMetrics(SM_CXFULLSCREEN); // 取显示器屏幕高宽
	int cy = GetSystemMetrics(SM_CYFULLSCREEN);
	int x = cx / 2 - width / 2;
	int y = cy / 2 - height / 2;

	MoveWindow(m_windowHwnd, x, y, width, height, false); // 移动窗口位置居中

	//自带的有点靠下。。
	//wkeMoveToCenter(m_window);
}

void CWebView::ShowWebViewWindow(bool show)
{
	if (!m_window)
		return;
	
	if (show)
	{
		::ShowWindow(m_windowHwnd, SW_SHOWNORMAL);
		SetForegroundWindow(m_windowHwnd);
	}
	else
	{
		::ShowWindow(m_windowHwnd, SW_HIDE);
	}
	//wkeShowWindow(m_window, show);
}

void CWebView::MinimizeWindow()
{
	::PostMessage(m_windowHwnd, WM_SYSCOMMAND, SC_MINIMIZE, 0);
}

void CWebView::MaximizeWindow()
{
	if (!IsZoomed(m_windowHwnd))
		::PostMessage(m_windowHwnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
	else
		::PostMessage(m_windowHwnd, WM_SYSCOMMAND, SC_RESTORE, 0);
}

void CWebView::CloseWebViewWindow()
{
	::PostMessage(m_windowHwnd, WM_CLOSE, 0, 0);
}

void CWebView::DestroyWebViewWindow()
{
	if (!m_window)
		return;

	wkeDestroyWebWindow(m_window);
	m_window = NULL;
}

void CWebView::SetCookieEnabled(bool bEnable)
{
	if (!m_window)
		return;

	wkeSetCookieEnabled(m_window, bEnable);
}


void CWebView::SetLocalStorageFullPath(const WCHAR *lpPath)
{
	if (!m_window)
		return;

	wkeSetLocalStorageFullPath(m_window, lpPath);
}

void CWebView::SetCookieJarPath(const WCHAR* path)
{
	if (!m_window)
		return;

	wkeSetCookieJarPath(m_window, path);
}

void CWebView::LoadURL(const utf8* url)
{
	if (!m_window)
		return;

	wkeLoadURL(m_window, url);
}

void CWebView::LoadURL(const wchar_t* url)
{
	if (!m_window)
		return;

	wkeLoadURLW(m_window, url);
}

void CWebView::LoadFile(const wchar_t* filename)
{
	if (!m_window)
		return;

	wkeLoadFileW(m_window, filename);
}

void CWebView::LoadHtml(const utf8* html)
{
	if (!m_window)
		return;

	wkeLoadHTML(m_window, html);
}

const utf8 *CWebView::GetURL()
{
	if (!m_window)
		return NULL;

	return wkeGetURL(m_window);
}

void CWebView::LoadMemoryFS(const std::map<std::string, std::string> mapFsBuf, const char *indexpath)
{
	if (!m_window)
		return;

	m_mapFsBuf = std::move(mapFsBuf);

	std::string fakeurl(FAKE_WEBVIEW_DOMAIN);
	if (indexpath == NULL)
		fakeurl += "index.html";
	else
		fakeurl += indexpath;

	wkeLoadURL(m_window, fakeurl.c_str());
}


jsValue CWebView::RunJS(const utf8 *script)
{
	if (!m_window)
		return jsUndefined();

	if (m_dwUIThreadID == GetCurrentThreadId())
	{
		return wkeRunJS(m_window, script);
	}
	else
	{
		/*
		//扫地僧提醒：跨线程返回的jsValue有可能会失效
		//如果需要跨线程RunJS并且需要返回值不要直接用此方法的返回值
		//可以使用如下方式：
			std::string ret = RunInUIThread([this](){
				//此代码在UI线程中执行
				jsValue jsRet = RunJS("return callfunc();");
				return jsValue2String(jsRet);
			});
		*/
		return RunInUIThread([script, this](){return RunJS(script); });
	}
}


void CWebView::ShowDevTools(const char *path)
{
	if (!m_window)
		return;

	std::string devtoolpath("file:///");
	devtoolpath += path;

	wkeSetDebugConfig(m_window, "showDevTools", devtoolpath.c_str());
}

void CWebView::OnUIMsg(int nCmd, const void *data)
{
	if (nCmd == UI_DATA_CMD_DEFAULT)
	{
		std::function<void()> *func = (std::function<void()> *)data;
		//执行
		(*func)();

		return;
	}

	//其他值是用户消息
	OnUIMsg(nCmd, data);
}


void CWebView::SendUIThreadMsg(int nCmd, const void *data)
{
	::PostMessage(m_windowHwnd, MSG_UI_DATA, nCmd, (LPARAM)data);
}


const utf8* CWebView::jsValue2String(jsValue val, jsExecState es /*= NULL*/)
{
	if (!m_window)
		return NULL;

	if (es == NULL)
		es = wkeGlobalExec(m_window);

	return jsToString(es, val);
}

int CWebView::jsValue2Int(jsValue val, jsExecState es /*= NULL*/)
{
	if (!m_window)
		return 0;

	if (es == NULL)
		es = wkeGlobalExec(m_window);

	return jsToInt(es, val);
}

bool CWebView::OnLoadUrlBeginCallBack(const utf8* url, wkeNetJob job)
{
	if (m_mapFsBuf.empty())
		return false;

	if (strncmp(FAKE_WEBVIEW_DOMAIN, url, 32) != 0) //strlen(fake_domain)==32
		return false;

	//解码为空就返回吧
	std::string data;
	std::string path = wkeUtilDecodeURLEscape(url + 32);//跳过域名
	if (!path.empty())
	{
		size_t endpos = path.rfind('?');
		if (endpos != std::string::npos)
			path = path.substr(0, endpos);

		auto iter = m_mapFsBuf.find(path);
		if (iter != m_mapFsBuf.end())
			data = iter->second;
	}

	//填充数据
	wkeNetSetData(job, (void *)data.c_str(), (int)data.length());

	//阻断继续网络访问
	return true;
}

LRESULT CALLBACK CWebView::SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
	if (uMsg == MSG_UI_DATA)
	{
		CWebView *pWebview = (CWebView *)dwRefData;
		if (pWebview)
			pWebview->OnUIMsg((int)wParam, (const void *)lParam);

		return 0;
	}

	if (uMsg == WM_NCDESTROY)
		RemoveWindowSubclass(hWnd, &CWebView::SubClassProc, 0);

	return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}
