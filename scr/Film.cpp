#include "Film.h"


Film::Film(int _h, int _w, int _c, const char* f)
	: h(_h), w(_w), c(_c), filename(f)
{
	aspect = (float)w / h;
}