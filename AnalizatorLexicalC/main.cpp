#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<algorithm>

using namespace std;

enum TipToken
{
	constantaIntreaga,
	constantaFlotanta,
	constantaString,
	constantaHexa,
	cuvantCheie,
	identificator,
	operatorr,
	separator,
	comentariu,
	eof,
	eroareComentariu,
	eroareConstantaString,
	eroareFinalNeasteptat,
	eroareHexa
};

class Token
{
public:
	TipToken tipToken;
	int pozValoare;

	Token(TipToken tip, int pozValoare)
	{
		tipToken = tip;
		this->pozValoare = pozValoare;
	}

	string tokenLizibil()
	{
		string tipToken;
		switch (this->tipToken)
		{
		case TipToken::comentariu:
			tipToken = "Comentariu";
			break;
		case TipToken::constantaFlotanta:
			tipToken = "Constanta Flotanta";
			break;
		case TipToken::constantaIntreaga:
			tipToken = "Constanta Intreaga";
			break;
		case TipToken::constantaHexa:
			tipToken = "Constanta Hexa";
			break;
		case TipToken::constantaString:
			tipToken = "Constanta String";
			break;
		case TipToken::cuvantCheie:
			tipToken = "Cuvant cheie";
			break;
		case TipToken::identificator:
			tipToken = "Identificator";
			break;
		case TipToken::operatorr:
			tipToken = "Operator";
			break;
		case TipToken::separator:
			tipToken = "Separator";
			break;
		case TipToken::eroareComentariu:
			tipToken = "EROARE! Comentariu formatat incorect.";
			break;
		case TipToken::eroareConstantaString:
			tipToken = "EROARE! String formatat incorect";
			break;
		case TipToken::eroareFinalNeasteptat:
			tipToken = "EROARE! Final fisier neasteptat!";
			break;
		case TipToken::eroareHexa:
			tipToken = "EROARE! Valoare hexa eronata!";
			break;
		default:
			tipToken = "Necunoscut";
			break;
		}
		return tipToken;
	}

	bool eEroare()
	{
		return (tipToken == TipToken::eroareComentariu || tipToken == TipToken::eroareConstantaString || tipToken == TipToken::eroareFinalNeasteptat);
	}
};


class AnalizatorLexical
{
private:
	vector<string> tabelaCuvCheie{"auto","break","case","char","const","continue","default",
							"do","double","else","enum","extern","float","for","goto",
							"if","inline","int","long","register","restrict","return","short","signed",
							"sizeof","static","struct","switch","typedef","union",
							"unsigned","void","volatile","while"};
	vector<string> tabelaValori;
	ifstream fisier;
	int caracterCurent=0;
	int linie = 1;
	int caracter = 0;

public:
	AnalizatorLexical(string caleFisier)
	{
		fisier.open(caleFisier,ios::in);
		if (!fisier.is_open())
		{
			cerr << "Eroare la deschidere fisier";
		}
	}

