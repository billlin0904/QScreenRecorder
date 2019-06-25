#pragma once

#include <windows.h>
#include <atlbase.h>
#include <DXGITYPE.h>
#include <DXGI1_2.h>
#include <d3d11.h>
#include <Wincodec.h>
#include <comdef.h>

#include <stdexcept>
#include <vector>

namespace screenshot {

class FastScreenCaptureException : public std::exception {
public:
	explicit FastScreenCaptureException(const char* expr);

protected:
	const char* expr_;
	std::string message_;
};

class NullPointerException : FastScreenCaptureException {
public:
	explicit NullPointerException(const char* expr);

	const char* what() const noexcept override;
};

class HrException : public FastScreenCaptureException {
public:
	HrException(HRESULT hr, const char *expr);

	const char* what() const noexcept override;

private:
	HRESULT hr_;
};

class FastScreenCapture {
public:
	FastScreenCapture();

	~FastScreenCapture();

	FastScreenCapture(const FastScreenCapture&) = delete;
	FastScreenCapture& operator=(const FastScreenCapture&) = delete;

	void setInterval(int mills);

	const std::vector<BYTE>& getImage();

private:
	class DXGIOutput;

	void enumAvailableOutput();

	CComPtr<IDXGIFactory1> factory_;
	CComPtr<IWICImagingFactory> imaging_factory_;
	std::vector<DXGIOutput> outputs_;
};

}