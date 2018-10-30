#include "stdafx.h"
#include "BlendState.h"

BlendState::BlendState()
	: state(NULL)
{
	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));

	desc.AlphaToCoverageEnable = false; //외곽선 커버를 바꿔줄 녀석
	desc.IndependentBlendEnable = false;

	desc.RenderTarget[0].BlendEnable = false;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA; //바닥색 Desc * DescBlend + Src * SrcBlend
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA; //src 현재 그릴색 DC * (1-a) + SC * (a)
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD; //

	//알파값 합성
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO; 
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; //알파 테스트 후 알파값을 쓸지 말지 테스트

	Changed();
}

BlendState::~BlendState()
{
	SAFE_RELEASE(state);
}

void BlendState::AlphaToCoverageEnable(bool val)
{
	desc.AlphaToCoverageEnable = val;

	Changed();
}

void BlendState::BlendEnable(bool val)
{
	desc.RenderTarget[0].BlendEnable = val;

	Changed();
}

void BlendState::OMSetBlendState()
{
	D3D::GetDC()->OMSetBlendState(state, NULL, 0xFF);
}

void BlendState::BlendOp(D3D11_BLEND_OP val)
{
	desc.RenderTarget[0].BlendOp = val;
	Changed();
}

void BlendState::DestBlend(D3D11_BLEND val)
{
	desc.RenderTarget[0].DestBlend = val;
	Changed();
}

void BlendState::SrcBlend(D3D11_BLEND val)
{
	desc.RenderTarget[0].SrcBlend = val;
	Changed();
}

void BlendState::Changed()
{
	SAFE_RELEASE(state);

	
	HRESULT hr = D3D::GetDevice()->CreateBlendState(&desc, &state);
	assert(SUCCEEDED(hr));
}
