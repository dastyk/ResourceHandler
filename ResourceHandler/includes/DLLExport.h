#ifndef DECLDIR_RH
#if defined DLL_EXPORT_RESOURCE_HANDLER
#define DECLDIR_RH_C extern "C" __declspec(dllexport)
#define DECLDIR_RH __declspec(dllexport)
#else
#define DECLDIR_RH_C extern "C" __declspec(dllimport)
#define DECLDIR_RH __declspec(dllimport)
#endif
#endif