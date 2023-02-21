#pragma once
#include <string>


class currentView
{
	std::string CheminArchive;
	int currentPage;
public:
	currentView();
	currentView(std::string a, int b);
	int get_page_Number();
	std::string get_current_archvie();
	void set_page_number(int a);
	void set_current_archvie(std::string a);
};

