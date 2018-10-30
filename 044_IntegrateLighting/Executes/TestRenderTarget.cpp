#include "stdafx.h"
#include "TestRenderTarget.h"
#include "./Landscape/Sky.h"
#include "./Objects/MeshPlane.h"
#include "./Objects/MeshSphere.h"
#include "./Objects/MeshCube.h"
#include "./Objects/PointLight.h"

#include "./PostEffects/Grayscale.h"
#include "./PostEffects/Blur.h"
#include "./PostEffects/Mosaic.h"
#include "./PostEffects/ColorShift.h"
#include "./PostEffects/Gamma.h"
#include "./PostEffects/BitPlannerSlicing.h"
#include "./PostEffects/Embossing.h"
//#include "./PostEffects/Sharpening.h"
#include "./PostEffects/GaussianBlur.h"


TestRenderTarget::TestRenderTarget(ExecuteValues * values)
	: Execute(values)
{
	sky = new Sky(values);
	shader = new Shader(Shaders + L"033_NormalMap.hlsl");

	/*renderTarget[0] = new RenderTarget(values);
	renderTarget[1] = new RenderTarget(values);
	renderTarget[2] = new RenderTarget(values);*/
	renderTarget = new RenderTarget(values);

	pointLight = new PointLight();
	{
		PointLight::Desc desc;
		desc.Color = D3DXVECTOR3(1, 0, 0);
		desc.Intensity = 3;
		desc.Position = D3DXVECTOR3(3, 0, 0);
		desc.Range = 5;

		pointLight->Add(desc);
	}
	//Create Render2D
	{
		D3DDesc desc;
		D3D::GetDesc(&desc);

		render2D = new Render2D(values);
		render2D->Scale(desc.Width * 0.5f, desc.Height);

		postEffectCount[0] = new Grayscale(values);
		postEffectCount[1] = new Blur(values);
		postEffectCount[2] = new Mosaic(values);
		postEffectCount[3] = new ColorShift(values);
		postEffectCount[4] = new Gamma(values);
		postEffectCount[5] = new BitPlannerSlicing(values);
		postEffectCount[6] = new Embossing(values);
		postEffectCount[7] = new GaussianBlur(values, "PS_X");
		postEffectCount[8] = new GaussianBlur(values, "PS_Y");
		postEffectCount[9] = new GaussianBlur(values, "PS_XY");

		//postEffectCount[7] = new Sharpening(values);

		for (int i = 0; i < 10; i++)
		{
			postEffectCount[i]->Position(desc.Width * 0.5f, 0);
			postEffectCount[i]->Scale(desc.Width * 0.5f, desc.Height);
		}
		postEffect = postEffectCount[0];
		Effect = 0;
	}

	plane = new MeshPlane();
	plane->Scale(3, 3, 3);
	plane->SetShader(shader);
	plane->SetDiffuse(1, 1, 1, 1);
	plane->SetDiffuseMap(Textures + L"Floor.png");
	plane->SetNormalMap(Textures + L"Floor_normal.png");

	sphere = new MeshSphere();
	sphere->Scale(3, 3, 3);
	sphere->Position(7.0f, 1.5f, 0.0f);
	sphere->SetShader(shader);
	sphere->SetDiffuse(1, 1, 1, 1);
	sphere->SetDiffuseMap(Textures + L"Wall.png");
	sphere->SetNormalMap(Textures + L"Wall_normal.png");

	cube[0] = new MeshCube();
	cube[0]->Scale(2, 2, 2);
	cube[0]->Position(-5.0f, 1.0f, 0.0f);
	cube[0]->SetShader(shader);
	cube[0]->SetDiffuse(1, 1, 1, 1);
	cube[0]->SetDiffuseMap(Textures + L"Bricks.png");
	cube[0]->SetNormalMap(Textures + L"Bricks_normal.png");

	cube[1] = new MeshCube();
	cube[1]->Scale(2, 2, 2);
	cube[1]->Position(0.0f, 1.0f, 0.0f);
	cube[1]->SetShader(shader);
	cube[1]->SetDiffuse(1, 1, 1, 1);
	cube[1]->SetDiffuseMap(Textures + L"Stones.png");
	cube[1]->SetNormalMap(Textures + L"Stones_normal.png");
}

TestRenderTarget::~TestRenderTarget()
{
	
}

void TestRenderTarget::Update()
{
	sky->Update();

	pointLight->Update();

	plane->Update();
	sphere->Update();
	cube[0]->Update();
	cube[1]->Update();
}

void TestRenderTarget::PreRender()
{
	renderTarget->Set();

	sky->Render();
	plane->Render();
	sphere->Render();
	cube[0]->Render();
	cube[1]->Render();

	renderTarget->Set();

	postEffectCount[7]->Update();
	postEffectCount[7]->SRV(renderTarget->GetSRV());
	postEffectCount[7]->Render();

	renderTarget->Set();

	postEffectCount[8]->Update();
	postEffectCount[8]->SRV(renderTarget->GetSRV());
	postEffectCount[8]->Render();

	/*if (Keyboard::Get()->Down(VK_SPACE))
		renderTarget->SaveRtvTexture(L"Test.png");*/
	
}

void TestRenderTarget::Render()
{
	ImGui::Separator();
	ImGui::SliderInt("PostEffect", &Effect, 0, 9);
	ImGui::Separator();
	postEffect = postEffectCount[Effect];
}

void TestRenderTarget::PostRender()
{
	render2D->Update();
	render2D->SRV(renderTarget->GetSRV());
	render2D->Render();

	postEffect->Update();
	postEffect->SRV(renderTarget->GetSRV());
	postEffect->Render();

	/*postEffectCount[9]->Update();
	postEffectCount[9]->SRV(postEffectCount[7]->SRV);
	postEffectCount[9]->Render();

	postEffectCount[9]->Update();
	postEffectCount[9]->SRV(postEffectCount[8]->SRV);
	postEffectCount[9]->Render();

	postEffectCount[9]->Update();
	postEffectCount[9]->SRV(postEffectCount[9]->SRV);
	postEffectCount[9]->Render();*/
}

