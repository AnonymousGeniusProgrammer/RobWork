/*********************************************************************
 * RobWork Version 0.3
 * Copyright (C) Robotics Group, Maersk Institute, University of Southern
 * Denmark.
 *
 * RobWork can be used, modified and redistributed freely.
 * RobWork is distributed WITHOUT ANY WARRANTY; including the implied
 * warranty of merchantability, fitness for a particular purpose and
 * guarantee of future releases, maintenance and bug fixes. The authors
 * has no responsibility of continuous development, maintenance, support
 * and insurance of backwards capability in the future.
 *
 * Notice that RobWork uses 3rd party software for which the RobWork
 * license does not apply. Consult the packages in the ext/ directory
 * for detailed information about these packages.
 *********************************************************************/

#include <rw/math/Vector2D.hpp>

#include <boost/test/unit_test.hpp>

using namespace rw::math;

BOOST_AUTO_TEST_CASE(Vector2DTest)
{
    BOOST_MESSAGE("- Testing Vector2D");
    Vector2D<> v1(1.0, 2.0);
    Vector2D<> v2(v1);
    Vector2D<> v3 = v1+v2;
    Vector2D<> v4 = Vector2D<>(2.0, 4.0);

    BOOST_CHECK ( (v3-v4).normInf() == 0);

    Vector2D<> v5(3.0, 4.0);
    Vector2D<> v5_norm = normalize(v5);
    BOOST_CHECK(fabs( v5_norm.norm2()-1)<1e-15);
    double l = v5.norm2();
    BOOST_CHECK(fabs(v5_norm(0)-v5(0)/l) < 1e-15);
    BOOST_CHECK(fabs(v5_norm(1)-v5(1)/l) < 1e-15);

    v5(0) = l;
    v5(1) = 2*l;
    BOOST_CHECK(v5(0) == l);
    BOOST_CHECK(v5(1) == 2*l);

    Vector2D<std::string> vs1("x1", "y1");

    BOOST_CHECK(vs1(0) == "x1");
    BOOST_CHECK(vs1(1) == "y1");

    double cr = cross(Vector2D<>(2,1), Vector2D<>(1, 3));
    BOOST_CHECK(cr == 5);

    Vector2D<double> vd(1.51, -2.51);
    Vector2D<int> vi = cast<int>(vd);
    BOOST_CHECK(vi(0) == 1);
    BOOST_CHECK(vi(1) == -2);
}
