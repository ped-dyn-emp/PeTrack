#ifndef VECTOR_HH
#define VECTOR_HH

#include "helper.h"

#include <QVector3D>
#include "opencv2/core/types_c.h"
#include <QMatrix4x4>

struct CvPoint;
struct CvPoint2D32f;
class QPointF;
class QPoint;

inline void printQVector3D(const QVector3D &p)
{
    debout << p.x() << " " << p.y() << " " << p.z() << endl;
}


inline void printQMatrix4x4(const QMatrix4x4 &m)
{
    for (int i=0; i<4; ++i)
        debout << (m.data())[i*4+0] << " " << (m.data())[i*4+1] << " "
               << (m.data())[i*4+2] << " " << (m.data())[i*4+3] << endl;
}

class Vec3F
{
protected:
    double mX;
    double mY;
    double mZ;

public:
    Vec3F();
    Vec3F(double x, double y, double z);
    Vec3F(const Vec3F& v);
//     Vec3F(const QPointF& v);
//     Vec3F(const CvPoint* v);

//     CvPoint toCvPoint() const;
//     QPoint toQPoint() const;
//     QPointF toQPointF() const;

    double x() const;
    double y() const;
    double z() const;
    void setX(double x);
    void setY(double y);
    void setZ(double z);
    void set(double x, double y, double z);

//     Vec3F operator=(const CvPoint *v);

    Vec3F operator+(const Vec3F& v) const;
    Vec3F operator-(const Vec3F& v) const;
    const Vec3F operator+=(const Vec3F& v);
    const Vec3F operator-=(const Vec3F& v);
    Vec3F operator-() const;
	
	Vec3F operator*(double n) const; // scalar product
	Vec3F operator/(double n) const; // scalar divide

	double operator*(const Vec3F& v) const; // dot product
// 	double operator%(const Vec3F& v) const; // cross product

	bool operator==(const Vec3F& v) const;
	bool operator!=(const Vec3F& v) const;

	double length() const;
// 	double angle() const;
	Vec3F unit() const;
// 	Vec3F normal() const;
	void normalize();

    double distanceToPoint(const Vec3F& p) const;

//     // return distance from vec to line between p1 and p2
//     double distanceToLine(const Vec3F& p1, const Vec3F& p2) const;
//     double angleBetweenVec(const Vec3F& v) const;

// 	static Vec3F fromAngle(double angle);
};


inline Vec3F operator*(double f, const Vec3F& v)
{
    return v*f;
}

inline ostream& operator<< (ostream& s, const Vec3F& v)
{
    s << "(" << v.x() << ", " << v.y() << ", " << v.z() << ")";
    return s;
}

//-------------------------------------------------------------------------------

class Vec2F
{
protected:
    //private: dann waeren mX... nicht direkt aus abgeleitet klassen zugreifbar
    double mX;
    double mY;

public:
    Vec2F();
    Vec2F(double x, double y);
    Vec2F(const Vec2F& v);
    Vec2F(const QPointF& v);
    Vec2F(const CvPoint* v);
    Vec2F(const CvPoint2D32f* v);

    CvPoint toCvPoint() const;
    CvPoint2D32f toCvPoint2D32f() const;
    QPoint toQPoint() const;
    QPointF toQPointF() const;

    double x() const;
    double y() const;
    void setX(double x);
    void setY(double y);
    void set(double x, double y);

    Vec2F operator=(const CvPoint *v);

    const Vec2F& operator+=(const Vec2F& v);
    const Vec2F operator+(const Vec2F& v) const;
    const Vec2F operator-=(const Vec2F& v);
    const Vec2F operator-(const Vec2F& v) const;
    Vec2F operator-() const;
	
	Vec2F operator*(double n) const; // scalar product
	Vec2F operator/(double n) const; // scalar divide

	double operator*(const Vec2F& v) const; // dot product
// 	double operator%(const Vec2F& v) const; // cross product

	bool operator==(const Vec2F& v) const;
	bool operator!=(const Vec2F& v) const;

	double length() const;
	double angle() const;
	Vec2F unit() const;
	Vec2F normal() const;
	void normalize();

    double distanceToPoint(const Vec2F& p) const;
    // return distance from vec to line between p1 and p2
    double distanceToLine(const Vec2F& p1, const Vec2F& p2) const;
    double angleBetweenVec(const Vec2F& v) const;

	static Vec2F fromAngle(double angle);
};


inline Vec2F operator*(double f, const Vec2F& v)
{
    return v*f;
}

inline ostream& operator<< (ostream& s, const Vec2F& v)
{
    s << "(" << v.x() << ", " << v.y() << ")";
    return s;
}

// only for combine programm, not for petrack
inline QTextStream& operator<< (QTextStream& s, const Vec2F& v)
{
    s << v.x() << " " << v.y(); // ohne (,) !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    return s;
}
inline istream& operator>> (istream& s, Vec2F& v)
{
    double d;
    s >> d; 
    v.setX(d);
    s >> d; 
    v.setY(d);
    return s;
}
inline QTextStream& operator>> (QTextStream& s, Vec2F& v)
{
    double d;
    s >> d; 
    v.setX(d);
    s >> d; 
    v.setY(d);
    return s;
}

#endif
