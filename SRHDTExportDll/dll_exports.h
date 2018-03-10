#pragma once

#define DLL_EXPORT __declspec( dllexport ) 

extern "C"
{
    namespace hello
    {
        DLL_EXPORT void Init();
        DLL_EXPORT void Release();
        DLL_EXPORT void PredictImage(unsigned char* img_data, unsigned char* img_data_out, int height, int width, int channel);
    }
}

#undef DLL_EXPORT