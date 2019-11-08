#pragma once
#include"common.h"
#include"Model.h"
#include"camera.h"
#include"Polylist.h"

class ZBuffer
{
private:
	void refreshLine(); 
	void refreshOutput();
public:
	std::vector<std::vector<int>> _output;
	std::vector<double> _depth;
	std::vector<int> _buffer;
	PolyList* _pll;

	ZBuffer(PolyList* pll) : _pll(pll) 
	{
		_output.clear();
		for (int i = 0; i < V_PIX_NUM; i++)
			_output.push_back(std::vector<int>(U_PIX_NUM));
		for (int i = 0; i < U_PIX_NUM; i++)
		{
			_depth.push_back(FLT_MAX);
			_buffer.push_back(-1);
		}
	};
	void generate();
	void scan(int y);
	void scanInterval(int y);

	void drawCharSrc();
};

