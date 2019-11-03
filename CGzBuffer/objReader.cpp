#include "objReader.h"

void objReader::read(std::vector<Face>& face, std::vector<vec3>& pos, std::vector<vec3>& nml, std::vector<vec2>& tex)
{
	std::string s;
	while (!fin.eof())
	{
		getline(fin, s);
		std::stringstream ss(s);
		ss >> s;
		if (s == "") continue;
		if (s[0] == '#') continue;
		if (s == "v")
		{
			float x, y, z;
			ss >> x >> y >> z;
			pos.push_back(vec3(x, y, z));
			continue;
		}
		if (s == "vn")
		{
			float x, y, z;
			ss >> x >> y >> z;
			nml.push_back(vec3(x, y, z));
			continue;
		}
		if (s == "vt")
		{
			float x, y;
			ss >> x >> y;
			tex.push_back(vec2(x, y));
			continue;
		}
		if (s == "f")
		{
			std::string temp;
			std::vector<Vertex> tv(0);
			for (int i = 0; i < 3; i++)
			{
				ss >> temp;
				int p0 = temp.find('/');
				int p1 = temp.find('/', p0 + 1);
				int x=-1, y=-1, z=-1;
				if (p0 == -1 && p1==-1)
				{
					x = std::atoi(temp.c_str());
				}
				else if (p0 != -1 && p1 == -1)
				{
					x = std::atoi(temp.substr(0, p0).c_str());
					y = std::atoi(temp.substr(p0 + 1, temp.size()).c_str());
				}
				else if (p0 != -1 && p1 != -1)
				{
					if (p1 == p0 + 1)
					{
						x = std::atoi(temp.substr(0, p0).c_str());
						z = std::atoi(temp.substr(p1 + 1, temp.size()).c_str());
					}
					else
					{
						x = std::atoi(temp.substr(0, p0).c_str());
						y = std::atoi(temp.substr(p0 + 1, p1).c_str());
						z = std::atoi(temp.substr(p1 + 1, temp.size()).c_str());
					}
				}
				else
				{
					std::cout << "ERROR::objReader::faild to recognize the form" << std::endl;
				}
				//std::cout << temp<<' '<<x << ' ' << y << ' ' << z << std::endl;
				tv.push_back(Vertex(x-1, y-1, z-1));
			}
			face.push_back(Face(tv));
			face[face.size() - 1].calcNormal(pos);
		}
	}
	std::cout << "Num of Vertices: " << pos.size() << std::endl;
	std::cout << "Num of Normals: " << nml.size() << std::endl;
	std::cout << "Num of Textures: " << tex.size() << std::endl;
	std::cout << "Num of Faces: " << face.size() << std::endl;
	std::cout << "Successfully load file: " << _fileName << std::endl;
}
