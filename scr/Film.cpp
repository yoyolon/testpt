#include "Film.h"


Film::Film(int _w, int _h, int _c, const std::string& _filename)
	: w(_w), h(_h), c(_c), filename(_filename)
{
	aspect = (float)w / h;
}