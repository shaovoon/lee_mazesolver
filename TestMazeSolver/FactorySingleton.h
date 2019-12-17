#pragma once

using namespace D2D1;
using namespace Microsoft::WRL;

class FactorySingleton
{
public:
	static ComPtr<ID2D1Factory> GetGraphicsFactory();
	static ComPtr<IDWriteFactory> GetTextFactory();
private:
	static ComPtr<ID2D1Factory> m_GraphicsFactory;
	static ComPtr<IDWriteFactory> m_TextFactory;
};

