# Note of Daily Pathtracer

[link](http://aras-p.info/blog/2018/03/28/Daily-Pathtracer-Part-0-Intro/)

---

## 1

c++代码的基本架构。

---

## 2

由于随机数在多线程中共享变量内存的问题，导致多核效率不高，但是我照他所说改了之后单位时间光线数没有增多。

---

## 3

使用struct而非class构建基本对象：简单类型是值存储，而复杂类型是堆存储索引，开销更大。详见[官方说明](https://docs.microsoft.com/en-us/dotnet/standard/design-guidelines/choosing-between-class-and-struct)

改写为c#代码在unity运行。多线程用Parallel.For实现。把DrawTest得到的buffer作为一种材质显示。

---

## 4

改正多帧融合问题：代码中使用了多个帧的颜色数值线性叠加的计算方式，但是sRGB是一个伽马校正后的颜色表示，所以需要修改最后的显示方法，以显示正确的颜色 。

改正漫反射bug：Scatter函数中漫反射是以单位外界球中随机点作为反射方向的，应该为单位外界球表面上随机点，这样满足单位化。

改正light sampling可能重复的问题：对于Lambert材质的球体，他们会发生漫反射和light sampling，做过light sampling后的漫反射若再吸收碰到物体发的光，就计算重复了（虽然这个场景里只会出现在漫反射球反射的光线碰到了光源球的情况，因为只有一个光源），所以需要Lambert材质下一次反射后不吸收emissive的光，修改在Trace函数中加一个bool判断，决定是否在这次Trace中加入emissive的量。这次修改后渲染效果大幅提升，少了很多噪点、黑边。

---

## 5&6

GPU implemention，Metal，D3D11

---

## 7

SIMD：Single instruction, multiple data。

第一种优化是使用SSE改写float3结构体。

第二种是开启MSVC（微软的VC编译器）的 /fp:fast模式。

这些优化主要是针对CPU进行的，提高并行度。

### SOA

SOA：sturuct of arrays，在结构体中对所有成员存储数组。与之相对的，常用的是AOS：array of structs，一个结构体所有实例的数组。使用SOA可以在内存访问成员时移动更少的距离，比如找到所有球体的半径变量，在使用SOA内存上是连续存储的。另外一个小优化是存储半径的平方：计算时只用到平方。

---

## 8

对HitSphere函数做SSE的SIMD优化：我根本看不懂。

---

## 9

别人对作者提出的一些优化：看不懂。

---

## 10

把7-9的优化在C#和GPU代码中实现。

---

## 11

从“深度优先”（递归计算光线）改为“广度优先”（初始光线buffer-一次碰撞后光线buffer...）。

---

## 12

把11的方法用于shader中，博客中他一直称这个东西为CS：compute shader，计算着色器。在改为buffer-oriented的GPU后，文中也遇到了很多问题，最后效率也不好。

---

## 13

回到递归计算光线的代码，把场景数据放入共享内存：不懂。

---

## ending

后续是一些移动平台/web实现，和一些博客参考。

---

[js教程，有关于光线追踪的讲解](http://www.cnblogs.com/miloyip/archive/2010/03/29/1698953.html)
