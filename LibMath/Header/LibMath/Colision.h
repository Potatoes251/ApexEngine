#ifndef COLISION2D_H
#define COLISION2D_H

#include "Geometry.h"
#include "Vector/Vector2.h"

namespace LibMath
{
	//2D collision

	bool			colisionPointLine(Point2D, Line2D);
	bool			colisionTwoLine(Line2D, Line2D);

	bool			colisionPointCircle(Point2D, Circle);
	bool			colisionLineCircle(Line2D, Circle);
	bool			colisionTwoCircle(Circle, Circle);

	void			getMinMax(float& min, float& max, Point2D corners[4], Vector2 axis);
	void			getNormals(Vector2 axes[8], Point2D corners1[4], Point2D corners2[4]);

	bool			colisionPointRectangle(Point2D, Rectangle);
	bool			colisionLineRectangle(Line2D, Rectangle);
	bool			colisionTwoRectangle(Rectangle, Rectangle);
}

#endif