	Token gettoken()
	{
		// Verificare final fisier
		if (fisier.eof())
		{
			return Token(TipToken::eof,caracterCurent);
		}

		// Variabila pentru acumulare valori
		string valoareAcumulata = "";

		// Variabila pt setare tip
		TipToken tipToken = TipToken::eof;

		// Variabila pentru memorare stare
		int codStare = 0;

		// Variabila pt marcare eroare
		bool eroare = false;

		while (true)
		{
			char c = fisier.get();
			valoareAcumulata += c;
			caracterCurent++;

			if (c == '\n')
			{
				caracter = 0;
				linie++;
			}
			else
			{
				caracter++;
			}
			
			// Verificare final fisier
			if (fisier.eof())
			{
				// s-a terminat
				break;
			}

			// Stare initiala
			if (codStare == 0)
			{
				// Verificare spatiu
				if (isWhitespace(c))
				{
					// ignoram
					valoareAcumulata = "";
					continue;
				}

				// String literal
				if (c == '"')
				{
					codStare = 1;
					continue;
				}

				// Inceput constanta intreaga/flotanta
				if (isDigit(c) && c != '0')
				{
					codStare = 3;
					continue;
				}

				// Inceput cifra 0
				if (c == '0')
				{
					codStare = 11;
					continue;
				}

				// Separator
				if (isSeparator(c))
				{
					tipToken = TipToken::separator;
					break;
				}

				// Operator
				if (c == '.' || c == '~')
				{
					// Operatori care nu pot avea decat lungime 1
					tipToken = TipToken::operatorr;
					break;
				}
				if (isOperator(c))
				{
					codStare = 5;
					continue;
				}

				//Identificatori
				if (isLetter(c) || c == '_')
				{
					tipToken = TipToken::identificator;
					codStare = 7;
					continue;
				}
			}

			#pragma region ConstantaString

			if (codStare == 1)
			{
				// Verificam aparitie "
				if (c == '"')
				{
					tipToken = TipToken::constantaString;
					// Ne oprim pt ca starea urmatoare nu are tranzitii
					break;
				}

				// Verificare aparitie '\'
				if (c == '\\')
				{
					codStare = 2;
					continue;
				}

				// Verificare caractere
				if (isCharacter(c) || c == ' ' || isDigit(c))
				{
					continue;
				}

				// Eroare
				eroare = true;
				tipToken = TipToken::constantaString;
				break;
			}

			if (codStare == 2)
			{
				// Verificam caractere
				if (isCharacter(c) || c == ' ' || isDigit(c))
				{
					codStare = 1;
					continue;
				}

				// Eroare
				eroare = true;
				tipToken = TipToken::constantaString;
				break;
			}

			#pragma endregion

			#pragma region ConstantaInt
			if (codStare == 3)
			{
				tipToken = TipToken::constantaIntreaga;
				// Verificam daca avem .
				if (c == '.')
				{
					// Mergem in starea pt flotant
					codStare = 4;
					continue;
				}

				if (isDigit(c))
				{
					continue;
				}

				// Blocaj
				fisier.unget();
				caracterCurent--;
				valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				break;
			}

			#pragma endregion

			#pragma region ConstantaFlotant
			if (codStare == 4)
			{
				tipToken = TipToken::constantaFlotanta;
				if (isDigit(c))
				{
					continue;
				}

				//Am atins finalul
				if (c != 'f')
				{
					fisier.unget();
					caracterCurent--;
					valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				}
				break;
			}

			#pragma endregion

			#pragma region Operatori

			if (codStare == 5)
			{
				// Verificare comentariu
				if (c == '*' && valoareAcumulata[0] == '/')
				{
					// Mergem la comentariu
					codStare = 9;
					continue;
				}

				tipToken = TipToken::operatorr;
				// verificam operatorii de lungime 2 formati din dublare <<,>>,&&,||,++,--
				if ((c == '<' || c == '>' || c == '&' || c == '|' || c == '+' || c == '-') && c == valoareAcumulata[0])
				{
					// Verificam << sau >> care pot merge si la lungime 3
					if (c == '<' || c == '>')
					{
						codStare = 6;
						continue;
					}

					// Gata
					break;
				}

				if (c == '>' && valoareAcumulata[0] == '-')
				{
					// Gata
					break;
				}

				if (c == '=')
				{
					// Gata
					break;
				}

				// Blocaj
				valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				fisier.unget();
				caracterCurent--;
				break;
			}

			if (codStare == 6)
			{
				if (c == '=')
				{
					break;
				}

				// Blocaj
				valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				fisier.unget();
				caracterCurent--;
				break;
			}

			#pragma endregion

			#pragma region Identificatori

			if (codStare == 7)
			{
				if (isLetter(c) || isDigit(c) || c == '_')
				{
					continue;
				}

				// Blocaj
				valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				fisier.unget();
				caracterCurent--;
				break;
			}

			#pragma endregion

			#pragma region Comentarii

			if (codStare == 9)
			{
				tipToken = TipToken::comentariu;
				// Caz steluta
				if (c == '*')
				{
					codStare = 10;
					continue;
				}

				// Caz caracter eronat
				if (!(isCharacter(c) || isWhitespace(c)))
				{
					eroare = true;
					tipToken = TipToken::eroareComentariu;
					break;
				}
			}


			if (codStare == 10)
			{
				// Daca avem / s-a terminat, altfel inapoi
				if (c == '/')
				{
					tipToken = TipToken::comentariu;
					// Putem opri fortat parcurgerea pentru ca starea urmatoare nu are tranzitii
					break;
				}

				// Daca avem * ramanem pe loc
				if (c == '*')
				{
					continue;
				}

				// Daca avem caracter ne intoarcem la 9
				if (isCharacter(c) || isWhitespace(c))
				{
					codStare = 9;
					continue;
				}

				// Eroare
				eroare = true;
				tipToken = TipToken::eroareComentariu;
				break;
			}
			#pragma endregion

			#pragma region Hexa/Octal

			if (codStare == 11)
			{
				if (c == 'x' || c=='X')
				{
					tipToken = TipToken::constantaIntreaga;
					codStare = 12;
					continue;
				}

				//Blocaj, am gasit 0 const int
				tipToken = TipToken::constantaIntreaga;
				valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				fisier.unget();
				caracterCurent--;
				break;
			}

			if (codStare == 12)
			{
				string caractereHexaPrima = "123456789abcdefABCDEF";
				if (caractereHexaPrima.find(c) != string::npos)
				{
					codStare = 13;
					continue;
				}

				// Eroare
				eroare = true;
				tipToken = TipToken::eroareComentariu;
				break;
			}

			if (codStare == 13)
			{
				string caractereHexa = "0123456789abcdefABCDEF";
				if (caractereHexa.find(c) != string::npos)
				{
					continue;
				}

				// Blocaj
				tipToken = TipToken::constantaHexa;
				valoareAcumulata = valoareAcumulata.substr(0, valoareAcumulata.size() - 1);
				fisier.unget();
				caracterCurent--;
				break;
			}

			#pragma endregion
		}
		
		if (fisier.eof())
		{
			if (codStare == 0)
			{
				return Token(TipToken::eof, 0);
			}

			if (codStare == 1 || codStare == 2 || codStare == 8 || codStare == 9 || codStare == 10)
			{
				tipToken = TipToken::eroareFinalNeasteptat;
				eroare = true;
			}
		}

		// Caz oprire cu eroare
		if (eroare)
		{
			return Token(tipToken, caracterCurent);
		}

		int pozitie = 0;
		vector <string>::iterator it;

		// Verificam identificatorii daca sunt cuvinte cheie
		if (tipToken == TipToken::identificator)
		{
			it = find(tabelaCuvCheie.begin(), tabelaCuvCheie.end(), valoareAcumulata);
			if (it != tabelaCuvCheie.end())
			{
				// Este un identificator
				tipToken = TipToken::cuvantCheie;
				pozitie = distance(tabelaCuvCheie.begin(), it);
				return Token(tipToken, pozitie);
			}
		}

		// Cautam valoarea acumulata in tabela de stringuri
		it = find(tabelaValori.begin(), tabelaValori.end(), valoareAcumulata);
		if (it != tabelaValori.end())
		{
			pozitie = distance(tabelaValori.begin(), it);
		}
		else
		{
			// adaugam la finalul listei
			tabelaValori.push_back(valoareAcumulata);
			pozitie = tabelaValori.size() - 1;
		}		
		
		return Token(tipToken, pozitie);
	}

