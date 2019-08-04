#include <Windows.h>
#include <string>
 
void DoThings() {
	std::wstring message = L"Injection succeeded in PID " + std::to_wstring(GetCurrentProcessId());
	MessageBox(NULL, message.c_str(), L"Success", MB_OK);
}
// For DLL mode
extern "C" __declspec(dllexport)
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD ul_reason_for_call,
	LPVOID lpReserved) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		CreateThread(NULL, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(DoThings), NULL, 0, 0);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}