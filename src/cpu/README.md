# CPU实现

---

## Code Walkthrough

[第一篇教程](http://aras-p.info/blog/2018/03/28/Daily-Pathtracer-Part-1-Initial-C-/)初始C++代码自上而下详解。

### 准备工作

`maths.h`文件中自定义了需要用到的结构体和函数。

`float3`既可表示颜色又可表示点/向量的坐标，并且实现了运算符重载。

`Ray`表示射线，包含射线起点和射线方向。

`Hit`表示射线和圆的交点，包含交点位置，交点法向量，射线起点到交点的距离。

`Sphere`球。

`Camera`相机，变量除了基本的坐标轴外（场景采用右手系，相机也是，z轴指向屏幕外），还有`lowerLeftCorner`投影平面左下角， `horizontalVec, verticalVec`用于方便将屏幕坐标转换为视平面坐标, `lensRadius`光圈半径。

构造函数的参数和OpenGL用到的类似：

```cpp
// 观察起点，观察终点，坐标系单位朝上向量，纵向fov，屏幕宽高比，光圈直径，焦距
Camera(const float3& lookFrom, const float3& lookAt, const float3& vup, float vfov, float aspect, float aperture, float focusDist);
```

用屏幕坐标获取对应光线的GetRay函数：

```cpp
// s和t表示屏幕上某像素点坐标宽度和高度占的比例，用于得到焦平面的对应像素点
Ray GetRay(float s, float t, uint32_t& state) const
```

此外还有一套获取随机数的函数。

reflect refract shilick

数学类的准备工作完成后，我们从上到下来进行代码walkthrough。

### GDI

第一个步骤是绘制窗口。由于是CPU实现，这里使用的是GDI库，它是windows的通用图形库，用于在窗口程序中绘制，和DirectX的应用场景不同。需要用到win32 API，所有传统windows的窗口都要用到这个win32 API，只是DirectX或OpenGL把这些封装起来了，实际上它们绘制窗口都要调用这些函数。[GDI官方文档](https://docs.microsoft.com/zh-cn/windows/desktop/gdi/windows-gdi)，[Win32官方入门教程](https://docs.microsoft.com/zh-cn/windows/desktop/LearnWin32/your-first-windows-program)。

`main.cpp`做的就是调用GDI将backbuffer绘制到屏幕上，使用了双缓冲。

### enkiTS

现在知道如何绘制backbuffer了，接下来是绘制buffer的函数`DrawTest()`和它调用的`TraceRowJob()`。这里用到了一个多线程库enkiTS。

#### enkiTS安装

把enkiTS/src目录下的文件包含到项目代码中；

把`D:\enkiTS\enkiTS\enkiTS.lib`包含到`项目-属性-链接器-输入-附加依赖项`。

#### 使用

用法参考github中的examples，`src/enkiTS-Helloworld`是使用demo。 其中`m_SetSize`是一个隐含变量，默认为1，为这个变量赋值相当于在外部给range赋值再执行ExecuteRange，而内部利用m_SetSize调用ExecuteRange是再次分为多线程并行的，每次分配的range不确定。

这里将屏幕每一行分配给线程执行，TraceRowJob做的就是遍历一行中的所有像素获取光线，再追踪得到该像素的颜色值写入backbuffer（这就是改写为GPU实现的核心）。

### Trace

### Scatter

---

## Problems

### 相机：透视投影，视锥体

位置顺序：视点——视平面——场景。我目前认为**视点和光圈是重合的**，因为在相机代码getRay函数计算射线时的视点加上了一个按光圈大小的偏移；**视平面是最终屏幕上显示的平面**，光线追踪中的射线就是从视点和视平面上的点得到的，再把这个射线投入场景中，因此视平面的宽高比和屏幕一样。

类似的还有焦距，焦距就是视点到视平面的距离，这里改变焦距最终只会影响光线追踪中投射的射线，所以焦距改为很小（0-0.1）图像很模糊或干脆都是天空的颜色，焦距很大的图像也会很模糊，但是改变焦距（不超过视点到场景的距离）并不会影响图像中物体的大小，因为透视投影比例是相同的（由下图，底片和投影面是一样的），当焦距超过了视点到场景的距离，感觉上应该是看到的场景非常小，但是实际上还是比例不变。但是焦距会影响清晰度（GPU implemention中观察得知），离视平面近的物体附近投射的射线多，显示清晰，反之相对模糊。

![windows](../imgs/window.PNG)

光圈我还不懂：真实相机的光圈作用是~~并不存在站在一个点，场景中一个范围的光线全部被捕捉，其实只有光圈中很小的一部分光线射入了~~，控制模糊范围。

