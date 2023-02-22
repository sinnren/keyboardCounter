# keyboardCounter
统计手指敲键盘的里程工具。

每一次敲击键盘，手指行进了按键键程的距离

每天行走的步数和里程有各种方式统计，但是手指“行走”的距离缺没有统计工具。

于是。。。

keyboardCounter粗来啦。。。


# 配置
keycounts.ini里面，keydistence是按键的键程，一般情况下官方是有公布的，当然也可以自己粗略的估计一下。单位毫米。

# 构架
利用miniblink(https://github.com/miniblink/miniblink)作为ui框架，依赖的dll是note64.dll。
主界面是keycounts.html，随手写了几行。。。

# 后续想法
定时将所有按键信息都回写到了sqlite里面，可以依赖于miniblink库结合html+css+js去做漂亮的统计图等等东西，类似于搜狗输入法的输入统计，可以按时间、天、月等统计输入次数、速度等曲线图，等等吧。。。。

