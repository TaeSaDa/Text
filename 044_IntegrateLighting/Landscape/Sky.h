#pragma once

class Sky
{
public:
	Sky(ExecuteValues* values);
	~Sky();

	void Update();
	void PreRender();
	void Render();

private:
	void GenerateSphere();
	void GenerateQuad();
	void GenerateCloud();
	D3DXVECTOR3 GetDirection();

	float GetStarIntensity();

	void DrawMoonGlow();
	void DrawCloud();

private:
	ExecuteValues* values;

	bool realTime;

	float theta, phi;
	float prevTheta, prevPhi;

	RenderTarget* mieTarget, *rayleighTarget;
	Shader* scatterShader;
	Shader* targetShader;
	Shader* moonShader;
	Shader* cloudShader;

	UINT vertexCount, indexCount;
	UINT radius, slices, stacks;
	UINT domeCount;//구의 정점이 몇개가 될지

	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	VertexTexture* quadVertices;
	ID3D11Buffer* quadBuffer;

	Render2D* rayleigh2D;
	Render2D* mie2D;

	WorldBuffer* worldBuffer;
	WorldBuffer* moonBuffer;

	RasterizerState* rasterizerState[2];
	DepthStencilState* depthState[2];
	BlendState* alphaBlend[2];

	Texture* starField;
	Texture* moonField;
	Texture* moonglowField;
	Texture* moonGlowField;
	Texture* permTex;

	SamplerState* rayleighSampler;
	SamplerState* mieSampler;
	SamplerState* starSampler;
	SamplerState* moonSampler;
	SamplerState* cloudSampler;

private:
	float timeFactor;
private:
	class ScatterBuffer : public ShaderBuffer
	{
	public:
		ScatterBuffer() : ShaderBuffer(&Data, sizeof(Data))
		{
			Data.StarIntensity = 0.0f;
			Data.MoonAlpha = 0.0f;
		}

		struct Struct
		{
			float StarIntensity;
			float MoonAlpha;
			float Padding[2];
		} Data;
	};
	ScatterBuffer* scatterBuffer;

	class TargetBuffer : public ShaderBuffer
	{
	public:
		TargetBuffer() : ShaderBuffer(&Data, sizeof(Data))
		{
			Data.WaveLength = D3DXVECTOR3(0.65f, 0.57f, 0.475f);

			Data.InvWaveLength.x = 1.0f / powf(Data.WaveLength.x, 4);
			Data.InvWaveLength.y = 1.0f / powf(Data.WaveLength.y, 4);
			Data.InvWaveLength.z = 1.0f / powf(Data.WaveLength.z, 4);

			Data.WaveLengthMie.x = powf(Data.WaveLength.x, -0.84f);
			Data.WaveLengthMie.y = powf(Data.WaveLength.y, -0.84f);
			Data.WaveLengthMie.z = powf(Data.WaveLength.z, -0.84f);

			Data.SampleCount = 20;
		}

		struct Struct
		{
			D3DXVECTOR3 WaveLength;
			int SampleCount;
			D3DXVECTOR3 InvWaveLength;
			float Padding;
			D3DXVECTOR3 WaveLengthMie;
			float Padding2;
		} Data;
	};
	TargetBuffer* targetBuffer;

	class CloudBuffer : public ShaderBuffer
	{
	public:
		CloudBuffer() : ShaderBuffer(&Data, sizeof(Data))
		{
			Data.NumTiles = 0.0f;
		}

		struct Struct
		{
			float NumTiles;
			float padding[3];

		} Data;
	};

	CloudBuffer * cloudBuffer;
};