#include "ZBuffer.h"


void ZBuffer::refreshLine()
{
	for (int i = 0; i < U_PIX_NUM; i++)
	{
		_depth[i] = -DBL_MAX;
		_buffer[i] = -1;
	}
}

void ZBuffer::refreshOutput()
{
	for (int i = 0; i < V_PIX_NUM; i++)
		for (int j = 0; j < U_PIX_NUM; j++)
			_output[i][j] = -1;
}

double activeTimeUse = 0;
double deleteTimeUse = 0;
double updateTimeUse = 0;
double drawTimeUse = 0;

void ZBuffer::generateScan()
{
	_pll->refreshList();
	clock_t start = clock();
	
	for (int i = 0; i <= V_PIX_NUM; i++)
	{
		_depthFull.push_back(std::vector<double>(0));
		for (int j = 0; j <= U_PIX_NUM; j++)
		{
			_depthFull[i].push_back(-DBL_MAX);
			_output[i][j] = -1;
		}
	}

	for (int i = V_PIX_NUM - 1; i >= 0; i--)
	{
		refreshLine();
		scan(i);
		for (int j = 0; j < U_PIX_NUM; j++)
		{
			_output[i][j] = _buffer[j];
		}
	}
	clock_t end = clock();
	std::cout << "Generate scan method --- Totol time use：" << end - start << "ms" << std::endl;	
	_over = 1;
}

void ZBuffer::generateScanWithoutClassEdge()
{
	_pll->refreshList();
	clock_t start = clock();

	for (int i = V_PIX_NUM - 1; i >= 0; i--)
	{
		refreshLine();
		scanWithoutClassifiedEdges(i);
		for (int j = 0; j < U_PIX_NUM; j++)
		{
			_output[i][j] = _buffer[j];
		}
	}
	clock_t end = clock();
	std::cout << "Generate scan method without classified edges --- Totol time use：" << end - start << "ms" << std::endl;
	_over = 1;
}

void ZBuffer::generateScanInter()
{
	_pll->refreshList();
	clock_t start = clock();

	for (int i = V_PIX_NUM - 1; i >= 0; i--)
	{
		refreshLine();
		scanInterval(i);
		for (int j = 0; j < U_PIX_NUM; j++)
		{
			_output[i][j] = _buffer[j];
		}
	}
	clock_t end = clock();
	std::cout << "Generate with interval scan method --- Totol time use：" << end - start << "ms" << std::endl;
	std::cout << "Active Polygon part time use: " << activeTimeUse << "ms" << std::endl;
	std::cout << "Delete part time use: " << deleteTimeUse << "ms" << std::endl;
	std::cout << "Update part time use: " << updateTimeUse << "ms" << std::endl;
	std::cout << "Draw part time use: " << drawTimeUse << "ms" << std::endl;
	_over = 1;
}

void ZBuffer::generateNaive()
{
	_pll->refreshNaive();
	clock_t start = clock();

	for (int i = 0; i <= V_PIX_NUM; i++)
	{
		_depthFull.push_back(std::vector<double>(0));
		for (int j = 0; j <= U_PIX_NUM; j++)
		{
			_depthFull[i].push_back(-DBL_MAX);
			_output[i][j] = -1;
		}
	}
	_pll->rastrizeTri(_output, _depthFull);

	clock_t end = clock();
	std::cout << "Generate with naive method --- Totol time use：" << end - start << "ms" << std::endl;
	_over = 1;
}

void ZBuffer::generateQtree()
{
	using namespace std;
	_pll->refreshNaive();
	cout << "start qtTree init" << endl;

	_QtPtr.clear();
	for (int i = 0; i <= V_PIX_NUM; i++)
	{
		_depthFull.push_back(std::vector<double>(0));
		_QtPtr.push_back(std::vector<QtreeNode*>(0));
		for (int j = 0; j <= U_PIX_NUM; j++)
		{
			_depthFull[i].push_back(-DBL_MAX);
			_output[i][j] = -1;
			_QtPtr[i].push_back(nullptr);
		}
	}
	_Qtree = new QtreeNode(U_PIX_NUM, V_PIX_NUM, vec2i(0, 0), nullptr,_QtPtr);
	
	
	for (int i=0;i<U_PIX_NUM;i++)
		for (int j = 0; j < V_PIX_NUM; j++)
		{
			if (_QtPtr[j][i] == nullptr)
			{
				std::cout << "warnning!!" << std::endl;
			}
		}

	
	
	cout << "finish qtTree init" << endl;
	clock_t start = clock();
	_pll->rastrizeTriQtree(_output, _depthFull, _Qtree, _QtPtr);

	clock_t end = clock();
	std::cout << "Generate with QTree --- Totol time use：" << end - start << "ms" << std::endl;
	_over = 1;
}

