#include "stdafx.h"
#include "RpcBinding.h"
#include "RpcBindingTemplate.h"
#include "RpcException.h"
#include "RpcProtocol.h"
#include "vm.service.h"
#include <memory>

RpcBindingTemplate t(nullptr, RpcProtocol::Local, _T("vm.service.RemoteSystemCalls"));
RpcBindingTemplate u = { nullptr, RpcProtocol::Local, _T("vm.service.RemoteSystemCalls") };

void test(void)
{
	fshandle_t fsh;

	RpcBinding* classic = RpcBinding::Compose(RpcProtocol::Local, _T("vm.service.RemoteSystemCalls"));
	////classic->Bind(RemoteSystemCalls_v1_0_c_ifspec);
	classic->Reset();
	HRESULT hr = rpc005_open(classic->Handle, nullptr, 0, 0, &fsh);
	delete classic;

	uuid_t junk = GUID_NULL;

	std::unique_ptr<RpcBinding> fast(RpcBinding::Create(u));
	fast->Bind(RemoteSystemCalls_v1_0_c_ifspec);
	fast->Reset();
	fast->Bind(RemoteSystemCalls_v1_0_c_ifspec);
	fast->Object = junk;
	HRESULT h2 = rpc005_open(fast->Handle, nullptr, 0, 0, &fsh);
	fast->Unbind();
	//delete fast;
}