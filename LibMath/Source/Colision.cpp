#define _USE_MATH_DEFINES
#include <cmath>

#include "LibMath/Colision.h"

bool LibMath::colisionPointLine(Point2D point, Line2D line)
{
	float lineLength = line.length();

	float d1 = point.distance(line.getStart());
	float d2 = point.distance(line.getEnd());

	return std::fabs((d1 + d2) - lineLength) <= g_epsilon;
}

bool LibMath::colisionTwoLine(Line2D line1, Line2D line2)
{
	float x1 = line1.getStart().getX();
	float y1 = line1.getStart().getY();
	float x2 = line1.getEnd().getX();
	float y2 = line1.getEnd().getY();

	float x3 = line2.getStart().getX();
	float y3 = line2.getStart().getY();
	float x4 = line2.getEnd().getX();
	float y4 = line2.getEnd().getY();

	// check if lines are identical
	if ((x1 == x3 && y1 == y3 && x2 == x4 && y2 == y4) || (x1 == x4 && y1 == y4 && x2 == x3 && y2 == y3))
	{
		return true;
	}

	float uA = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));

	float uB = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / ((y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1));

	if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) 
	{
		return true;
	}
	return false;
}

bool LibMath::colisionPointCircle(Point2D point, Circle circle)
{
	return point.distance(circle.getCenter()) <= circle.getRadius();
}

bool LibMath::colisionLineCircle(Line2D line, Circle circle)
{
	if (colisionPointCircle(line.getStart(), circle) ||
		colisionPointCircle(line.getEnd(), circle))
	{
		return true;
	}

	float lineLength = line.lengthSquare();

	float cx = circle.getCenter().getX();
	float cy = circle.getCenter().getY();
	float x1 = line.getStart().getX();
	float y1 = line.getStart().getY();
	float x2 = line.getEnd().getX();
	float y2 = line.getEnd().getY();

	float dot = (((cx - x1) * (x2 - x1)) + ((cy - y1) * (y2 - y1))) / lineLength;
	
	if (dot < 0)
	{
		dot = 0;
	}
	else if (dot > 1)
	{
		dot = 1;
	}

	Point2D closest(x1 + (dot * (x2 - x1)), y1 + (dot * (y2 - y1)));


	return colisionPointLine(closest, line) && closest.distance(circle.getCenter()) <= circle.getRadius();
}

bool LibMath::colisionTwoCircle(Circle circle1, Circle circle2)
{
	return circle1.getCenter().distance(circle2.getCenter()) <= (circle1.getRadius() + circle2.getRadius());
}

void LibMath::getNormals(Vector2 axes[8], Point2D corners1[4], Point2D corners2[4])
{
	int axisCount = 0;

	for (int i = 0; i < 4; i++)
	{
		float x = corners1[(i + 1) % 4].getX() - corners1[i].getX();
		float y = corners1[(i + 1) % 4].getY() - corners1[i].getY();

		Vector2 axis(-y, x);
		axis.normalize();
		axes[axisCount++] = axis;
	}

	for (int i = 0; i < 4; i++)
	{
		float x = corners2[(i + 1) % 4].getX() - corners2[i].getX();
		float y = corners2[(i + 1) % 4].getY() - corners2[i].getY();

		Vector2 axis(-y, x);
		axis.normalize();
		axes[axisCount++] = axis;
	}
}

void LibMath::getMinMax(float& min, float& max, Point2D corners[4], Vector2 axis)
{
	Vector2		corner;

	corner = Vector2(corners[0].getX(), corners[0].getY());

	min = max = axis.dotProduct(corner);

	for (int j = 1; j < 4; j++)
	{
		corner = Vector2(corners[j].getX(), corners[j].getY());

		float projection = axis.dotProduct(corner);

		if (projection < min)
		{
			min = projection;
		}
		if (projection > max)
		{
			max = projection;
		}
	}
}

bool LibMath::colisionTwoRectangle(Rectangle rec1, Rectangle rec2)
{
	// AABB
	if (rec1.getRotation().raw() == 0 && rec2.getRotation().raw() == 0)
	{
		if (rec1.getCenter().getX() - rec1.extentX() > rec2.getCenter().getX() + rec2.extentX() ||
			rec1.getCenter().getY() - rec1.extentY() > rec2.getCenter().getY() + rec2.extentY() ||
			rec2.getCenter().getX() - rec2.extentX() > rec1.getCenter().getX() + rec1.extentX() ||
			rec2.getCenter().getY() - rec2.extentY() > rec1.getCenter().getY() + rec1.extentY())
		{
			return false;
		}
		return true;
	}

	// OBB

	Point2D corners1[4];
	Point2D corners2[4];

	rec1.getCorners(corners1);
	rec2.getCorners(corners2);

	Vector2 axes[8];

	getNormals(axes, corners1, corners2);

	float		minA;
	float		maxA;
	float		minB;
	float		maxB;
	Vector2		corner;

	int			axisCount = 8;

	for (int i = 0; i < axisCount; i++)
	{
		getMinMax(minA, maxA, corners1, axes[i]);

		getMinMax(minB, maxB, corners2, axes[i]);

		if (maxA < minB || maxB < minA)
		{
			return false;
		}
	}
	return true;
}

bool LibMath::colisionPointRectangle(Point2D point, Rectangle rect)
{
	return (point.getX() <= rect.getCenter().getX() + rect.extentX() &&
			point.getY() <= rect.getCenter().getY() + rect.extentY() &&
			point.getX() >= rect.getCenter().getX() &&
			point.getY() >= rect.getCenter().getY());
}

bool LibMath::colisionLineRectangle(Line2D line, Rectangle rect)
{
	Point2D corners[4];

	rect.getCorners(corners);

	return (colisionTwoLine(line, Line2D(corners[0], corners[1])) ||
			colisionTwoLine(line, Line2D(corners[1], corners[2])) ||
			colisionTwoLine(line, Line2D(corners[2], corners[3])) ||
			colisionTwoLine(line, Line2D(corners[3], corners[0])));
}