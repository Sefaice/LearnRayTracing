## 安装

把enkiTS/src目录下的文件包含到项目代码中；

把`D:\enkiTS\enkiTS\enkiTS.lib`包含到`项目-属性-链接器-输入-附加依赖项`。

---

## enkiTS

多线程库，用法参考examples。

`m_SetSize`是一个隐含变量，默认为1，为这个变量赋值相当于在外部给range赋值再执行ExecuteRange，而内部利用m_SetSize调用ExecuteRange是再次分为多线程并行的，每次分配的range不确定。

---

## GDI

windows的通用图形库，用于在窗口程序中绘制，和DirectX的应用场景不同。需要用到win32 API，所有传统windows的窗口都要用到这个win32 API，只是DirectX或OpenGL把这些封装起来了，实际上它们绘制窗口都要调用这些函数。[GDI官方文档](https://docs.microsoft.com/zh-cn/windows/desktop/gdi/windows-gdi)，[Win32官方入门教程](https://docs.microsoft.com/zh-cn/windows/desktop/LearnWin32/your-first-windows-program)