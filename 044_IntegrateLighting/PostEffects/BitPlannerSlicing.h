#pragma once

class BitPlannerSlicing : public Render2D
{
public:
	BitPlannerSlicing(ExecuteValues* values);
	~BitPlannerSlicing();

	void Render();

private:
	class Buffer : public ShaderBuffer
	{
	public:
		Buffer() : ShaderBuffer(&Data, sizeof(Data))
		{
			Data.Bit = 0;

			//Data.Color = D3DXCOLOR(1, 1, 1, 1);
		}

		struct Struct
		{
			int Bit;

			float Padding[3];
		} Data;
	};
	Buffer* buffer;
};