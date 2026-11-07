#include "stdafx.h"
#include "Configuration.h"

Configuration::Configuration(char* file)
{
	ifstream in(file);
	char str[255];
	int line = 0;
	// Room for "255.255.255.255" / "65535" plus the terminating zero.
	ip = new char[16];
	port = new char[6];
	ip[0] = '\0';
	port[0] = '\0';
	districtId = 0;
	districtType = 0;
	language = 0;

	if (!in)
	{
		cout << "Cannot open config file.\n";
		return;
	}

	while (in) 
	{
		in.getline(str, 255);
		if (in)
		{
			if (line == 0) strncpy_s(ip, 16, str, _TRUNCATE);
			else if (line == 1) strncpy_s(port, 6, str, _TRUNCATE);
			else if (line == 2)
			{
				//TODO: support for other districts when their IDs are found
				if (strcmp(str, "Social") == 0) districtType = 1;
				else if (strcmp(str, "Financial") == 0) districtType = 2;
				else if (strcmp(str, "Tutorial") == 0) districtType = 14;
				else if (strcmp(str, "Waterfront") == 0) districtType = 21;
				else districtType = 1;
			}
			else if (line == 3) districtId = atoi(str);
			else if (line == 4)
			{
				if (strcmp(str, "English") == 0) language = 0;
				else if (strcmp(str, "French") == 0) language = 1;
				else if (strcmp(str, "Italian") == 0) language = 2;
				else if (strcmp(str, "German") == 0) language = 3;
				else if (strcmp(str, "Spanish") == 0) language = 4;
				else if (strcmp(str, "Russian") == 0) language = 5;
				else language = 0;
			}
			line++;
		}
	}
	in.close();
}

char* Configuration::GetWorldIP()
{
	return this->ip;
}

char* Configuration::GetWorldPort()
{
	return this->port;
}

int Configuration::GetDistrictType()
{
	return this->districtType;
}

int Configuration::GetDistrictID()
{
	return this->districtId;
}

int Configuration::GetDistrictLanguage()
{
	return this->language;
}