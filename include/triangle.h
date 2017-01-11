#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <QVector3D>

//A class used to store triangle data with a couple of
//specialised boolean operators and functions
class Triangle
{
public:
    QVector3D A = QVector3D(1000000, 1000000, 1000000);
    QVector3D B = QVector3D(1000000, -1000000, 1000000);
    QVector3D C = QVector3D(1000000, 1000000, -1000000);

    inline bool operator == (const Triangle &rhs)
    {
        return (( A.x() == rhs.A.x() && A.y() == rhs.A.y() && A.z() == rhs.A.z() ) &&
                    ( B.x() == rhs.B.x() && B.y() == rhs.B.y() && B.z() == rhs.B.z() ) &&
                    ( C.x() == rhs.C.x() && C.y() == rhs.C.y() && C.z() == rhs.C.z() ));
    }

    inline bool operator!= (const Triangle &rhs) { return !operator ==(rhs); }

    QVector3D getNormal()
    {
        QVector3D e1 = B - A;
        QVector3D e2 = C - A;

        return QVector3D::crossProduct(e1, e2).normalized();
    }

    QVector3D getMiddle()
    {
        QVector3D middle = A + B + C;
        return middle/3.0f;
    }

};

#endif // TRIANGLE_H
