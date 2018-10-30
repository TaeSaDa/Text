#include "stdafx.h"
#include "Sky.h"
#include "../Objects/GameModel.h"

Sky::Sky(ExecuteValues* values)
	: values(values)
	, realTime(false), theta(0), phi(0), domeCount(32)
	, radius(10), slices(32), stacks (16), timeFactor(0.0f)
{
	mieTarget = new RenderTarget(values, 128, 64);
	rayleighTarget = new RenderTarget(values, 128, 64);

	scatterShader = new Shader(Shaders + L"044_Scattering.hlsl", "VS_Scattering", "PS_Scattering");
	targetShader = new Shader(Shaders + L"044_Scattering.hlsl", "VS_Target", "PS_Target");
	moonShader = new Shader(Shaders + L"044_Scattering.hlsl", "VS_Moon", "PS_Moon");
	cloudShader = new Shader(Shaders + L"044_Scattering.hlsl", "VS_Cloud", "PS_Cloud");

	worldBuffer = new WorldBuffer(); 					
	moonBuffer = new WorldBuffer();						
	targetBuffer = new TargetBuffer();					
	scatterBuffer = new ScatterBuffer();
	cloudBuffer = new CloudBuffer();
														
	GenerateSphere();									
	GenerateQuad();
	GenerateCloud();

	rasterizerState[0] = new RasterizerState();
	rasterizerState[1] = new RasterizerState();
	rasterizerState[1]->CullMode(D3D11_CULL_NONE);

	depthState[0] = new DepthStencilState();
	depthState[1] = new DepthStencilState();
	depthState[1]->DepthEnable(false);
	depthState[1]->DepthWriteEnable(D3D11_DEPTH_WRITE_MASK_ZERO);

	alphaBlend[0] = new BlendState();
	alphaBlend[1] = new BlendState();
	alphaBlend[1]->BlendEnable(true);
	//alphaBlend[1]->AlphaToCoverageEnable(true);
	alphaBlend[1]->DestBlend(D3D11_BLEND_ONE);
	//alphaBlend[1]->DestBlend(D3D11_BLEND_INV_SRC_ALPHA);
	alphaBlend[1]->SrcBlend(D3D11_BLEND_SRC_ALPHA);
	alphaBlend[1]->BlendOp(D3D11_BLEND_OP_ADD);

	rayleigh2D = new Render2D(values);
	rayleigh2D->Position(0, 100);
	rayleigh2D->Scale(200, 100);

	mie2D = new Render2D(values);
	mie2D->Position(0, 0);
	mie2D->Scale(200, 100);

	starField = new Texture(Textures + L"Starfield.png");
	moonField = new Texture(Textures + L"Moon.tga");
	moonGlowField = new Texture(Textures + L"moonglow.tga" );

	rayleighSampler = new SamplerState();
	rayleighSampler->AddressU(D3D11_TEXTURE_ADDRESS_CLAMP);
	rayleighSampler->AddressV(D3D11_TEXTURE_ADDRESS_CLAMP);
	rayleighSampler->AddressW(D3D11_TEXTURE_ADDRESS_CLAMP);
	rayleighSampler->Filter(D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	mieSampler = new SamplerState();
	mieSampler->AddressU(D3D11_TEXTURE_ADDRESS_CLAMP);
	mieSampler->AddressV(D3D11_TEXTURE_ADDRESS_CLAMP);
	mieSampler->AddressW(D3D11_TEXTURE_ADDRESS_CLAMP);
	mieSampler->Filter(D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	starSampler = new SamplerState();
	starSampler->AddressU(D3D11_TEXTURE_ADDRESS_CLAMP);
	starSampler->AddressV(D3D11_TEXTURE_ADDRESS_CLAMP);
	starSampler->AddressW(D3D11_TEXTURE_ADDRESS_CLAMP);
	starSampler->Filter(D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	moonSampler = new SamplerState();
	moonSampler->AddressU(D3D11_TEXTURE_ADDRESS_WRAP);
	moonSampler->AddressV(D3D11_TEXTURE_ADDRESS_WRAP);
	moonSampler->AddressW(D3D11_TEXTURE_ADDRESS_WRAP);
	moonSampler->Filter(D3D11_FILTER_MIN_MAG_MIP_LINEAR);
}

Sky::~Sky()
{
	for (int i = 0; i < 2; i++)
	{
		SAFE_DELETE(rasterizerState[i]);
		SAFE_DELETE(depthState[i]);
		SAFE_DELETE(alphaBlend[i]);
	}

	SAFE_DELETE_ARRAY(quadVertices);
	SAFE_RELEASE(quadBuffer);

	SAFE_DELETE(scatterBuffer);
	SAFE_DELETE(targetBuffer);
	SAFE_DELETE(worldBuffer);
	SAFE_DELETE(moonBuffer);
	SAFE_RELEASE(vertexBuffer);
	SAFE_RELEASE(indexBuffer);

	SAFE_DELETE(rayleighSampler);		  
	SAFE_DELETE(mieSampler);			  
	SAFE_DELETE(starSampler);
	SAFE_DELETE(moonSampler);
										  
	SAFE_DELETE(moonField);
	SAFE_DELETE(starField);
	SAFE_DELETE(permTex);

	SAFE_DELETE(scatterShader);
	SAFE_DELETE(targetShader);
	SAFE_DELETE(moonShader);
	SAFE_DELETE(cloudShader);

	SAFE_DELETE(mieTarget);
	SAFE_DELETE(rayleighTarget);
}

void Sky::Update()
{
	D3DXMATRIX V;
	D3DXVECTOR3 position;

	values->MainCamera->Position(&position);
	D3DXMatrixTranslation(&V, position.x, position.y, position.z);

	theta += Time::Delta() * timeFactor;
	if (theta > Math::PI * 2.0f) theta -= Math::PI * 2.0f;

	values->GlobalLight->Data.Direction = GetDirection();

	worldBuffer->SetMatrix(V);
}

void Sky::PreRender()
{
	if (prevTheta == theta && prevPhi == phi)
		return;

	mieTarget->Set();
	rayleighTarget->Set();

	ID3D11RenderTargetView* rtvs[2];
	rtvs[0] = rayleighTarget->GetRTV();
	rtvs[1] = mieTarget->GetRTV();

	ID3D11DepthStencilView* dsv;
	dsv = rayleighTarget->GetDSV();

	D3D::Get()->SetRenderTargets(2, rtvs, dsv);

	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	targetBuffer->SetPSBuffer(11);
	targetShader->Render();

	D3D::GetDC()->Draw(6, 0);
}

void Sky::Render()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	D3D::GetDC()->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	D3D::GetDC()->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	scatterShader->Render();
	worldBuffer->SetVSBuffer(1);

	ID3D11ShaderResourceView* srv;
	srv = rayleighTarget->GetSRV();
	D3D::GetDC()->PSSetShaderResources(10, 1, &srv);

	srv = mieTarget->GetSRV();
	D3D::GetDC()->PSSetShaderResources(11, 1, &srv);

	
	rayleighSampler->PSSetSamplers(10);
	mieSampler->PSSetSamplers(11);
	
	starSampler->PSSetSamplers(12);
	starField->SetShaderResource(12);

	scatterBuffer->Data.StarIntensity = values->GlobalLight->Data.Direction.y;
	scatterBuffer->SetPSBuffer(10);

	rasterizerState[1]->RSSetState();
	depthState[1]->OMSetDepthStencilState();
	alphaBlend[1]->OMSetBlendState();
	D3D::GetDC()->DrawIndexed(indexCount, 0, 0);

	DrawMoonGlow();
	DrawCloud();

	depthState[0]->OMSetDepthStencilState();
	rasterizerState[0]->RSSetState();
	alphaBlend[0]->OMSetBlendState();

	rayleigh2D->SRV(rayleighTarget->GetSRV());
	rayleigh2D->Update();
	rayleigh2D->Render();

	mie2D->SRV(mieTarget->GetSRV());
	mie2D->Update();
	mie2D->Render();

	ImGui::SliderFloat("Theta", &theta, 0.0f, Math::PI * 2.0f, "%.5f");
}

void Sky::GenerateSphere()
{
	UINT latitude = domeCount / 2;
	UINT longitude = domeCount;

	vertexCount = longitude * latitude * 2;
	indexCount = (longitude - 1) * (latitude - 1) * 2 * 8;

	VertexTexture* vertices = new VertexTexture[vertexCount];

	UINT index = 0;
	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = Math::PI * j / (latitude - 1);

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);
			vertices[index].Position *= 10.0f; // 크기를 키우려고 임의의 값 곱한거

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)

	for (UINT i = 0; i < longitude; i++)
	{
		float xz = 100.0f * (i / (longitude - 1.0f)) * Math::PI / 180.0f;

		for (UINT j = 0; j < latitude; j++)
		{
			float y = (Math::PI * 2.0f) - (Math::PI * j / (latitude - 1));

			vertices[index].Position.x = sinf(xz) * cosf(y);
			vertices[index].Position.y = cosf(xz);
			vertices[index].Position.z = sinf(xz) * sinf(y);
			vertices[index].Position *= 10.0f; // 크기를 키우려고 임의의 값 곱한거

			vertices[index].Uv.x = 0.5f / (float)longitude + i / (float)longitude;
			vertices[index].Uv.y = 0.5f / (float)latitude + j / (float)latitude;

			index++;
		} // for(j)
	}  // for(i)


	index = 0;
	UINT* indices = new UINT[indexCount * 3];

	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = i * latitude + j;
			indices[index++] = (i + 1) * latitude + j;
			indices[index++] = (i + 1) * latitude + (j + 1);

			indices[index++] = (i + 1) * latitude + (j + 1);
			indices[index++] = i * latitude + (j + 1);
			indices[index++] = i * latitude + j;
		}
	}

	UINT offset = latitude * longitude;
	for (UINT i = 0; i < longitude - 1; i++)
	{
		for (UINT j = 0; j < latitude - 1; j++)
		{
			indices[index++] = offset + i * latitude + j;
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + j;

			indices[index++] = offset + i * latitude + (j + 1);
			indices[index++] = offset + (i + 1) * latitude + (j + 1);
			indices[index++] = offset + i * latitude + j;
		}
	}

	//CreateVertexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * vertexCount;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = vertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &vertexBuffer);
		assert(SUCCEEDED(hr));
	}

	//CreateIndexBuffer
	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(UINT) * indexCount;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = indices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &indexBuffer);
		assert(SUCCEEDED(hr));
	}

	SAFE_DELETE_ARRAY(vertices);
	SAFE_DELETE_ARRAY(indices);
}

