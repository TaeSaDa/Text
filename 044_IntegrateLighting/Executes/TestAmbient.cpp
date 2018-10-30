#include "stdafx.h"
#include "TestAmbient.h"
#include "../Objects/MeshPlane.h"
#include "../Objects/MeshCube.h"
#include "../Objects/MeshSphere.h"
#include "../Objects/MeshBunny.h"

TestAmbient::TestAmbient(ExecuteValues * values)
	: Execute(values)
{
	shader = new Shader(Shaders + L"046_Lightning.hlsl");
	
	

	plane = new MeshPlane();
	plane->Scale(10, 1, 10);
	plane->SetShader(shader);
	plane->SetDiffuse(1, 1, 1);

	cube = new MeshCube();
	cube->Position(-10, 1.5f, 0);
	cube->SetShader(shader);
	cube->SetDiffuse(0, 1, 0);
	cube->Scale(3, 3, 3);


	sphere = new MeshSphere();
	sphere->Position(10, 1.5f, 0);
	sphere->SetShader(shader);
	sphere->SetDiffuse(0, 0, 1);
	sphere->Scale(3, 3, 3);

	sphere2 = new MeshSphere();
	sphere2->Position(12, 1.5f, 10);
	sphere2->SetShader(shader);
	sphere2->SetDiffuse(1, 1, 1);
	sphere2->Scale(3, 3, 3);

	bunny = new MeshBunny();
	bunny->Position(0, 5, 0);
	bunny->SetShader(shader);
	bunny->SetDiffuse(1, 1, 1);
	bunny->Scale(0.02f, 0.02f, 0.02f);
	bunny->SetDiffuseMap(Textures + L"White.png");

	//CreateLightBuffer
	{
		D3D11_BUFFER_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_BUFFER_DESC));

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D10_CPU_ACCESS_WRITE;
		desc.ByteWidth = sizeof(LightData);

		HRESULT hr = D3D::GetDevice()->CreateBuffer(&desc, NULL, &lightBuffer);
		assert(SUCCEEDED(hr));
	}
	//D3D::GetDevice()->CreateBuffer();
}

TestAmbient::~TestAmbient()
{
	

	SAFE_DELETE(sphere);
	SAFE_DELETE(sphere2);

	SAFE_DELETE(cube);
	SAFE_DELETE(plane);
	SAFE_DELETE(bunny);
}

void TestAmbient::Update()
{
	plane->Update();
	cube->Update();
	sphere->Update();
	sphere2->Update();
	bunny->Update();
}

void TestAmbient::PostRender()
{	
}

void TestAmbient::Render()
{
	ImGui::Separator();
	ImGui::Text("Ambient");
	ImGui::SliderFloat3("AmbientFoolr", (float*)&ambientBuffer->Data.Floor, 0, 1);
	ImGui::SliderFloat3("AmbientCeil", (float*)&ambientBuffer->Data.Ceil, 0, 1);
	ImGui::ColorEdit3("DirectionColor", (float*)&ambientBuffer->Data.color);

	ImGui::Separator();
	ImGui::Text("Specular");
	ImGui::SliderFloat("SpecularExp", (float*)&specularBuffer->Data.Exp, 1, 100);
	ImGui::SliderFloat("SpecularIntensity", (float*)&specularBuffer->Data.Intensity, 0, 10);

	ImGui::Separator();
	ImGui::Text("PointLight");
	ImGui::SliderFloat3("PointLight Position", (float*)&pontLightBuffer->Data.Position, 50, -50);
	ImGui::SliderFloat("PointLight Range", (float*)&pontLightBuffer->Data.Range, 0, 10);
	ImGui::ColorEdit3("PointLight Color", (float*)&pontLightBuffer->Data.Color);

	ImGui::Separator();
	ImGui::Text("SpotLight");
	ImGui::SliderFloat3("SpotLight Position", (float*)&spotLightBuffer->Data.Position, -50, 50);
	ImGui::SliderFloat("SpotLight Range", (float*)&spotLightBuffer->Data.Range, 0, 100);
	ImGui::ColorEdit3("SpotLight Color", (float*)&spotLightBuffer->Data.Color);
	ImGui::SliderFloat3("SpotLight Direction", (float*)&spotLightBuffer->Data.Direction, -1, 1);
	ImGui::SliderFloat("SpotLight Outer", (float*)&spotLightBuffer->Data.Outer, 1, 90);
	ImGui::SliderFloat("SpotLight Inner", (float*)&spotLightBuffer->Data.Inner, 1, 100);

	ImGui::Separator();

	ImGui::Text("Capsule Light");
	ImGui::SliderFloat3("CapsuleLight Position", (float*)&capsuleLightBuffer->Data.Position, -50, 50);
	ImGui::SliderFloat("CapsuleLight Range", (float*)&capsuleLightBuffer->Data.Range, 1, 100);
	ImGui::ColorEdit3("CapsuleLight Color", (float*)&capsuleLightBuffer->Data.Color);
	ImGui::SliderFloat3("CapsuleLight Direction", (float*)&capsuleLightBuffer->Data.Direction, -1, 1);
	ImGui::SliderFloat("CapsuleLight Length", (float*)&capsuleLightBuffer->Data.Length, 0, 100);

	ImGui::Separator();
	
	ambientBuffer->SetPSBuffer(10);
	specularBuffer->SetPSBuffer(11);
	pontLightBuffer->SetPSBuffer(12);
	spotLightBuffer->SetPSBuffer(13);
	capsuleLightBuffer->SetPSBuffer(9);

	plane->Render();
	sphere->Render();
	sphere2->Render();
	cube->Render();
	bunny->Render();
}

void TestAmbient::PreRender()
{

}