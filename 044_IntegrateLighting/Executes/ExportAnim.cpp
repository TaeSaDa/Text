#include "stdafx.h"
#include "ExportAnim.h"
#include "../Fbx/Exporter.h"

ExportAnim::ExportAnim(ExecuteValues * values)
	: Execute(values)
{
	Fbx::Exporter* exporter = NULL;

	////Kachujin Mesh
	//exporter = new Fbx::Exporter(Assets + L"Monster/Mesh.fbx");
	//exporter->ExportMaterial(Models + L"Monster/", L"Monster.material");
	//exporter->ExportMesh(Models + L"Monster/", L"Monster.mesh");
	//SAFE_DELETE(exporter);

	//exporter = new Fbx::Exporter(Assets + L"Monster/Idle.fbx");
	//exporter->ExportAnimation(Models + L"Monster/", L"Idle.anim", 0);
	//SAFE_DELETE(exporter);

	////Taunt
	//exporter = new Fbx::Exporter(Assets + L"Monster/Punch.fbx");
	//exporter->ExportAnimation(Models + L"Monster/", L"Punch.anim", 0);
	//SAFE_DELETE(exporter);
	////Running
	//exporter = new Fbx::Exporter(Assets + L"Monster/Walking.fbx");
	//exporter->ExportAnimation(Models + L"Monster/", L"Walking.anim", 0);
	//SAFE_DELETE(exporter);

	////Running
	//exporter = new Fbx::Exporter(Assets + L"Monster/Hit.fbx");
	//exporter->ExportAnimation(Models + L"Monster/", L"Hit.anim", 0);
	//SAFE_DELETE(exporter);

	////Running
	//exporter = new Fbx::Exporter(Assets + L"Monster/Dying.fbx");
	//exporter->ExportAnimation(Models + L"Monster/", L"Dying.anim", 0);
	//SAFE_DELETE(exporter);
}

ExportAnim::~ExportAnim()
{
	
}

void ExportAnim::Update()
{
	
}

void ExportAnim::PreRender()
{
	
}

void ExportAnim::Render()
{
	
}

void ExportAnim::PostRender()
{
	
}

void ExportAnim::ResizeScreen()
{
}

