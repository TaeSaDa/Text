#include "stdafx.h"
#include "DirectWrite.h"

DirectWrite* DirectWrite::instance = NULL;

ID2D1DeviceContext* DirectWrite::deviceContext = NULL;
IDWriteFactory* DirectWrite::writeFactory = NULL;

vector<FontBrushDesc> DirectWrite::fontBrush;
vector<FontTextDesc> DirectWrite::fontText;

void DirectWrite::Create()
{
	assert(instance == NULL);

	instance = new DirectWrite();
}

void DirectWrite::Delete()
{
	assert(instance != NULL);

	SAFE_DELETE(instance);
}

DirectWrite * DirectWrite::Get()
{
	return instance;
}

DirectWrite::DirectWrite()
{
	HRESULT hr;
	//__uuidof(IDWriteFactory) 클래스 자료형 객체 (IUnknown**) 은 무엇이 들어올지 모르기에 writeFactory 에 넣어라
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFactory);
	assert(SUCCEEDED(hr));

	D2D1_FACTORY_OPTIONS option;
	option.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	//2D 용 팩토리 생성
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
	assert(SUCCEEDED(hr));

	IDXGIDevice* dxgiDevice; //DXGI 는 다이렉트 디엑스 그래픽 인터페이스 : GPU 따라 DX특성이 달라지만 안돼서 추상단계를 준다 이 단계를 DXGI :: 동일한 역할을 할수있게 해주는 녀석
	//추상 단계란? 밑에 어떤 역활을 할진 모르지만 이름을 통일해서 통일에 의한 동작을 하기 위해 바꿔줄수있는 단계를 추상 단계
	//픽쳐 레벨 : 한번에 채크?
	hr = D3D::GetDevice()->QueryInterface(&dxgiDevice);
	assert(SUCCEEDED(hr));

	hr = factory->CreateDevice(dxgiDevice, &device);
	assert(SUCCEEDED(hr));

	hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &deviceContext);
	assert(SUCCEEDED(hr));


	IDXGISurface* dxgiSurface; //표면 스왑체인에 연결되어 출력해줄녀석
	D3D::GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), (void **)&dxgiSurface);
	assert(SUCCEEDED(hr));

	//DX2D용 비트맵
	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;//BGI를 써도 되자만 RGB가 편하므로
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96;
	bp.dpiY = 96; //인치 하나에 들어갈 도트 갯수 1인치를 쓴다면 DPI * 1 모니터 화면에 맞춰 자동으로 맞춰진다. 장치 독립적으로 하기 위해
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW; // 3D랑 쳡쳐서 들어가지말라고 써줌
	bp.colorContext = NULL;
	//장비가 좋아야한다

	//비트맵 생성
	hr = deviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bp, &tergetBitmap); //표면의 부터 비트맵 생성하라, 표면에 붙을녀석
	assert(SUCCEEDED(hr));

	deviceContext->SetTarget(tergetBitmap);

	//TODO : 테스트용
	/*hr = deviceContext->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1), &brush);
	assert(SUCCEEDED(hr));

	hr = writeFactory->CreateTextFormat(L"궁서", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"ko", &textFormat);
	assert(SUCCEEDED(hr));*/
}

DirectWrite::~DirectWrite()
{
	for (FontBrushDesc desc : fontBrush)
		SAFE_RELEASE(desc.Brush);

	for (FontTextDesc desc : fontText)
		SAFE_RELEASE(desc.Format);

	SAFE_RELEASE(tergetBitmap);

	SAFE_RELEASE(factory);
	SAFE_RELEASE(writeFactory);

	SAFE_RELEASE(deviceContext);
	SAFE_RELEASE(device);
}

void DirectWrite::RenderText(wstring text, RECT rect, float size, wstring font, D3DXCOLOR color, DWRITE_FONT_WEIGHT weight, DWRITE_FONT_STYLE style, DWRITE_FONT_STRETCH stretch)
{
	FontBrushDesc brushDesc;
	brushDesc.Color = color;

	FontBrushDesc* findBrush = NULL;
	for (FontBrushDesc& desc : fontBrush)
	{
		if (desc == brushDesc)
		{
			findBrush = &desc;
			break;
		}
	}

	if (findBrush == NULL)
	{
		D2D1::ColorF colorF = D2D1::ColorF(color.r, color.g, color.b);
		deviceContext->CreateSolidColorBrush(colorF, &brushDesc.Brush);

		fontBrush.push_back(brushDesc);
		findBrush = &brushDesc;
	}


	FontTextDesc textDesc;
	textDesc.Font = font;
	textDesc.FontSize = size;
	textDesc.Stretch = stretch;
	textDesc.Style = style;
	textDesc.Weight = weight;

	FontTextDesc* findText = NULL;
	for (FontTextDesc& desc : fontText)
	{
		if (desc == textDesc)
		{
			findText = &desc;
			break;
		}
	}

	if (findText == NULL)
	{
		writeFactory->CreateTextFormat
		(
			textDesc.Font.c_str(), NULL, textDesc.Weight, textDesc.Style, textDesc.Stretch, textDesc.FontSize,
			L"ko", &textDesc.Format
		);

		fontText.push_back(textDesc);
		findText = &textDesc;
	}

	D2D1_RECT_F rectF;
	rectF.left = (float)rect.left;
	rectF.right = (float)rect.right;
	rectF.top = (float)rect.top;
	rectF.bottom = (float)rect.bottom;

	deviceContext->DrawTextW
	(
		text.c_str(), text.size(), findText->Format, rectF, findBrush->Brush
	);
}

//void DirectWrite::RenderText(wstring text)
//{
//	D2D1_RECT_F rect;
//	rect.left = 100;
//	rect.right = 500;
//	rect.top = 0;
//	rect.bottom = 1000;
//
//	deviceContext->DrawTextW(text.c_str(), text.size(), textFormat, rect, brush);
//}
