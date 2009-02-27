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

#include "../TestSuiteConfig.hpp"

#include <rw/math/EAA.hpp>

#include <rw/trajectory/Path.hpp>
#include <rw/trajectory/TrajectoryFactory.cpp>
#include <rw/math/Q.hpp>
#include <rw/math/Constants.hpp>

#include <rw/models/WorkCell.hpp>
#include <rw/models/SerialDevice.hpp>
#include <rw/loaders/WorkCellLoader.hpp>

#include <iostream>

#include <boost/test/unit_test.hpp>
#include <boost/foreach.hpp>

using namespace rw::math;
using namespace rw::models;
using namespace rw::kinematics;
using namespace rw::loaders;
using namespace rw::trajectory;

BOOST_AUTO_TEST_CASE( PathTest ){
    BOOST_MESSAGE("- Testing Path");

    //Test the simple path
    {
        QPath path;
        Q q(7);
        q(0) = 1;
        path.push_back(q);
        q(0) = 2;
        path.push_back(q);
        q(0) = 3;
        path.push_back(q);
        int index = 1;
        BOOST_FOREACH(Q q, path) {
            BOOST_CHECK(q(0) == index);
            index++;
        }
    }

    StatePath statepath;
    WorkCellPtr workcell = WorkCellLoader::load(testFilePath() + "MultiRobotDemo/Scene.wu");
    BOOST_CHECK(workcell);
    Device* dev = workcell->getDevices().front();
    const State defstate = workcell->getDefaultState();

    //Test StatePath and the possibility of creating
    {
        StatePath statepath;
        State state = defstate;

        statepath.push_back(state);
        Q q = dev->getQ(state);
        for (size_t i = 0; i<q.size(); i++)
            q(i) = 0.5;
        dev->setQ(q, state);
        statepath.push_back(state);
        for (size_t i = 0; i<q.size(); i++)
            q(i) = -1;
        dev->setQ(q, state);
        statepath.push_back(state);

        StateTrajectoryPtr statetrajectory = TrajectoryFactory::makeLinearTrajectoryUnitStep(statepath);
        State s0 = statetrajectory->x(0);
        q = dev->getQ(s0);
        BOOST_CHECK(q(0) == 0);

        State s05 = statetrajectory->x(0.5);
        q = dev->getQ(s05);
        BOOST_CHECK(q(0) == 0.25);

        State s1 = statetrajectory->x(1);
        q = dev->getQ(s1);
        BOOST_CHECK(q(0) == 0.5);

        State s15 = statetrajectory->x(1.5);
        q = dev->getQ(s15);
        std::cout<<"q = "<<q<<std::endl;
        BOOST_CHECK(q(0) == -0.25);

        State s2 = statetrajectory->x(2);
        q = dev->getQ(s2);
        BOOST_CHECK(q(0) == -1);
    }

    //Test StatePath and the possibility of creating
    {
        TimedStatePath timedStatePath;
        WorkCellPtr workcell = WorkCellLoader::load(testFilePath() + "MultiRobotDemo/Scene.wu");
        BOOST_CHECK(workcell);

        Device* dev = workcell->getDevices().front();
        State state = workcell->getDefaultState();
        timedStatePath.push_back(Timed<State>(0, state));

        Q q = dev->getQ(state);
        for (size_t i = 0; i<q.size(); i++)
            q(i) = 0.5;
        dev->setQ(q, state);
        timedStatePath.push_back(Timed<State>(12, state));

        for (size_t i = 0; i<q.size(); i++)
            q(i) = -1;
        dev->setQ(q, state);
        timedStatePath.push_back(Timed<State>(24, state));

        StateTrajectoryPtr statetrajectory = TrajectoryFactory::makeLinearTrajectory(timedStatePath);
        State s0 = statetrajectory->x(0);
        q = dev->getQ(s0);
        std::cout<<"q(0) = "<<q<<std::endl;
        BOOST_CHECK(q(0) == 0);

        State s3 = statetrajectory->x(3);
        q = dev->getQ(s3);
        std::cout<<"q(3) = "<<q<<std::endl;
        BOOST_CHECK(q(0) == 0.125);

        State s1 = statetrajectory->x(12);
        q = dev->getQ(s1);
        BOOST_CHECK(q(0) == 0.5);

        State s15 = statetrajectory->x(18);
        q = dev->getQ(s15);
        std::cout<<"q(18) = "<<q<<std::endl;
        BOOST_CHECK(q(0) == -0.25);

        State s2 = statetrajectory->x(24);
        q = dev->getQ(s2);
        BOOST_CHECK(q(0) == -1);
    }
}
