#include<iostream>
#include<fstream>
#include<string>

enum TipToken
{
	constantaIntreaga,
	constantaFlotanta
};

class Token
{
public:
	TipToken tipToken;
	std::string valoare;
};


