#pragma once

#include "d3d10_1_base.h"
#include "d3d10_1_device.h"

namespace dxup
{
	template <typename DX10Interface, typename DX11Interface>
	class D3D10DeviceChild : public D3D10Base<DX10Interface, DX11Interface>
	{
	public:
		void STDMETHODCALLTYPE GetDevice(ID3D10Device **ppDevice)
		{
			this->m_device->AddRef();
			*ppDevice = this->m_device;
		}
	protected:
		D3D10Device* m_device;

	};

	template <typename DX10DescType, typename DX10InterfaceType, typename DX11InterfaceType>
	class D3D10DeviceChildDescBase : public D3D10DeviceChild<DX10InterfaceType, DX11InterfaceType>
	{
	protected:
		DX10DescType m_desc;
	};

	template <typename DX10DescType, typename DX10InterfaceType, typename DX11InterfaceType>
	class D3D10DeviceChildDesc1 : public D3D10DeviceChildDescBase<DX10DescType, DX10InterfaceType, DX11InterfaceType>
	{
	public:
		void STDMETHODCALLTYPE GetDesc1(DX10DescType* pDesc)
		{
			*pDesc = this->m_desc;
		}
	};

	template <typename DX10DescType, typename DX10InterfaceType, typename DX11InterfaceType>
	class D3D10DeviceChildDesc : public D3D10DeviceChildDescBase<DX10DescType, DX10InterfaceType, DX11InterfaceType>
	{
	public:
		void STDMETHODCALLTYPE GetDesc(DX10DescType* pDesc)
		{
			*pDesc = this->m_desc;
		}
	};
}
