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

	//�ص�����
	std::function<void()> m_funCallback;
	//�˳����
	bool m_bExit;
	//��ʱʱ��
	volatile uint32_t m_nTimeout;
	//�ֶ�����timer������
	std::atomic<int> m_nManual;

	void _InitTimer()
	{
		m_nManual = 0;

		m_loopTask = std::async(std::launch::async, [this]()->int{

			//printf("timer thread start...\n");

			//�����߳�ID
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

				//���۳�ʱ
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

	//��ֹ�������ƶ�
	FTimerEvent(const FTimerEvent&) = delete;
	FTimerEvent& operator=(const FTimerEvent&) = delete;

	//Ĭ�ϵĹ��캯��
	FTimerEvent() : m_bExit(false), m_nTimeout(0){};

	//�������Ĺ��캯��
	template<class F, class... Args>
	FTimerEvent(uint32_t dwTimeout, F&& f, Args&&... args) : 
		m_bExit(false), m_nTimeout(dwTimeout) 
	{
		m_funCallback = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
		_InitTimer();
	}

	//���ʹ��Ĭ�Ϲ��캯�������ʾ�� �����ֶ���ʼ�� �������ظ�����
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

	//�����¼�
	//waitset���Ϊtrue�����ȴ��ֶ����õ��¼���ִ�лص���ϣ�false��ֱ�ӽ�������ִ��
	//����ڻص��ڲ����ã�����������
	void StopTimer(bool waitset = false)
	{
		assert(m_funCallback);
		if (m_funCallback == nullptr)
			return;
		//if (!m_bExit)
		//	printf("Stop Timer \n");

		//stop flag
		m_bExit = true;
		//���ȴ��ֶ�Set���¼�����0��
		if (!waitset)
			m_nManual = 0;

		//֪ͨһ�������¼�
		if (std::this_thread::get_id() != m_loopThreadID)
		{
			m_Condition.notify_one();
			
			//�ȴ��߳��˳�
			try
			{
				m_loopTask.get();
			}
			catch (...){}
		}
	}


	//����һ���¼� ��������
	//Set���ξͻᴥ�����λص�
	void SetTimerEvent()
	{
		assert(m_funCallback);
		if (m_funCallback == nullptr)
			return;
		
		m_nManual++;
		m_Condition.notify_one();
	}

	//�޸Ķ�ʱ��ʱ�䣬�´���Ч
	void SetTimeout(uint32_t nTimeout)
	{
		//���֮ǰ�ĳ�ʱΪ0�ֶ�������ô��Ҫ����һ�£������һֱ�������ﲻ����
		bool bNeedNotify = false;
		if (m_nTimeout == 0)
			bNeedNotify = true;

		m_nTimeout = nTimeout;
		if (bNeedNotify)
			m_Condition.notify_one();
	}

	//�Ƿ��Ѿ���ʼ����
	bool IsInited()
	{
		return (m_funCallback != nullptr);
	}
};



//////////////////////////////////////////////////////////////////////////
//�����ǲ��Դ���
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
			//�����ʱΪ0����ÿ��Ϊ�ֶ�����
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