void Sky::GenerateQuad()
{
	quadVertices = new VertexTexture[6];

	quadVertices[0].Position = D3DXVECTOR3(-1.0f, -1.0f, 0.0f);
	quadVertices[1].Position = D3DXVECTOR3(-1.0f,  1.0f, 0.0f);
	quadVertices[2].Position = D3DXVECTOR3( 1.0f, -1.0f, 0.0f);
	quadVertices[3].Position = D3DXVECTOR3( 1.0f, -1.0f, 0.0f);
	quadVertices[4].Position = D3DXVECTOR3(-1.0f,  1.0f, 0.0f);
	quadVertices[5].Position = D3DXVECTOR3( 1.0f,  1.0f, 0.0f);

	quadVertices[0].Uv = D3DXVECTOR2(0, 1);
	quadVertices[1].Uv = D3DXVECTOR2(0, 0);
	quadVertices[2].Uv = D3DXVECTOR2(1, 1);
	quadVertices[3].Uv = D3DXVECTOR2(1, 1);
	quadVertices[4].Uv = D3DXVECTOR2(0, 0);
	quadVertices[5].Uv = D3DXVECTOR2(1, 0);

	{
		D3D11_BUFFER_DESC desc = { 0 };
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.ByteWidth = sizeof(VertexTexture) * 6;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		D3D11_SUBRESOURCE_DATA data = { 0 };
		data.pSysMem = quadVertices;

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, &data, &quadBuffer);
		assert(SUCCEEDED(hr));
	}

}