	string valoareTabelaString(Token t)
	{
		if (t.tipToken == TipToken::cuvantCheie)
			return tabelaCuvCheie[t.pozValoare];

		return tabelaValori[t.pozValoare];
	}

	int getLinie()
	{
		return linie;
	}

	int getCaracter()
	{
		return caracter;
	}

private:
	// functii pt caractere
	bool isLetter(char c)
	{
		string litere = "abcdefghijqlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

		return litere.find(c) != string::npos;

	}

	bool isDigit(char c)
	{
		string cifre = "0123456789";

		return cifre.find(c) != string::npos;
	}

	bool isCharacter(char c)
	{
		string caractere = "abcdefghijqlmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!\"#%&'()*+,-./:;<=>?[\\]^_{|}~0123456789";

		return caractere.find(c) != string::npos;
	}

	bool isWhitespace(char c)
	{
		return isspace(c);
	}

	bool isSeparator(char c)
	{
		string separatori = "{},();";

		return separatori.find(c) != string::npos;
	}

	bool isOperator(char c)
	{
		string operatori = "+-*/%=&|^<>!";
		return operatori.find(c) != string::npos;
	}
};


int main(int argc, char *argv[])
{
	// Verificare parametrii main
	if (argc != 3)
	{
		cerr << "Numar incorect de parametrii! Param1: cale fisier cu cod C, Param2: cale fisier output tokeni";
	}

	// Luam parametrii
	string caleFisierC = argv[1], caleOutput = argv[2];

	// Construim analizatorul
	AnalizatorLexical analizator(caleFisierC);

	// Deschidem fisierul de output
	ofstream output(caleOutput);

	Token token = analizator.gettoken();
	while (token.tipToken != TipToken::eof)
	{
		if (token.eEroare())
		{
			cerr << "A aparut o eroare in analiza lexicala.";
			output << "Eroare lexicala de tipul: " << token.tokenLizibil() << ", la pozitia absoluta: " << token.pozValoare << 
				" ,linia: " << analizator.getLinie() << " ,caracterul: " << analizator.getCaracter() << endl;
			return 0;
		}
		output << "( " << token.tokenLizibil() << ", " << analizator.valoareTabelaString(token)<<")"<<endl;
		token = analizator.gettoken();
	}

	return 0;
}