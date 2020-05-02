# ReDraw

随机重绘图像。目前只支持绘制射线，辐射状充填。C89 规范编写，除 GUI 部分外没有额外依赖。

![screenshot](https://github.com/i-evi/redraw/raw/master/demo/screenshot.gif)

|                原图                    |               重绘                 |
|----------------------------------------|-----------------------------------|
|![](https://github.com/i-evi/redraw/raw/master/demo/amplogo.png)|![](https://github.com/i-evi/redraw/raw/master/demo/preview_logo.png)|
|![](https://github.com/i-evi/redraw/raw/master/demo/test.jpg)|![](https://github.com/i-evi/redraw/raw/master/demo/preview_test.jpg)|


* 在 linux 上应该很容易编译，直接 `make` 就好了。

* 没有提供命令行版本，如果需要，参考 `random_draw.h`，很容易实现。

* 因为使用的 [Nuklear GUI](https://github.com/Immediate-Mode-UI/Nuklear) 用了 SDL&OpenGL 后端， Windows 用户请使用 [Mingw](http://mingw.org/) 编译（现在下载 Mingw 似乎在安装 GCC 时就会安装好 OpenGL，所以下载安装 [SDL](http://libsdl.org/) 就好了）。因为我实在不太会用 Visual Studio。如果觉得安装 Mingw 麻烦的话，我已经编译了一个 exe 文件在 repo 里（以后可能不维护）。

* 打开和保存文件时分别用了 Windows API 和 GTK API，参考 `sys_fn.c`，因此 Mac OS 应该没法直接编译（我没有 Mac 电脑，所以没有写 Mac OS 的部分）。

