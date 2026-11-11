#include "game/POIPicture.hpp"

POIPicture::POIPicture() : GUI::Picture(2), location(NULL), label(NULL),
	coordinateX(0), coordinateY(0)
{
}

POIPicture::~POIPicture()
{
	//4587C0 only runs the Picture/base lifetime. The label is a child alias.
}
