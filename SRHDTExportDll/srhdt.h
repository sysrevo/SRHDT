#pragma once

#define DLL_EXPORT __declspec(dllexport)
#define DLL_IMPORT __declspec(dllimport)

#define SRHDT_API DLL_EXPORT

#define BYTE unsigned char

extern "C"
{
namespace srhdt
{
	/// <summary>
	/// Initialize the system.
	/// </summary>
    /// <param name="data_path">Path of the data file, in local encoding</param>
	SRHDT_API void Init(const char* data_path);

	/// <summary>
	/// Release and cleanup the current system.
	/// </summary>
	SRHDT_API void Release();

	/// <summary>
	/// Predict the sr image of the input image
	/// </summary>
    /// <param name="input_img">Pointer of the image data of type UINT8.</param>
    /// <param name="channel">Number of channels of input images. This can only be 1 (for gray image) or 3 (for BGR image) </param>
    /// <param name="input_width">Width of input image.</param>
    /// <param name="input_height">Height of input image.</param>
    /// <param name="output_width">Width of output image.</param>
    /// <param name="output_height">Height of output image.</param>
    /// <param name="output_img">Pointer of the image data to ouput. This must be pre-allocated and have the correct size.</param>
	SRHDT_API void PredictImage(BYTE* input_img, int channel, 
		int input_height, int input_width, int output_height, int output_width,
		BYTE* output_img);
}
}

#undef BYTE

#undef DLL_EXPORT
#undef DLL_IMPORT
#undef SRHDT_API