#include "stdafx.h"
#include "MeshBunny.h"

MeshBunny::MeshBunny()
	: GameModel(Materials + L"Meshes/", L"StanfordBunny.material", Models + L"Meshes/", L"StanfordBunny.mesh")
{
	
}

MeshBunny::~MeshBunny()
{
	
}

void MeshBunny::Update()
{
	__super::Update();
}

void MeshBunny::Render()
{
	__super::Render();
}
