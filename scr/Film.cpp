#include "Film.h"


Film::Film(int _w, int _h, int _c, const char* f)
	: w(_w), h(_h), c(_c), filename(f)
{
	aspect = (float)w / h;
}