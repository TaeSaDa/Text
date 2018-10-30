#pragma once

struct FontBrushDesc;
struct FontTextDesc;


class DirectWrite
{
public:
	static void Create();
	static void Delete();

	static DirectWrite* Get();
	static ID2D1DeviceContext* GetDC() { return deviceContext; }

private:
	static DirectWrite* instance;

private:
	DirectWrite();
	~DirectWrite();

private:
	ID2D1Factory1* factory; //숫자붙은건 픽쳐 레벨을 의미한다. 숫자가 늘어나면 전 숫자의 레벨을 상속받아서 업그레이드가 되어있다는 의미
	static IDWriteFactory* writeFactory;//팩토리가 있어야 스왑체인을 활용해서 만들수 있다
	
	ID2D1Device* device; //
	static ID2D1DeviceContext* deviceContext; //스왑체인 연동(Direct3D와 Direct2D를 연결해서 사용할수있게 해줄녀석)

	ID2D1Bitmap1* tergetBitmap;

private:
	static vector<FontBrushDesc> fontBrush;
	static vector<FontTextDesc> fontText;

public:
	static void RenderText(wstring text, RECT rect, float size = 20.0f, wstring font = L"돋움체", D3DXCOLOR color = D3DXCOLOR(1,1,1,1), DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL);



//TODO: 테스트용
//private:
//	ID2D1SolidColorBrush* brush;
//	IDWriteTextFormat* textFormat;
//
//public:
//	void RenderText(wstring text);
};

struct FontBrushDesc
{
	D3DXCOLOR Color;
	
	ID2D1SolidColorBrush* Brush = NULL;

	bool operator==(const FontBrushDesc& val)
	{
		return Color.r == val.Color.r &&
			Color.g == val.Color.g &&
			Color.b == val.Color.b &&
			Color.a == val.Color.a;
	}
};

struct FontTextDesc
{
	wstring Font;
	float FontSize;

	DWRITE_FONT_WEIGHT Weight;
	DWRITE_FONT_STYLE Style;
	DWRITE_FONT_STRETCH Stretch;

	IDWriteTextFormat* Format = NULL;

	bool operator == (const FontTextDesc& val)
	{
		bool b = true;
		b &= Font == val.Font;
		b &= FontSize == val.FontSize;
		b &= Weight == val.Weight;
		b &= Style == val.Style;
		b &= Stretch == val.Stretch;

		return b;
	}
};