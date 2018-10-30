#include "stdafx.h"
#include "Program.h"
#include "./Viewer/Freedom.h"

#include "./Executes/Export.h"
//#include "./Executes/TestLandscape.h"
//#include "./Executes/TestBlend.h"
//#include "./Executes/TestMultiTexture.h"
#include "./Executes/TestScattering.h"
#include "./Executes/TestAmbient.h"


Program::Program()
	: CExecute(-1)
{
	int a = 10 / 0;

	D3DDesc desc;
	D3D::GetDesc(&desc);

	values = new ExecuteValues();
	values->ViewProjection = new ViewProjectionBuffer();
	values->GlobalLight = new LightBuffer();
	values->Perspective = new Perspective(desc.Width, desc.Height);
	values->Viewport = new Viewport(desc.Width, desc.Height);

	values->MainCamera = new Freedom(75);
	values->MainCamera->RotationDegree(-5, 268);
	values->MainCamera->Position(415, 70, 223);

	values->GlobalLight->Data.Direction = D3DXVECTOR3(-1, -1, 1);

	executes.push_back(new Export(values));
	
	executes.push_back(new TestAmbient(values));
	executes.push_back(new TestScattering(values));
	//executes.push_back(new TestBlend(values));

}

Program::~Program()
{
	for (Execute* exe : executes)
		SAFE_DELETE(exe);

	SAFE_DELETE(values->GlobalLight);
	SAFE_DELETE(values->ViewProjection);
	SAFE_DELETE(values->Perspective);
	SAFE_DELETE(values->Viewport);
	SAFE_DELETE(values);
}

void Program::Update()
{
	values->MainCamera->Update();

	/*for (Execute* exe : executes)
		exe->Update();*/

	if (CExecute != -1)
		executes[CExecute]->Update();
}

void Program::PreRender()
{
	D3DXMATRIX view, projection;
	values->MainCamera->Matrix(&view);
	values->Perspective->GetMatrix(&projection);

	values->ViewProjection->SetView(view);
	values->ViewProjection->SetProjection(projection);
	values->ViewProjection->SetVSBuffer(0);

	/*for (Execute* exe : executes)
		exe->PreRender();*/
	if (CExecute != -1)
		executes[CExecute]->PreRender();
}

void Program::Render()
{
	D3DXMATRIX view, projection;
	values->MainCamera->Matrix(&view);
	values->Perspective->GetMatrix(&projection);

	values->ViewProjection->SetView(view);
	values->ViewProjection->SetProjection(projection);
	values->ViewProjection->SetVSBuffer(0);

	values->GlobalLight->SetPSBuffer(0);
	values->Viewport->RSSetViewport();

	/*for (Execute* exe : executes)
		exe->Render();*/

	if (CExecute != -1)
		executes[CExecute]->Render();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("Mode"))
		{
			if (ImGui::MenuItem("TestAmbient"))
			{
				CExecute = 1;
			}
			if (ImGui::MenuItem("TestScattering"))
			{
				CExecute = 2;
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
	ImGui::Separator();
	ImGui::SliderFloat3("Direction", (float *)&values->GlobalLight->Data.Direction, -1, 1);
	ImGui::Separator();
}

void Program::PostRender()
{
	/*for (Execute* exe : executes)
		exe->PostRender();*/
	if (CExecute != -1)
		executes[CExecute]->PostRender();

	wstring str;
	RECT rect = { 0,670,800,800 };

	str = String::Format(L"FPS : %.0f", ImGui::GetIO().Framerate);
	DirectWrite::RenderText(str, rect, 12);

	rect.top += 12;
	D3DXVECTOR3 vec;
	values->MainCamera->Position(&vec);
	str = String::Format(L"CameraPos : %.0f, %.0f, %.0f", vec.x, vec.y, vec.z);
	DirectWrite::RenderText(str, rect, 12);

	rect.top += 12;
	D3DXVECTOR2 rot;
	values->MainCamera->RotationDegree(&rot);
	str = String::Format(L"CameraRot : %.0f, %.0f", rot.x, rot.y);
	DirectWrite::RenderText(str, rect, 12);

}
	

void Program::ResizeScreen()
{
	D3DDesc desc;
	D3D::GetDesc(&desc);

	values->Perspective->Set(desc.Width, desc.Height);
	values->Viewport->Set(desc.Width, desc.Height);

	for (Execute* exe : executes)
		exe->ResizeScreen();
}