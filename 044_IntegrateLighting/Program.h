#pragma once

class Program
{
public:
	Program();
	~Program();

	void Update();

	void PreRender();
	void Render();
	void PostRender();

	void ResizeScreen();

private:
	int CExecute;

	ExecuteValues* values;

	vector<class Execute *> executes;
};