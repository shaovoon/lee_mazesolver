## Table of Contents

* Introduction
* Lee Algorithm for Mazesolving
* Important Notes
* Direct2D Code
* References
* History

![screenshot.png](/images/screenshot.png)

## Introduction

In 1999, my team won Obstacle Avoidance Robot(OAR) first prize in the Singapore Robotics Games(SRG). That was 20 years ago. I do not have a photo of OAR, but it looks very much like micromouse. The micromouse maze is constructed with the walls while OAR maze is constructed with obstacles. Both micromouse and OAR team in my school make use of [Lee algorithm](https://en.wikipedia.org/wiki/Lee_algorithm) to solve the maze. The Lee algorithm was used to route single layer print circuit board (PCB) in 1960s and is of historical footnote until Google uses it as an interview technical test. I must point out the algorithm described in the Wikipedia page and heat map seems fishy based on my rusty memory of the algorithm, by right, the heat map should change around obstacles. Lee algorithm is a very simple algorithm where the robot travels from a higher value cell to lower value cell. Each cell value is determined from the minimum value of the 4 neighboring cells and increment one. Diagonal cell values are not taken into consideration.

The green cell is the starting cell while the yellow one is the destination. The program cannot see the entire map of obstacles. It updates its map with obstacle as it explores along. When start button is clicked, there are 2 runs: first is maze solving mode and second is optimized path mode. Optimized path might not be shortest path because as I say, it does not have the entire map, optimized path is based on the path it has explored before.

The original simulator written in Turbo C++ 20 years ago was in DOS. This time, I wrote a Windows version with MFC and Direct2D. Direct2D turned out surprisingly intuitive to use. The first half of article explains the Lee algorithm while the second half on the Direct2D code.

## Lee Algorithm for Mazesolving

Choose a maximum number, `m` to represent obstacle, say `0xFF` and the default value for unoccupied cell is `m-1`. Why `0xFF`? The maximum number would depend on your cell dimensions of your maze or (grid if talking about PCB routing). An maze of 16x16 cells would be 16*16=256(`0x100`). In other words, choose a maximum value which is impossible to exceed in any circumstance. Choice of `0xFF` is due to embedded hardware limitation because the 2 dimensional array is made up of `unsigned char` due to limited RAM on the motherboard.

1. Set current robot in the cell[0][15].
1. Set the value of destination cell[12][3] to 0.
1. Before travelling next cell, do the below to determine the next cell.
1. Set `changed` variable to `false`.
1. Initialize all empty cells to be `m-1` if they are not occupied by obstacle.
1. Do the following for every empty cell, i.e., skip the obstacle occupied cell. Calculate the value of current cell by finding the minimum value of neighboring cells (up, down, left, right) and add one and if the new value is not equal to current value, set `changed` to `true`. Set new value as the cell value.
1. If `changed` is `true`, repeat steps 4 to 7, else do step 8.
1. Travel to the next neighbouring cell with the minimum value.
1. If current cell is destination cell[12][3], stop else do from step 3 onwards.

There are some default behaviour not documented by the pseudo-code above. If front cell and left or right cell have the same minimum value, favor travelling straight instead of making a turn. If left or right cell has the same minimum value, favor making right turn. There is no right or wrong with these defaults. It depends on the maze. Robot could make a right turn leads to a dead end while left turn could lead to the destination. Whatever your choice of default behaviour, stick with them.

This is a partial weightage map (full map is too big to be shown) which all other cells in the map, other than the destination, are all initialized to `0xFE` while the destination is set to `0x00`.

```
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|00|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
```

After the map is initialized, the weightage is calculated by getting all the neighbouring cells and find the least value among them and add one to the least value and set it as its current cell value. The (partial) weightage map looks like this.

```
06|05|04|03|04|05|06
05|04|03|02|03|04|05
04|03|02|01|02|03|04
03|02|01|00|01|02|03
04|03|02|01|02|03|04
05|04|03|02|03|04|05
06|05|04|03|04|05|06
```

If there is any maze which this mazesolver cannot solve, save it and sent it to me or message me in this forum. Of course, if you cordon off the yellow destination cell with obstacles completely, it will keep running forever until you stop it. Below is the weightage when the destination cell is surrounded by obstacles (represented by `0xFF`).

```
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
FE|FE|FF|FF|FF|FE|FE
FE|FE|FF|00|FF|FE|FE
FE|FE|FF|FF|FF|FE|FE
FE|FE|FE|FE|FE|FE|FE
FE|FE|FE|FE|FE|FE|FE
```

## Important Notes

Difference between classic Lee Algorithm literature and this article, the start and target point are interchanged. Classic paper says setting the start point, not target point as zero. This is why I use the name of destination cell, not target cell to avoid confusion. In PCB routing, there is no real notion of a start and a target point, the algorithm is just trying to connect 2 endpoints.

## Direct2D Code

In this section, we&#39;ll examine the Direct2D code used. Direct2D, DirectWrite and Windows Image Component(WIC) are designed from ground up to replace the ageing GDI+, their method call are as inituitive, only complaint is the class names are sometimes very verbose, a side-effect from being descriptive. Direct2D and DirectWrite are based on Component Object Model(COM) but `CoInitialize()` and `CoUninitialize()` calls are not required. This exception rule does not apply to WIC which needs COM runtime. In this mazesolver application, we are only utilizing Direct2D because text and image rendering are not required. To use Direct2D, we&#39;ll need the Windows Runtime Library (WRL) `ComPtr` smart pointer, similar to ATL&#39;s `CComPtr` (note the extra `C` prefix) in a way that both keeps COM object alive by incrementing reference count though `IUnknown` interface&#39;s `AddRef()` and destroy it after the reference count decremented by `Release()` to zero. Below is a list of headers the application needed.

```Cpp
#include <wrl.h>              // Windows Runtime Library for its ComPtr
#include <d2d1.h>             // Direct2D v1 header
#include <stdexcept>          // C++ exception class

#include "ComException.h"
#include "FactorySingleton.h" // Singleton to get Direct2D factories
```

Next, we import Direct2D library for linkage.

```Cpp
// Direct2D import libraries
#pragma comment(lib, "d2d1")
```

We have the `FactorySingleton` to restrict the instance of ID2D1Factory to one.

```Cpp
using namespace D2D1;
using namespace Microsoft::WRL;

class FactorySingleton
{
public:
    static ComPtr<ID2D1Factory> GetGraphicsFactory();
private:
    static ComPtr<ID2D1Factory> m_GraphicsFactory;
};
```

`FactorySingleton::GetGraphicsFactory()` creates the factory if `m_GraphicsFactory` is `nullptr`.

```Cpp
#include "FactorySingleton.h"

ComPtr<ID2D1Factory> FactorySingleton::m_GraphicsFactory;

ComPtr<ID2D1Factory> FactorySingleton::GetGraphicsFactory()
{
    if (!m_GraphicsFactory)
    {
        D2D1_FACTORY_OPTIONS fo = {};

#ifdef DEBUG
        fo.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

        HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
            fo,
            m_GraphicsFactory.GetAddressOf()));

    }
    return m_GraphicsFactory;
}
```

Coming up next is the declaration of the `ComException` and `TestResult`.

```Cpp
#include <Windows.h>
#include <string>

void TestResult(const char* expression, HRESULT hr);

struct ComException
{
    HRESULT const hr;
    std::string where;
    ComException(const char* expression, HRESULT const value) : where(expression), hr(value) {}
    std::string Message() const;
};
```

The body of `TestResult()` and `ComException::Message()` are defined. `TestResult()` is actually used in the `HR` macro to check for failed `HRESULT`.

```Cpp
#include "ComException.h"

void TestResult(const char* expression, HRESULT hr)
{
    if (FAILED(hr)) throw ComException(expression, hr);
}

std::string ComException::Message() const
{
    char buf[800];
    memset(buf, 0, sizeof(buf));
    sprintf_s(buf, "ComException hr:%x, Where:%s", hr, where.c_str());
    std::string str = buf;
    return str;
}

// Macro to test HRESULT and throw ComException for failed HRESULT
#define HR(expression) TestResult(#expression, (expression));
```

Next, we declare the render `target(RT)` member objects and the brushes within the `ComPtr` smart pointer. To draw something in Direct2D, we need to have at least one render target. But why 2 render targets? `m_BmpTarget` is the offscreen RT based on a bitmap. While drawing is being done to `m_DCTarget`, user could see the incomplete or partial drawing, therefore everything is drawn on an offscreen `m_BmpTarget` which its bitmap is then `DrawImage()` by `m_DCTarget`.

```Cpp
ComPtr<ID2D1DCRenderTarget> m_DCTarget;
ComPtr<ID2D1BitmapRenderTarget> m_BmpTarget;

ComPtr<ID2D1SolidColorBrush> m_BmpBlackBrush;
ComPtr<ID2D1SolidColorBrush> m_BmpWhiteBrush;
ComPtr<ID2D1SolidColorBrush> m_BmpYellowBrush;
ComPtr<ID2D1SolidColorBrush> m_BmpGreenBrush;
ComPtr<ID2D1SolidColorBrush> m_BmpRedBrush;
```

Direct2D device dependent resources such as brush are tied to the RT. You cannot use a resource created from a RT on another RT. `CreateDeviceResources()` is a function to create brush. `ReleaseAndGetAddressOf()` is called to release the COM object and return address of the `ComPtr`&#39;s encapsulated raw pointer member in a pointer-to-pointer argument because a new `brush` object is about to be created and assigned to this argument.

```Cpp
void CreateDeviceResources(
    ID2D1RenderTarget* target, 
    ComPtr<ID2D1SolidColorBrush>& brush, 
    D2D1_COLOR_F color)
{
    HR(target->CreateSolidColorBrush(color,
        brush.ReleaseAndGetAddressOf()));
}
```

Next, the `m_DCTarget` and `m_BmpTarget` creation is shown. we need to specify the `pixelFormat` to be `DXGI_FORMAT_B8G8R8A8_UNORM` in order to be compatible with GDI&#39;s device context(DC) format. That is the `m_DCTarget` renders to DC. `Get()` is to return the internal raw pointer to RT encapsulated by the `ComPtr` smart pointer.

```Cpp
// Create a pixel format and initial its format
// and alphaMode fields.
// https://docs.microsoft.com/en-gb/windows/win32/direct2d/
// supported-pixel-formats-and-alpha-modes#supported-formats-for-id2d1devicecontext
D2D1_PIXEL_FORMAT pixelFormat = D2D1::PixelFormat(
    DXGI_FORMAT_B8G8R8A8_UNORM,
    D2D1_ALPHA_MODE_PREMULTIPLIED
);

D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
props.pixelFormat = pixelFormat;

HR(FactorySingleton::GetGraphicsFactory()->CreateDCRenderTarget(&props,
    m_DCTarget.ReleaseAndGetAddressOf()));

m_DCTarget->BindDC(pDC->GetSafeHdc(), &rect);

HR(m_DCTarget->CreateCompatibleRenderTarget(m_BmpTarget.ReleaseAndGetAddressOf()));

m_BmpTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
m_BmpTarget->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

CreateDeviceResources(m_BmpTarget.Get(), m_BmpBlackBrush, COLOR_BLACK);
CreateDeviceResources(m_BmpTarget.Get(), m_BmpWhiteBrush, COLOR_WHITE);
CreateDeviceResources(m_BmpTarget.Get(), m_BmpYellowBrush, COLOR_YELLOW);
CreateDeviceResources(m_BmpTarget.Get(), m_BmpGreenBrush, COLOR_GREEN);
CreateDeviceResources(m_BmpTarget.Get(), m_BmpRedBrush, COLOR_RED);
```

In case you have been wondering how the colors for the brush are defined, they are listed below. `D2D_COLOR_F` color channels are namely in RGBA in `float` type.

```Cpp
D2D_COLOR_F const COLOR_WHITE = { 1.0f,  1.0f,  1.0f,  1.0f };
D2D_COLOR_F const COLOR_BLACK = { 0.0f,  0.0f,  0.0f,  1.0f };
D2D_COLOR_F const COLOR_YELLOW = { 0.99f, 0.85f, 0.0f,  1.0f };
D2D_COLOR_F const COLOR_GREEN = { 0.0f,  1.0f,  0.0f,  1.0f };
D2D_COLOR_F const COLOR_RED = { 1.0f,  0.0f,  0.0f,  1.0f };
```

To draw a white line, `DrawLine()` is called with one start point, one end point and the white brush.

```Cpp
D2D1_POINT_2F p0{ 0.0f, sum };
D2D1_POINT_2F p1{ 50.0f, sum };
m_BmpTarget->DrawLine(p0, p1, m_BmpWhiteBrush.Get());
```

To draw a white obstacle, `FillRectangle()` is called with a rectangle and a white brush.

```Cpp
auto rectTarget = RectF(0.0f, 0.0f, 10.0f, 10.0f);
m_BmpTarget->FillRectangle(&rectTarget, m_BmpWhiteBrush.Get());
```

The obstacle avoidance robot is represented by a green circle. To draw a ellipse as a circle, `D2D1_ELLIPSE` need to be initialized with a center point and 2 equal `radiusX` and `radiusY` value. Then supply the `D2D1_ELLIPSE` object to the `FillEllipse()` with its brush.

```Cpp
D2D1_ELLIPSE ell;
ell.point = Point2F(5.0f, 5.0f);
ell.radiusX = 3.0f;
ell.radiusY = 3.0f;
m_BmpTarget->FillEllipse(ell, m_BmpGreenBrush.Get());
```

Next, we are ready to implement the `OnPaint()` function. If `m_DCTarget` is `nullptr`, `CreateDCTarget()` creates both the `m_DCTarget` and `m_BmpTarget`. `BeginDraw()` must precede before any drawing and `EndDraw()` must be called after all drawing calls are completed. `DrawMap()` and `DrawObstacles()` are performed on `m_BmpTarget`. Then `m_DCTarget` is binded with DC, this is something which has to be done on every `OnPaint()` calls. In between the `m_DCTarget`&#39;s `BeginDraw()` and `EndDraw()`, `m_DCTarget` draws `m_BmpTarget`&#39;s internal bitmap. If `EndDraw()` fails with `D2DERR_RECREATE_TARGET`, `m_DCTarget` is reset (meaning destroyed), `Invalidate()` sends `WM_PAINT` messsage to get the `OnPaint()` to be called again with a null `m_DCTarget` (to be created again).

```Cpp
CPaintDC dc(this);
if (!m_DCTarget)
{
    CreateDCTarget(&dc);
}

ComPtr<ID2D1Bitmap> bitmap;
m_BmpTarget->GetBitmap(bitmap.GetAddressOf());

CRect rectClient;
GetClientRect(&rectClient);

RECT rect;

rect.top = 0;
rect.bottom = rectClient.Height();
rect.left = SHIFT_LEFT;
rect.right = rectClient.Width();

m_BmpTarget->BeginDraw();
DrawMap();
DrawObstacles();
m_BmpTarget->EndDraw();

m_DCTarget->BindDC(dc.GetSafeHdc(), &rect);

m_DCTarget->BeginDraw();

m_DCTarget->DrawBitmap(bitmap.Get());

if (D2DERR_RECREATE_TARGET == m_DCTarget->EndDraw())
{
    m_DCTarget.Reset();
    Invalidate();
    return;
}
```

Minimum OS should be Windows 7 and onwards because Direct2D features for higher Windows 8 and above is not used. Have fun playing with the program!

The code is hosted on [GitHub](https://github.com/shaovoon/lee_mazesolver).

## References

* [Lee Algorithm on Wikipedia](https://en.wikipedia.org/wiki/Lee_algorithm)
* [Direct2D Fundamentals](https://app.pluralsight.com/library/courses/direct2d-fundamentals/table-of-contents) by Kenny Kerr

## History

* 2024-03-04: Fixed the optimized path by giving the full view of the obstacle map.
* 2021-12-04: Added 10 new mazes for download. __Note:__ Optimized path is based on the 1st travelled path, so there can be another more optimal path which is not discovered. Robot does not have the complete view of the map.
* 2020-01-01: Added "Important Notes" section
* 2019-12-22: First release

