// dllmain.h : Declaration of module class.

class CdevicesModule : public ATL::CAtlDllModuleT< CdevicesModule >
{
public :
	DECLARE_LIBID(LIBID_devicesLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_DEVICES, "{F9AA5D7C-7266-4DAD-9062-3B7BC67A05A1}")
};

extern class CdevicesModule _AtlModule;
