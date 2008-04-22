#include <iostream>
#include <fstream>


#include <boost/numeric/ublas/vector.hpp>
#include <rw/math/Q.hpp>
#include <rw/math/Transform3D.hpp>
#include <sandbox/interpolator/Trajectory.hpp>
#include <sandbox/interpolator/LineInterpolator.hpp>
#include <sandbox/interpolator/ParabolicBlend.hpp>
#include <sandbox/interpolator/LloydHaywardBlend.hpp>
#include <sandbox/interpolator/CircularInterpolator.hpp>
#include <sandbox/interpolator/TrajectoryIterator.hpp>
#include <rw/common/Exception.hpp>

using namespace rw::math;
using namespace rw::sandbox;
using namespace rw::common;


//typedef boost::numeric::ublas::vector<float> T;
typedef Q T;
//typedef Transform3D<> T;

const double dt = 0.01;

int main() {
 /*   std::cout<<"Test of CircularInterpolator "<<std::endl;
    Vector3D<> p1(0,-1,0);
    Vector3D<> p2(0,0,1);
    Vector3D<> p3(0,1,0);
    
    CircularInterpolator<Vector3D<> > interpolator(p1, p2, p3, 1);
    Vector3D<> xlast = interpolator.x(0);
    Vector3D<> dxlast = interpolator.dx(0);
    std::ofstream out("test.dat");
    for (double t = 0; t<=interpolator.getLength(); t += dt) {
        Vector3D<> x = interpolator.x(t);
        Vector3D<> dx = interpolator.dx(t);
        Vector3D<> ddx = interpolator.ddx(t);
       out<<t<<" "<<x(0)<<" "<<x(1)<<" "<<x(2)<<" ";
       out<<dx(0)<<" "<<dx(1)<<" "<<dx(2)<<" "<<(x(0)-xlast(0))/dt<<" "<<(x(1)-xlast(1))/dt<<" "<<(x(2)-xlast(2))/dt<<" ";
       out<<ddx(0)<<" "<<ddx(1)<<" "<<ddx(2)<<" "<<(dx(0)-dxlast(0))/dt<<" "<<(dx(1)-dxlast(1))/dt<<" "<<(dx(2)-dxlast(2))/dt<<std::endl;
       xlast = x;
       dxlast = dx;
    }
    out.close();

    */
    std::cout<<"Test of Transform3D<> based Interpolator"<<std::endl;
    Transform3D<> T1(Vector3D<>(0,0,0), EAA<>(0,0,0));
    Transform3D<> T2(Vector3D<>(1,1,0), EAA<>(1,1,0));
    Transform3D<> T3(Vector3D<>(2,0,0), EAA<>(2,2,0));
    Transform3D<> T4(Vector3D<>(3,2,0), EAA<>(0,0,2));
    
    LineInterpolator<Transform3D<> >* cartInt1 = new LineInterpolator<Transform3D<> >(T1, T2, 1);
    LineInterpolator<Transform3D<> >* cartInt2 = new LineInterpolator<Transform3D<> >(T2, T3, 1);
    LineInterpolator<Transform3D<> >* cartInt3 = new LineInterpolator<Transform3D<> >(T3, T4, 1.5);
    ParabolicBlend<Transform3D<> >* blend1 = new ParabolicBlend<Transform3D<> >(cartInt1, cartInt2, 0.4);
    LloydHaywardBlend<Transform3D<> >* blend2 = new LloydHaywardBlend<Transform3D<> >(cartInt2, cartInt3, 0.4, 15/2);
    Trajectory<Transform3D<> > trajectory;
    trajectory.add(cartInt1);
    trajectory.add(blend1, cartInt2);
    Trajectory<Transform3D<> > trajectory2;
    trajectory2.add(blend2, cartInt3);
    trajectory.add(&trajectory2);
    std::cout<<"getLength "<<trajectory.getLength()<<std::endl;
    std::ofstream out("test.dat");
    TrajectoryIterator<Transform3D<> > iter(&trajectory);
    while (!iter.isEnd()) {
        Transform3D<> x = iter.x();
        Transform3D<> dx = iter.dx();
        Transform3D<> ddx = iter.ddx();
        EAA<> eaa(x.R()); 
        out<<iter.getTime()<<" "<<x.P()(0)<<" "<<x.P()(1)<<" "<<x.P()(2)<<" "<<eaa(0)<<" "<<eaa(1)<<" "<<eaa(2)<<std::endl;
        
        iter += dt;
        
    }
    /*
    for (double t = 0; t<=trajectory.getLength(); t += dt) {
        Transform3D<> x = trajectory.x(t);
        Transform3D<> dx = trajectory.dx(t);
        Transform3D<> ddx = trajectory.ddx(t);
        EAA<> eaa(x.R()); 
        out<<t<<" "<<x.P()(0)<<" "<<x.P()(1)<<" "<<x.P()(2)<<" "<<eaa(0)<<" "<<eaa(1)<<" "<<eaa(2)<<std::endl;
    }
    */
    out.close();
    std::cout<<"Finished"<<std::endl;
    

    
    /*std::cout<<"Test of ordinary Q based interpolators"<<std::endl;
    Trajectory<T> trajectory;
    T a(2); a(0) = 0; a(1) = 0;
    T b(2); b(0) = 1; b(1) = 1;    
    T c(2); c(0) = 2; c(1) = 0;
    T d(2); d(0) = 3; d(1) = 1;
    
    LineInterpolator<T>* line1 = new LineInterpolator<T>(a,b,1);
    LineInterpolator<T>* line2 = new LineInterpolator<T>(b,c,1);
    LineInterpolator<T>* line3 = new LineInterpolator<T>(c,d,0.5);
    
    ParabolicBlend<T>* blend1 = new ParabolicBlend<T>(line1, line2, 0.5);
    //LloydHaywardBlend<T>* blend1 = new LloydHaywardBlend<T>(line1, line2, 0.2, 15/2);
    //ParabolicBlend<T>* blend2 = new ParabolicBlend<T>(line2, line3, 0.25);
    LloydHaywardBlend<T>* blend2 = new LloydHaywardBlend<T>(line2, line3, 0.35, 15/2);
    
    trajectory.add(line1, blend1);
    trajectory.add(line2, blend2);
    trajectory.add(line3);
    
    std::ofstream out("test.dat");
    T qlast = trajectory.x(0);
    T dqlast = trajectory.dx(0);
    
    for (double t = 0; t<trajectory.getLength(); t += dt ) {
        T q = trajectory.x(t);
        T dq = trajectory.dx(t);
        T ddq = trajectory.ddx(t);
        std::cout<<t<<":  "<<q<<std::endl;
        out<<t<<" "<<q(0)<<" "<<q(1)<<" "<<dq(0)<<" "<<dq(1)<<" "<<ddq(0)<<" "<<ddq(1)<<" ";
        out<<(q(0)-qlast(0))/dt<<" "<<(q(1)-qlast(1))/dt<<" ";
        out<<(dq(0)-dqlast(0))/dt<<" "<<(dq(1)-dqlast(1))/dt<<std::endl;
        qlast = q;
        dqlast = dq;
    }
    out.close();
    */
    return 0;
}
