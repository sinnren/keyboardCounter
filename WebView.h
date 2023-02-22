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

	//初始化一次即可
	static int InitWebView();
	//设置note.dll路径  默认是note.dll
	static void SetWkeDllPath(LPCTSTR lpDllPath);
	//无窗体消息循环的情况下可以手动调用
	static void DoMessageLoop();

	//绑定JS方法和值，都必须在创建浏览器之前调用，否则无效
	//绑定一个js方法
	void BindJSFunction(const char *name, int nArgCount, funcjsCallBack cb = nullptr);
	//绑定一个对js window绑定一个属性设置器回调
	//有bug，不能同时设置setter和getter 
	//https://github.com/weolar/miniblink49/issues/537 等作者更新...
	void BindJSValue(const char *valname, funcjsCallBack getter, funcjsCallBack setter);

	//浏览器是否已经创建/销毁
	bool IsAvailable();

	//创建浏览器窗口
	bool CreateWebViewWindow(wkeWindowType type, HWND parent, int x, int y, int width, int height);
	//获取浏览器窗口的句柄
	HWND GetWebViewWindowHWND();
	//窗口移动到屏幕中心
	void MoveToCenter();
	//显示窗口 默认创建后不显示
	void ShowWebViewWindow(bool show);
	//最小化窗口
	void MinimizeWindow();
	//最大化窗口
	void MaximizeWindow();
	//关闭窗口
	void CloseWebViewWindow();
	//销毁浏览器 包括窗口
	void DestroyWebViewWindow();

	//设置窗口背景透明 默认关闭
	void SetTransparent(bool b);
	//设置是否支持拖拽文件到页面，默认关闭
	void SetDragEnable(bool b);
	//设置窗口标题
	void SetWindowTitle(const utf8* title);
	void SetWindowTitle(const WCHAR *title);
	//设置窗口图标
	void SetWindowIcon(HICON hIcon);
	void SetWindowIcon(int nIconID, HINSTANCE hInstance = NULL);

	//开启cookies
	void SetCookieEnabled(bool bEnable);
	//设置local storage的全路径。如c:\mb\LocalStorage\，必须且只能是目录，默认是当前目录。
	void SetLocalStorageFullPath(const WCHAR *lpPath);
	//设置本地cookies目录路径 path是文件目录路径，不包括文件名，不能自动新建不存在的目录。
	void SetCookieJarPath(const WCHAR* path);


	//加载URL
	void LoadURL(const utf8* url);
	void LoadURL(const wchar_t* url);
	//加载文件
	void LoadFile(const wchar_t* filename);
	//加载HTML
	void LoadHtml(const utf8* html);
	//加载内存中的文件系统
	void LoadMemoryFS(const std::map<std::string, std::string> mapFsBuf, const char *indexpath);
	//获取主frame的URL
	const utf8 *GetURL();

	//运行js代码 内置跨线程处理方法
	jsValue RunJS(const utf8 *script);
	
	//打开调试工具 path是c:/miniblink-release/front_end/inspector.html 全路径，并且不能有中文
	void ShowDevTools(const char *path);

	//在非UI线程中要对webview交互是非法的，所以要跨线程进行交互回调，所以别在UI线程中调用这玩意。。。
	template<class F>
	auto RunInUIThread(const F& f) -> decltype(f())
	{
		assert(m_dwUIThreadID != GetCurrentThreadId());

		//整出来返回值
		std::packaged_task<decltype(f())()> task(f);
		std::future<decltype(f())> ret = task.get_future();

		//用function再次封装 统一函数格式方便接收处理
		std::function<void()> func = [&task](){ task(); };

		//发送到UI线程中
		SendUIThreadMsg(UI_DATA_CMD_DEFAULT, &func);

		//等待执行并返回结果
		return ret.get();
	}


	const utf8* jsValue2String(jsValue val, jsExecState es = NULL);
	int jsValue2Int(jsValue val, jsExecState es = NULL);


protected:

	//任何网络请求发起前会触发此回调 返回true表示取消
	//LoadMemoryFS依赖原始方法
	virtual bool OnLoadUrlBeginCallBack(const utf8* url, wkeNetJob job);

	//网络请求结束时会触发此回调，只有设置了wkeNetHookRequest才会触发该接口，wkeNetSetData不会触发（官方文档有误）
	virtual void OnLoadUrlEndCallBack(const utf8 *url, void *job, void* buf, int len){};

	//网页加载失败时触发此回调
	virtual void OnLoadUrlFailCallback(const utf8* url, wkeNetJob job){};

	//网页Document数据结构生成完成时触发此回调
	//由于现在网页多采用异步加载模式，Document数据结构生成完成 ≠ 网页加载完成，每个异步执行的加载动作完成时都可能会触发一次本回调，注意是可能，不是一定，另外页面中如有多个frame，每个frame加载完成时都会触发一次本回调，实践中可以采用wkeOnLoadingFinish接口，wkeOnTitleChanged接口，wkeOnURLChanged接口，以及检查页面中某个元素（如网站logo等）是否存在等方法来综合判断页面加载是否完成。
	virtual void OnDocumentReadyCallBack(){};
	virtual void OnDocumentReady2CallBack(wkeWebFrameHandle frameId){};

	//wkeWebView如果是正常窗口模式，则在收到WM_CLODE消息时触发此回调接口，可返回false拒绝关闭窗口
	virtual bool OnWindowClosingCallBack(){ return true; };

	//窗口即将被销毁时触发此回调 此时m_window已经被销毁并初始化为NULL
	virtual void OnWindowDestroyCallBack(){};

	//网页中js方法回调
	virtual jsValue OnJsNativeFunctionCallBack(const std::string &funcname, jsExecState es){ return jsUndefined(); };
	//默认内置的_excmd()方法
	virtual jsValue OnJsExCmdCallBack(jsExecState es){ return jsUndefined(); };

	//跨UI线程 接收到的要处理的用户消息
	virtual void OnUserUIMsg(int nCmd, const void *data){};

private:
	wkeWebView m_window;
	HWND m_windowHwnd;
	DWORD m_dwUIThreadID;
	CRenderGDI *m_pRender;  //暂时用不到 先定义着。。。
	std::map<std::string, std::string> m_mapFsBuf;

	void OnUIMsg(int nCmd, const void *data);
	void SendUIThreadMsg(int nCmd, const void *data);
	
	static LRESULT CALLBACK SubClassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);


	std::vector<std::tuple<std::string, int, funcjsCallBack>> m_arrBindFunctions;
	std::vector<std::tuple<std::string, funcjsCallBack, funcjsCallBack>> m_arrBindValues;

};

