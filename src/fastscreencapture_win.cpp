#pragma comment(lib, "DXGI.lib")
#pragma comment(lib, "D3D11.lib")

#include <QDebug>
#include <cassert>

#include "fastscreencapture_win.h"

#define IF_FAILED_CONTINUE(hresult) \
	if (FAILED((hresult))) continue

#define IF_FAILED_THROW(hresult) \
	if (FAILED((hresult))) \
	throw HrException(hresult, #hresult)

#define IF_NULL_PTR_THROW(hresult) \
	if ((hresult == nullptr)) \
		throw NullPointerException(#hresult)

namespace screenshot {

NullPointerException::NullPointerException(const char* expr)
	: FastScreenCaptureException(expr) {
}

const char* NullPointerException::what() const noexcept {
	return "Invalid pointer.";
}

FastScreenCaptureException::FastScreenCaptureException(const char* expr)
	: expr_(expr) {
}

HrException::HrException(HRESULT hr, const char* expr)
	: FastScreenCaptureException(expr)
	, hr_(hr) {
	message_ = std::system_category().message(hr);
}

const char* HrException::what() const noexcept {
	return message_.c_str();
}

static CComPtr<ID3D11Texture2D> createTexture(D3D11_TEXTURE2D_DESC texture_desc, CComPtr<ID3D11Device> &device) {
	D3D11_TEXTURE2D_DESC desc = { 0 };
	
	desc.Width = texture_desc.Width;
	desc.Height = texture_desc.Height;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.Format = texture_desc.Format;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;

	CComPtr<ID3D11Texture2D> texture;
	IF_FAILED_THROW(device->CreateTexture2D(&desc, nullptr, &texture));
	return texture;
}

class FastScreenCapture::DXGIOutput {
public:
	DXGIOutput(CComPtr<ID3D11Device> device,
		CComPtr<ID3D11DeviceContext> device_context,
		CComQIPtr<IDXGIOutput1> dxgi_output,
		CComPtr<IDXGIOutputDuplication> dupcation)
		: timeout_mills_(0)
		, device_(device)
		, device_context_(device_context)
		, dxgi_output_(dxgi_output)
		, dupcation_(dupcation) {
	}

	void allocateBuffer() {
		DXGI_OUTPUT_DESC desktop_desc = { 0 };
		IF_FAILED_THROW(dxgi_output_->GetDesc(&desktop_desc));

		auto desktop_rect = desktop_desc.DesktopCoordinates;
		auto desktop_width = desktop_rect.right - desktop_rect.left;
		auto desktop_height = desktop_rect.bottom - desktop_rect.top;
		buffer_.resize(desktop_width * desktop_height * 4);
	}

	void setInterval(int mills) {
		timeout_mills_ = mills;
	}

	const std::vector<BYTE> & getImage() {
		getNextFrameBuffer();
		return buffer_;
	}

private:
	void getNextFrameBuffer() {
		DXGI_OUTDUPL_FRAME_INFO frame_info = { 0 };

		CComPtr<IDXGIResource> desktop_resource;
		auto hr = dupcation_->AcquireNextFrame(timeout_mills_, &frame_info, &desktop_resource);
		if (hr == DXGI_ERROR_WAIT_TIMEOUT) {
			return;
		}
		IF_FAILED_THROW(hr);

		CComQIPtr<ID3D11Texture2D> texture_resource(desktop_resource);
		IF_NULL_PTR_THROW(texture_resource);

		D3D11_TEXTURE2D_DESC texture_desc = { 0 };
		texture_resource->GetDesc(&texture_desc);

		// Create Texture2D.
		auto texture = createTexture(texture_desc, device_);

		// Copy to texture.		
		device_context_->CopyResource(texture, texture_resource);

		CComQIPtr<IDXGISurface1> surface(texture);
		IF_NULL_PTR_THROW(surface);

		DXGI_MAPPED_RECT mapped_rect = { 0 };
		IF_FAILED_THROW(surface->Map(&mapped_rect, DXGI_MAP_READ));

		DXGI_OUTPUT_DESC desktop_desc = { 0 };
		IF_FAILED_THROW(dxgi_output_->GetDesc(&desktop_desc));

		auto desktop_rect = desktop_desc.DesktopCoordinates;

		auto desktop_width = desktop_rect.right - desktop_rect.left;
		auto desktop_height = desktop_rect.bottom - desktop_rect.top;
		OffsetRect(&desktop_rect, -desktop_rect.left, -desktop_rect.top);

		auto map_pitch_pixels = mapped_rect.Pitch / 4;
		auto stripe = desktop_width * 4;

		switch (desktop_desc.Rotation) {
		case DXGI_MODE_ROTATION_IDENTITY:
			for (LONG i = 0; i < desktop_height; i++) {
				memcpy_s(buffer_.data() + (desktop_rect.left + (i + desktop_rect.top) * desktop_width) * 4,
					stripe,
					mapped_rect.pBits + i * mapped_rect.Pitch,
					stripe);
			}
			break;
		}

		IF_FAILED_THROW(surface->Unmap());
		IF_FAILED_THROW(dupcation_->ReleaseFrame());
	}

	int timeout_mills_;
	std::vector<BYTE> buffer_;
	CComPtr<ID3D11Device> device_;
	CComPtr<ID3D11DeviceContext> device_context_;
	CComQIPtr<IDXGIOutput1> dxgi_output_;
	CComPtr<IDXGIOutputDuplication> dupcation_;
};

FastScreenCapture::FastScreenCapture() {
	IF_FAILED_THROW(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&factory_));	
	enumDXGIOutput();
	outputs_[0].allocateBuffer();
}

FastScreenCapture::~FastScreenCapture() {
}

void FastScreenCapture::setInterval(int mills) {
	outputs_[0].setInterval(mills);
}

const std::vector<BYTE>& FastScreenCapture::getImage() {
	return outputs_[0].getImage();
}

void FastScreenCapture::enumDXGIOutput() {
	std::vector<CComPtr<IDXGIAdapter1>> adapters;
	CComPtr<IDXGIAdapter1> adapter;

	for (auto i = 0; factory_->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; i++) {
		adapters.push_back(adapter);
		adapter.Release();
	}

	for (auto & adapter : adapters) {
		std::vector<CComPtr<IDXGIOutput>> outputs;
		CComPtr<IDXGIOutput> output;
		for (int i = 0; adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND; i++) {
			DXGI_OUTPUT_DESC output_desc;
			output->GetDesc(&output_desc);
			if (output_desc.AttachedToDesktop) {
				outputs.push_back(output);
			}
			output.Release();
		}

		CComPtr<ID3D11Device> device;
		CComPtr<ID3D11DeviceContext> device_context;
		D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_9_1;
		IF_FAILED_CONTINUE(D3D11CreateDevice(adapter,
			D3D_DRIVER_TYPE_UNKNOWN,
			nullptr,
			0,
			nullptr,
			0,
			D3D11_SDK_VERSION,
			&device,
			&fl,
			&device_context));

		for (auto& output : outputs) {
			CComQIPtr<IDXGIDevice1> dxgi_device(device);
			if (!dxgi_device) {
				continue;
			}
			CComQIPtr<IDXGIOutput1> dxgi_output(output);
			if (!dxgi_output) {
				continue;
			}
			CComPtr<IDXGIOutputDuplication> dupcation;
			IF_FAILED_THROW(dxgi_output->DuplicateOutput(dxgi_device, &dupcation));
			outputs_.emplace_back(device, device_context, dxgi_output, dupcation);
			IF_FAILED_THROW(imaging_factory_.CoCreateInstance(CLSID_WICImagingFactory));
		}
	}
}

}