#include "pch.h"
#include "framework.h"
#include "FactorySingleton.h"

ComPtr<ID2D1Factory> FactorySingleton::m_GraphicsFactory;
ComPtr<IDWriteFactory> FactorySingleton::m_TextFactory;

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

ComPtr<IDWriteFactory> FactorySingleton::GetTextFactory()
{
	if (!m_TextFactory)
	{
		HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,
			__uuidof(m_TextFactory),
			reinterpret_cast<IUnknown**>(m_TextFactory.GetAddressOf())));
	}
	return m_TextFactory;
}

