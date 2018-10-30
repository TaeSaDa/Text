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
	//__uuidof(IDWriteFactory) Ŭ���� �ڷ��� ��ü (IUnknown**) �� ������ ������ �𸣱⿡ writeFactory �� �־��
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&writeFactory);
	assert(SUCCEEDED(hr));

	D2D1_FACTORY_OPTIONS option;
	option.debugLevel = D2D1_DEBUG_LEVEL_NONE;
	//2D �� ���丮 ����
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &factory);
	assert(SUCCEEDED(hr));

	IDXGIDevice* dxgiDevice; //DXGI �� ���̷�Ʈ �𿢽� �׷��� �������̽� : GPU ���� DXƯ���� �޶����� �ȵż� �߻�ܰ踦 �ش� �� �ܰ踦 DXGI :: ������ ������ �Ҽ��ְ� ���ִ� �༮
	//�߻� �ܰ��? �ؿ� � ��Ȱ�� ���� ������ �̸��� �����ؼ� ���Ͽ� ���� ������ �ϱ� ���� �ٲ��ټ��ִ� �ܰ踦 �߻� �ܰ�
	//���� ���� : �ѹ��� äũ?
	hr = D3D::GetDevice()->QueryInterface(&dxgiDevice);
	assert(SUCCEEDED(hr));

	hr = factory->CreateDevice(dxgiDevice, &device);
	assert(SUCCEEDED(hr));

	hr = device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS, &deviceContext);
	assert(SUCCEEDED(hr));


	IDXGISurface* dxgiSurface; //ǥ�� ����ü�ο� ����Ǿ� ������ٳ༮
	D3D::GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), (void **)&dxgiSurface);
	assert(SUCCEEDED(hr));

	//DX2D�� ��Ʈ��
	D2D1_BITMAP_PROPERTIES1 bp;
	bp.pixelFormat.format = DXGI_FORMAT_R8G8B8A8_UNORM;//BGI�� �ᵵ ���ڸ� RGB�� ���ϹǷ�
	bp.pixelFormat.alphaMode = D2D1_ALPHA_MODE_IGNORE;
	bp.dpiX = 96;
	bp.dpiY = 96; //��ġ �ϳ��� �� ��Ʈ ���� 1��ġ�� ���ٸ� DPI * 1 ����� ȭ�鿡 ���� �ڵ����� ��������. ��ġ ���������� �ϱ� ����
	bp.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW; // 3D�� ���ļ� ��������� ����
	bp.colorContext = NULL;
	//��� ���ƾ��Ѵ�

	//��Ʈ�� ����
	hr = deviceContext->CreateBitmapFromDxgiSurface(dxgiSurface, &bp, &tergetBitmap); //ǥ���� ���� ��Ʈ�� �����϶�, ǥ�鿡 �����༮
	assert(SUCCEEDED(hr));

	deviceContext->SetTarget(tergetBitmap);

	//TODO : �׽�Ʈ��
	/*hr = deviceContext->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1), &brush);
	assert(SUCCEEDED(hr));

	hr = writeFactory->CreateTextFormat(L"�ü�", NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 30.0f, L"ko", &textFormat);
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