void Sky::GenerateCloud()
{
	int perm[] = { 151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};

	int gradValues[] = {
		1,1,0,
		-1,1,0, 1,-1,0,
		-1,-1,0, 1,0,1,
		-1,0,1, 1,0,-1,
		-1,0,-1, 0,1,1,
		0,-1,1, 0,1,-1,
		0,-1,-1, 1,1,0,
		0,-1,1, -1,1,0,
		0,-1,-1
	};
	//permTex = new Texture(Textures + L"PerlinNoise.png");
	permTex = new Texture(Textures + L"TempTex256.png");

	vector<D3DXCOLOR> pixels;
	for (int i = 0; i<256; i++)
	{
		for (int j = 0; j<256; j++)
		{
			//int offset = (i * 256 + j);
			int value = perm[(j + perm[i]) & 0xFF];
			D3DXCOLOR color;
			color.r = (float)(gradValues[value & 0x0F] * 64 + 64);
			color.g = (float)(gradValues[value & 0x0F + 1] * 64 + 64);
			color.b = (float)(gradValues[value & 0x0F + 2] * 64 + 64);
			color.a = (float)value;
			pixels.push_back(color);
		}
	}
	permTex->WritePixels(DXGI_FORMAT_R8G8B8A8_UNORM, pixels, true);
}

