#include "ZBuffer.h"

void ZBuffer::refreshLine()
{
	for (int i = 0; i < U_PIX_NUM; i++)
	{
		_depth[i] = -FLT_MAX;
		_buffer[i] = -1;
	}
}

void ZBuffer::refreshOutput()
{
	for (int i = 0; i < V_PIX_NUM; i++)
		for (int j = 0; j < U_PIX_NUM; j++)
			_output[i][j] = -1;
}

void ZBuffer::generate()
{
	_pll->refreshList();
	
	for (int i = V_PIX_NUM - 1; i >= 0; i--)
	{
		refreshLine();
		scan(i);
		for (int j = 0; j < U_PIX_NUM; j++)
		{
			_output[i][j] = _buffer[j];
		}
	}
}

void ZBuffer::scan(int y)
{
	//std::cout << "-------Scan: " << y << " ---------\n";
	PolyList* cls = _pll;
	ActiveList* act = _pll->_actList;
	//维护：激活该线上的分类多边形； 到尾端的删掉/维护
	//std::cout << "start ActiveP" << std::endl;
	_pll->activeP(y);
	//std::cout << "start delActiveP" << std::endl;
	act->delActiveP(y);
	//std::cout << "start updateActive" << std::endl;
	act->updataActiveE(y);
	//std::cout << "active Ply num: " << act->_actpoly.size() << " active edge num: "
	//	<< act->_actedge.size() << std::endl;
	
	//绘制
	//std::cout << "start draw" << std::endl;
	act->draw(y,_depth,_buffer);

	//维护dy
	act->decDy();
}

void ZBuffer::drawCharSrc()
{
	system("pause");
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
