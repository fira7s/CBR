#include "currentView.h"





currentView::currentView()
{

}
currentView::currentView(std::string a, int b)
{
	CheminArchive = a;
	currentPage = b;

}
int currentView::get_page_Number()
{
	return currentPage;
}
std::string currentView::get_current_archvie()
{
	return CheminArchive;
}
void currentView::set_page_number(int a)
{
	currentPage = a;
}
void currentView::set_current_archvie(std::string a)
{
	CheminArchive = a;
}
