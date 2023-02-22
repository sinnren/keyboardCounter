#pragma once

#include <cassert>
#include <thread>
#include <functional>
#include <chrono>
#include <future>
#include <mutex>
#include <atomic>
#include <condition_variable>

class FTimerEvent
{
private:
	std::mutex m_mtx;
	std::condition_variable m_Condition;

	std::future<int> m_loopTask;
	std::thread::id m_loopThreadID;

	//回调函数
	std::function<void()> m_funCallback;
	//退出标记
	bool m_bExit;
	//超时时间
	volatile uint32_t m_nTimeout;
	//手动触发timer计数器
	std::atomic<int> m_nManual;

	void _InitTimer()
	{
		m_nManual = 0;

		m_loopTask = std::async(std::launch::async, [this]()->int{

			//printf("timer thread start...\n");

			//保存线程ID
			m_loopThreadID = std::this_thread::get_id();
			
			while (!m_bExit || m_nManual)
			{
				//wait for timeout
				std::unique_lock<std::mutex> lock(m_mtx);

				if (m_nManual == 0)
				{
					if (m_nTimeout)
						m_Condition.wait_for(lock, std::chrono::milliseconds(m_nTimeout));
					else
						m_Condition.wait(lock);
				}

				//不论超时
				if (m_nManual)
					m_nManual--;

				//callback
				//printf("timer call back ... \n");
				if (m_funCallback)
					m_funCallback();
			}

			//printf("timer thread exit.\n");

			return 0;
		});

	}

public:

	//禁止拷贝和移动
	FTimerEvent(const FTimerEvent&) = delete;
	FTimerEvent& operator=(const FTimerEvent&) = delete;

	//默认的构造函数
	FTimerEvent() : m_bExit(false), m_nTimeout(0){};

	//带参数的构造函数
	template<class F, class... Args>
	FTimerEvent(uint32_t dwTimeout, F&& f, Args&&... args) : 
		m_bExit(false), m_nTimeout(dwTimeout) 
	{
		m_funCallback = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		_InitTimer();
	}

	//如果使用默认构造函数定义的示例 可以手动初始化 不允许重复调用
	template<class F, class... Args>
	bool InitTimer(uint32_t dwTimeout, F&& f, Args&&... args)
	{
		assert(m_funCallback == nullptr);
		if (m_funCallback != nullptr)
			return false;
		
		m_nTimeout = dwTimeout;
		m_funCallback = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		_InitTimer();

		return true;
	}

	~FTimerEvent()
	{
		StopTimer();

		//printf("Timer has Stopped.  \n");
	}

	//阻塞事件
	//waitset如果为true，则会等待手动设置的事件都执行回调完毕；false就直接结束不再执行
	//如果在回调内部调用，会立即返回
	void StopTimer(bool waitset = false)
	{
		assert(m_funCallback);
		if (m_funCallback == nullptr)
			return;
		//if (!m_bExit)
		//	printf("Stop Timer \n");

		//stop flag
		m_bExit = true;
		//不等待手动Set的事件，清0掉
		if (!waitset)
			m_nManual = 0;

		//通知一下阻塞事件
		if (std::this_thread::get_id() != m_loopThreadID)
		{
			m_Condition.notify_one();
			
			//等待线程退出
			try
			{
				m_loopTask.get();
			}
			catch (...){}
		}
	}


	//触发一次事件 立即返回
	//Set几次就会触发几次回调
	void SetTimerEvent()
	{
		assert(m_funCallback);
		if (m_funCallback == nullptr)
			return;
		
		m_nManual++;
		m_Condition.notify_one();
	}

	//修改定时器时间，下次生效
	void SetTimeout(uint32_t nTimeout)
	{
		//如果之前的超时为0手动管理，那么需要激活一下，否则会一直卡在哪里不动弹
		bool bNeedNotify = false;
		if (m_nTimeout == 0)
			bNeedNotify = true;

		m_nTimeout = nTimeout;
		if (bNeedNotify)
			m_Condition.notify_one();
	}

	//是否已经初始化过
	bool IsInited()
	{
		return (m_funCallback != nullptr);
	}
};



//////////////////////////////////////////////////////////////////////////
//以下是测试代码
namespace timertester
{

	class timertestclass
	{
	public:
		timertestclass(){};
		~timertestclass(){};
		void testfn()
		{
			printf("timer callback is class func.\n");
			FTimerEvent mTimer(3000, &timertestclass::threadfn, this);

			for (int i = 0; i < 5; i++)
			{
				printf("press any key to ...\n");
				getchar();
				mTimer.SetTimerEvent();
			}

			printf("stop timer \n");
			mTimer.StopTimer();
		}
		void threadf(int i)
		{
			printf("test f i:%d\n", i);
		}
		void threadfn()
		{
			printf("test fn d\n");
		}
		void threadfn2()
		{
			printf("test fn2222 \n");
		}
	};


	static void testfn1(timertestclass *cls)
	{
		cls->threadfn();
	}
	static void testpf()
	{
		printf("test printf \n");
		printf("callback ret. \n");
	}
	static void prt(int& i)
	{
		printf("print %d\n", i);
	}
	static void timertest()
	{
		int t = 0;
		FTimerEvent timerstdref(1000, prt, std::ref(t));
		for (int i = 0; i < 10; i++)
		{
			t = i;
			Sleep(1000);
		}
		timerstdref.StopTimer();

		{
			printf("timer 0 manual to set\n");
			//如果定时为0，则每次为手动触发
			FTimerEvent timerman(0, [](){
				printf("timerman in.\n");
				Sleep(5000);
				printf("timerman out.\n");
			});
			timerman.SetTimerEvent();
			timerman.SetTimerEvent();
			timerman.StopTimer(true);


		}

		printf("stop timer in callback\n");
		FTimerEvent timer4;
		timer4.InitTimer(1000, [](FTimerEvent *pTimer){ printf("exit timer\n"); pTimer->StopTimer(); }, &timer4);
		Sleep(3000);
		timer4.StopTimer();

		printf("set timer in callback\n");
		FTimerEvent timer5;
		timer5.InitTimer(2000, [](FTimerEvent *pTimer){
			static bool set = false;
			printf("timer in\n");
			if (!set)
			{
				printf("set timer\n");
				pTimer->SetTimerEvent();
				set = true;
			}
			printf("timer out\n");
		}, &timer5);
		Sleep(10000);
		timer5.StopTimer();

		timertestclass test1;
		test1.testfn();

		int x = 0;
		FTimerEvent timerref(1000, [&x]()
		{
			printf("x: %d \n", x);
		});
		for (int i = 0; i < 10; i++)
		{
			x = i;
			Sleep(1000);
		}
		timerref.StopTimer();

		FTimerEvent timerx(1000, [&test1]()
		{
			test1.threadfn2();
		});
		Sleep(10000);
		timerx.StopTimer();

		FTimerEvent timer0(1000, testpf);
		Sleep(10000);
		timer0.StopTimer();

		FTimerEvent timer1(1000, testfn1, &test1);
		Sleep(10000);
		timer1.StopTimer();

		FTimerEvent timer2(1000, [](){ printf("lambada no param \n"); });
		Sleep(10000);
		timer2.StopTimer();

		int param = 0;
		FTimerEvent timer3(1000, [](int *p){ printf("lambada with param: %d \n", *p); }, &param);
		Sleep(1000);
		param = 3;
		Sleep(10000);
		timer3.StopTimer();

		
	}
}