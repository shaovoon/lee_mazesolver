// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include "framework.h"
#include <wrl.h> // Windows Runtime Library for its ComPtr
#include <d2d1.h> // Direct2D v1 header
#include <dwrite.h> // DirectWrite header
#include <stdexcept> // C++11 exception class

#include "ComException.h"
#include "FactorySingleton.h" // Singleton to get Direct2D factories

// Direct2D and DirectWrite import libraries
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")

// Macro to test HRESULT and throw ComException for failed HRESULT
#define HR(expression) TestResult(#expression, (expression));


#endif //PCH_H
