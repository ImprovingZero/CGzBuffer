#pragma once
#include"common.h"
#include"Model.h"
#include"camera.h"
#include"Polylist.h"
#include"Qtree.h"

class QtreeNode;

class ZBuffer
{
private:
	void refreshLine(); 
	void refreshOutput();

	void scan(int y);
	void scanInterval(int y);
public:
	std::vector<std::vector<int>> _output;
	std::vector<std::vector<double>> _depthFull;
	std::vector<double> _depth;
	std::vector<int> _buffer;
	PolyList* _pll;
	QtreeNode* _Qtree;
	std::vector<std::vector<QtreeNode*>> _QtPtr;

	ZBuffer(PolyList* pll) : _pll(pll) 
	{
		_output.clear();
		_depthFull.clear();
		for (int i = 0; i <= V_PIX_NUM; i++)
			_output.push_back(std::vector<int>(U_PIX_NUM));
		for (int i = 0; i <= U_PIX_NUM; i++)
		{
			_depth.push_back(-DBL_MAX);
			_buffer.push_back(-1);
		}
	};
	void generateScan();
	void generateScanInter();
	void generateNaive();
	void generateQtree();
	

	void drawCharSrc();
};