float Sky::GetStarIntensity()
{
	return 0.0f;
}

void Sky::DrawMoonGlow()
{
	D3DXVECTOR3 camPos;
	D3DXMATRIX sc, rX, rY, dist, cam, matF;
	values->MainCamera->Position(&camPos);
	D3DXMatrixScaling(&sc, 50, 50, 1);
	D3DXMatrixRotationX(&rX, -theta - (Math::PI * 0.5f));
	D3DXMatrixRotationY(&rY, phi - (Math::PI * 0.5f));
	D3DXMatrixTranslation(&cam, camPos.x, camPos.y, camPos.z);

	if (theta < Math::PI * 0.5f || theta > Math::PI * 1.5f)
		scatterBuffer->Data.MoonAlpha = (float)abs(sinf(theta + (float)Math::PI / 2.0f));
	else
		scatterBuffer->Data.MoonAlpha = 0.0f;

	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;

	{
		// Moon
		D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3DXMatrixTranslation(&dist,
			values->GlobalLight->Data.Direction.x * 815.0f,
			values->GlobalLight->Data.Direction.y * 815.0f,
			values->GlobalLight->Data.Direction.z * 815.0f);
		matF = sc*rX*rY*dist*cam;

		moonBuffer->SetMatrix(matF);
		moonBuffer->SetVSBuffer(1);
		scatterBuffer->SetPSBuffer(10);

		moonField->SetShaderResource(10);
		moonSampler->PSSetSamplers(10);

		moonShader->Render();
		D3D::GetDC()->Draw(6, 0);
	}

	{
		// Moon Glow
		D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
		D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		D3DXMatrixScaling(&sc, 150, 150, 1);
		D3DXMatrixTranslation(&dist,
			values->GlobalLight->Data.Direction.x * 805.0f,
			values->GlobalLight->Data.Direction.y * 805.0f,
			values->GlobalLight->Data.Direction.z * 805.0f);
		matF = sc*rX*rY*dist*cam;

		moonBuffer->SetMatrix(matF);
		moonBuffer->SetVSBuffer(1);
		scatterBuffer->SetPSBuffer(10);

		moonGlowField->SetShaderResource(10);
		moonSampler->PSSetSamplers(10);

		moonShader->Render();
		D3D::GetDC()->Draw(6, 0);
	}
}

void Sky::DrawCloud()
{
	UINT stride = sizeof(VertexTexture);
	UINT offset = 0;
	D3D::GetDC()->IASetVertexBuffers(0, 1, &quadBuffer, &stride, &offset);
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DXVECTOR3 camPos;
	D3DXMATRIX sc, rX, rY, dist, cam, matF;

	values->MainCamera->Position(&camPos);

	D3DXMatrixScaling(&sc, 900, 900, 1);
	D3DXMatrixTranslation(&dist, 0, 0, -200.0f);
	D3DXMatrixRotationX(&rX, Math::PI * 0.5f);
	D3DXMatrixTranslation(&cam, camPos.x, camPos.y, camPos.z);
	matF = sc * dist * rX * cam;

	worldBuffer->SetMatrix(matF);
	worldBuffer->SetVSBuffer(1);

	cloudBuffer->Data.NumTiles = 16.25f;
	scatterBuffer->Data.MoonAlpha = Time::Get()->Running() / 8.0f;
	cloudBuffer->SetVSBuffer(10);
	scatterBuffer->SetPSBuffer(10);

	permTex->SetShaderResource(10);
	moonSampler->PSSetSamplers(10);

	cloudShader->Render();
	D3D::GetDC()->Draw(6, 0);

	ImGui::SliderFloat("Time", &timeFactor, 0, 5);
	ImGui::DragFloat("Sun Angle", &phi, 1, 0, 90);
}


D3DXVECTOR3 Sky::GetDirection()
{
	float y = (float)cosf(theta);
	float x = (float)(sinf(theta));
	float z = (float)(sinf(Math::ToRadian(phi)));

	return D3DXVECTOR3(x, y, z);
}