void ZBuffer::generateQtreeComplete()
{
	using namespace std;
	_pll->refreshOctree();
	cout << "start qtTree init" << endl;
	

	for (int i = 0; i <= V_PIX_NUM; i++)
	{
		_depthFull.push_back(std::vector<double>(0));
		_QtPtr.push_back(std::vector<QtreeNode*>(0));
		for (int j = 0; j <= U_PIX_NUM; j++)
		{
			_depthFull[i].push_back(-DBL_MAX);
			_output[i][j] = -1;
			_QtPtr[i].push_back(nullptr);
			
		}
	}
	_Qtree = new QtreeNode(U_PIX_NUM, V_PIX_NUM, vec2i(0, 0), nullptr, _QtPtr);

	cout << "finish qtTree init" << endl;
	Octree* oct = _pll->_oct;

	clock_t start = clock();
	_pll->rastrizeTriQtreeComp(_output, _depthFull, _Qtree, _QtPtr);
	clock_t end = clock();

	std::cout << "Generate with QTree Complete --- Totol time use：" << end - start << "ms" << std::endl;
	_over = 1;
}

void ZBuffer::generateFineQtree()
{
	using namespace std;
	_pll->refreshNaive();
	cout << "start qtTree init" << endl;
	_Qtree = new QtreeNode(U_PIX_NUM, V_PIX_NUM, vec2i(0, 0), nullptr);

	//_QtPtr.clear();
	clock_t start = clock();

	for (int i = 0; i <= V_PIX_NUM; i++)
	{
		_depthFull.push_back(std::vector<double>(0));
		//_QtPtr.push_back(std::vector<QtreeNode*>(0));
		for (int j = 0; j <= U_PIX_NUM; j++)
		{
			_depthFull[i].push_back(-DBL_MAX);
			_output[i][j] = -1;
			//_QtPtr[i].push_back(nullptr);
		}
	}
	cout << "finish qtTree init" << endl;
	_pll->rastrizeTriQtree(_output, _depthFull, _Qtree, _QtPtr);

	clock_t end = clock();
	std::cout << "Generate with QTree --- Totol time use：" << end - start << "ms" << std::endl;
	_over = 1;
}

void ZBuffer::scan(int y)
{
	//std::cout << "-------Scan: " << y << " ---------\n";
	if (y % 100 == 0)
	{
		std::cout << "Scanning: " << float(V_PIX_NUM-y) / V_PIX_NUM * 100 << " %\n";
	}
	PolyList* cls = _pll;
	ActiveList* act = _pll->_actList;
	//维护：激活该线上的分类多边形； 到尾端的删掉/维护
	//std::cout << "start ActiveP" << std::endl;
	_pll->activeP(y);
	//std::cout << "start delActiveP" << std::endl;
	act->delActiveP(y);
	//std::cout << "start updateActive" << std::endl;
	act->updataActiveE(y);
	
	//绘制
	//std::cout << "start draw" << std::endl;
	act->draw(y,_depth,_buffer);

	//维护dy
	act->decDy();
	
}

void ZBuffer::scanInterval(int y)
{
	//std::cout << "-------Scan: " << y << " ---------\n";
	if (y % 100 == 0)
	{
		std::cout << "Scanning: " << float(V_PIX_NUM - y) / V_PIX_NUM * 100 << " %\n";
	}
	PolyList* cls = _pll;
	ActiveList* act = _pll->_actList;
	//维护：激活该线上的分类多边形； 到尾端的删掉/维护
	//std::cout << "start ActiveP" << std::endl;
	clock_t start = clock();
	_pll->activePinter(y);
	clock_t end = clock();
	activeTimeUse += (end - start);

	//std::cout << "start updateActive" << std::endl;
	start = clock();
	act->updateActiveEInter(y);
	end = clock();
	updateTimeUse += (end - start);

	//std::cout << "start delActiveP" << std::endl;
	start = clock();
	act->delActiveP(y);
	end = clock();
	deleteTimeUse += (end - start);
	
	//绘制
	//std::cout << "start draw" << std::endl;
	start = clock();
	act->drawInter(y, _depth, _buffer);
	end = clock();
	drawTimeUse += (end - start);

	//维护dy
	act->decDyInter();

	//if (act->_actedgeInter.size()!=0) system("pause");

	//std::cout << "active Ply num: " << act->_actpoly.size() << " active edge num: "
	//	<< act->_actedge.size() << std::endl;
}

void ZBuffer::scanWithoutClassifiedEdges(int y)
{
	//std::cout << "-------Scan: " << y << " ---------\n";
	if (y % 100 == 0)
	{
		std::cout << "Scanning: " << float(V_PIX_NUM - y) / V_PIX_NUM * 100 << " %\n";
	}
	PolyList* cls = _pll;
	ActiveList* act = _pll->_actList;
	//维护：激活该线上的分类多边形； 到尾端的删掉/维护
	//std::cout << "start ActiveP" << std::endl;
	_pll->activePv2(y);
	//std::cout << "start delActiveP" << std::endl;
	act->delActiveP(y);
	//std::cout << "start updateActive" << std::endl;
	act->updateActiveEv2(y);

	//绘制
	//std::cout << "start draw" << std::endl;
	act->drawv2(y, _depth, _buffer);

	//维护dy
	act->decDy();
}

void ZBuffer::drawCharSrc()
{
	//system("pause");
	for (int i = V_PIX_NUM-1; i >=0; i--)
	{
		for (int j = 0; j < U_PIX_NUM; j++)
		{
			if (_output[i][j] == -1) std::cout << ' ';
			else std::cout << char(_output[i][j] + 'a');
		}
		std::cout << std::endl;
	}
}
