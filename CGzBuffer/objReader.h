#pragma once
#include"common.h"
#include"Face.h"
#include"Vertex.h"
#include<fstream>
#include<sstream>

class objReader
{
private:
	std::string _fileName;
	std::ifstream fin;
	
public:
	objReader(std::string file):_fileName(file)
	{
		fin.open(file);
		if (fin.fail())
		{
			std::cout << "ERROR_OBJREADER_cannot open file: " << file << std::endl;
		}
		else
		{
			std::cout << "Successfully open file: " << file << std::endl;
		}
	}
	void read(std::vector<Face>& face, std::vector<vec3>& pos, std::vector<vec3>& nml, std::vector<vec2>& tex);

};

