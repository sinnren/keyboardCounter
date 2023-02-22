// keycounts.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <sstream>
#include <ctime>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <atomic>
#include <array>
#include <numeric>
#include <windows.h>

#include "FTimerEvent.h"
#include "CppSQLite3.h"
#include "WebView.h"

#pragma warning(disable : 4996)

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Comctl32.lib")


#ifndef _DEBUG
#pragma comment(linker, "/subsystem:\"Windows\" /entry:\"mainCRTStartup\"")
#endif // _DEBUG

CppSQLite3DB g_db;
std::vector<uint32_t> g_keycounter;
int g_keydistence = 2;

//最后的键盘按键名
std::string g_lastkeyname;

//计算每分钟按键数
std::array<std::atomic<int>, 60> g_keyspeedrecorder;
int g_keyrecorderindex = 0;

//今日按键里程
volatile uint64_t g_totalkeycounts = 0;

LRESULT CALLBACK HookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT* pkbhs = (KBDLLHOOKSTRUCT*)lParam;

	if (nCode == HC_ACTION)
	{
		if (wParam == WM_KEYUP)
		{
			char keyname[32] = { 0 };
			DWORD msg = 1;
			msg += (pkbhs->scanCode << 16);
			msg += ((pkbhs->flags & LLKHF_EXTENDED) << 24);
			GetKeyNameTextA(msg, keyname, 32);
			g_lastkeyname = keyname;
			std::cout << "key: " << g_lastkeyname << "(" << pkbhs->vkCode << ")" << std::endl;
			g_keycounter.push_back(pkbhs->vkCode);

			g_keyspeedrecorder[g_keyrecorderindex]++;
			g_totalkeycounts++;
		}
	}
	
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void timerSaveKeys()
{
	if (g_keycounter.empty())
		return;

	std::cout << "保存键盘记录:" << g_keycounter.size() << "...\n";
	std::vector<uint32_t> keycounter;
	std::swap(keycounter, g_keycounter);
	
	std::string strKeys;
	strKeys.reserve(keycounter.size() * 4);
	for (uint32_t key : keycounter)
	{
		strKeys.append(std::to_string(key));
		strKeys.push_back('|');
	}
	
	try
	{
		auto s = g_db.compileStatement("insert into keys(tm, tmstr, keys, keycount) values(?, ?, ?, ?);");
		time_t t = ::time(0);
		struct tm tmt = { 0 };
		localtime_s(&tmt, &t);
		std::stringstream st;
		st << std::put_time(&tmt, "%Y-%m-%d %H:%M:%S");
		s.bind(1, t);
		s.bind(2, st.str().c_str());
		s.bind(3, strKeys.c_str());
		s.bind(4, (int64_t)keycounter.size());
		s.execDML();
	}
	catch (CppSQLite3Exception& e)
	{
		std::cout << "保存键盘记录错误:" << e.errorMessage() << std::endl;
	}

	std::cout << "保存键盘记录完成\n";
}


// //获取程序自身目录
 std::string GetAppPath()
 {
 	static std::string strAppPath;
	if (strAppPath.empty())
	{
		char szApp[1024] = { 0 };
		GetModuleFileNameA(NULL, szApp, sizeof(szApp));
		strAppPath = szApp;
		std::size_t pos = strAppPath.find_last_of("\\/");
		if (pos != std::string::npos)
			strAppPath = strAppPath.substr(0, pos);
	}
 	return strAppPath;
 }

int main()
{
    std::cout << "启动键盘记录..." << "\n";

	//初始化数据库
	try
	{
		g_db.open("keys.db");
		g_db.execDML("create table if not exists keys(tm int, tmstr text, keys text, keycount int);");
	}
	catch (CppSQLite3Exception& e)
	{
		std::cout << "初始化数据库错误:" << e.errorMessage() << std::endl;
		return -1;
	}

	g_keydistence = GetPrivateProfileIntA("setting", "keydistence", 0, (GetAppPath() + "\\keycounts.ini").c_str());
	if (g_keydistence == 0)
	{
		std::cout << "初始化配置错误:keydistence\n";
		return -1;
	}

	//初始化webview
	CWebView::SetWkeDllPath(L"node64.dll");
	if (CWebView::InitWebView() == 0)
	{
		std::cout << "初始化界面错误\n";
		return -1;
	}
	CWebView mainwnd;
	mainwnd.CreateWebViewWindow(wkeWindowType::WKE_WINDOW_TYPE_TRANSPARENT, NULL, 0, 0, 115, 70);
	//mainwnd.SetWindowIcon(IDC_MAINFRAME);
	mainwnd.SetWindowTitle("main");
#ifdef _DEBUG
	mainwnd.LoadURL(R"(C:\Users\fenlog\Desktop\keycounts\keycounts.html)");
#else
	mainwnd.LoadURL((GetAppPath() + "\\keycounts.html").c_str());
#endif // _DEBUG
	mainwnd.MoveToCenter();
	//不显示在任务栏
	LONG lStyle = ::GetWindowLong(mainwnd.GetWebViewWindowHWND(), GWL_EXSTYLE);
	lStyle |= WS_EX_TOOLWINDOW;
	::SetWindowLong(mainwnd.GetWebViewWindowHWND(), GWL_EXSTYLE, lStyle);
	//置顶
	::SetWindowPos(mainwnd.GetWebViewWindowHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	//显示窗口
	mainwnd.ShowWebViewWindow(true);

	
	HHOOK hhook= ::SetWindowsHookEx(WH_KEYBOARD_LL, HookCallback, GetModuleHandle(NULL), NULL);
	if (hhook == NULL)
	{
		std::cout << "安装钩子失败:" << GetLastError() << std::endl;
		return -1;
	}

	FTimerEvent timerTask(10 * 1000, timerSaveKeys);
	//显示最后按键名
	FTimerEvent timerShowKeyName(10, [&mainwnd]() {
		static std::string strShowKeyName;
		if (strShowKeyName != g_lastkeyname)
		{
			strShowKeyName = g_lastkeyname;
			mainwnd.RunJS(("document.getElementById('key').innerHTML = \"" + strShowKeyName + "\";").c_str());
		}
	});
	//显示按键速度
	FTimerEvent timerShowKeySpeed(1000, [&mainwnd]() {

		//计算速度
		int totalkeys = std::accumulate(g_keyspeedrecorder.cbegin(), g_keyspeedrecorder.cend(), 0);
		mainwnd.RunJS(("document.getElementById('speed').innerHTML = \"" + std::to_string(totalkeys) + "\";").c_str());
		
		//滑动记录器
		if (g_keyrecorderindex == 59)
			g_keyrecorderindex = 0;
		else
			g_keyrecorderindex++;
		
		//清0最后一秒的数据
		g_keyspeedrecorder[g_keyrecorderindex] = 0;	
	});
	//显示按键今日里程
	FTimerEvent timerShowDistence(1000, [&mainwnd]() {
		static int mday = 0;

		time_t t = ::time(0);
		struct tm tmt = { 0 };
		localtime_s(&tmt, &t);
		if (mday != tmt.tm_mday)
		{
			//天发生变化了  初始化总计数器
			mday = tmt.tm_mday;
			g_totalkeycounts;
		} 
		
		//按下弹起，两次键程，所以要x2，最后换算为米
		double distence = (double)(g_totalkeycounts * g_keydistence * 2) / 1000;
		std::stringstream ss;
		ss << std::setprecision(3) << distence;
		mainwnd.RunJS(("document.getElementById('distence').innerHTML = \"" + ss.str() + "\";").c_str());

	});

	MSG msg;
	while (GetMessageA(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	
	return 0;
}

