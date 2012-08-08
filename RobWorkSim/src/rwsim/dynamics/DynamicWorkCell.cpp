/********************************************************************************
 * Copyright 2009 The Robotics Group, The Maersk Mc-Kinney Moller Institute,
 * Faculty of Engineering, University of Southern Denmark
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ********************************************************************************/

#include "DynamicWorkCell.hpp"

#include <rw/common/macros.hpp>
#include <rw/kinematics/MovableFrame.hpp>
#include <rw/models/Device.hpp>
#include <rw/math/Vector3D.hpp>
#include <rw/math/Math.hpp>
#include <rw/models/WorkCell.hpp>

#include "Body.hpp"


using namespace rw::math;
using namespace rw::models;
using namespace rw::kinematics;
using namespace rw::proximity;
using namespace rwlibs::simulation;

using namespace rwsim::dynamics;

DynamicWorkCell::DynamicWorkCell(WorkCell::Ptr workcell,
                                 const DynamicWorkCell::BodyList& bodies,
                                 const DynamicWorkCell::BodyList& allbodies,
                                 const DynamicWorkCell::DeviceList& devices,
                                 const ControllerList& controllers):
    _workcell(workcell),
    _bodies(bodies),
    _allbodies(allbodies),
    _devices(devices),
    _controllers(controllers),
    _collisionMargin(0.001),
    _worldDimension(Vector3D<>(0,0,0), Vector3D<>(20,20,20)),
    _gravity(0,0,-9.82)
{
    BOOST_FOREACH(SimulatedController::Ptr b, _controllers){
        b->registerStateData( workcell->getStateStructure() );
    }

    BOOST_FOREACH(DynamicDevice::Ptr b, _devices){
        b->registerStateData( workcell->getStateStructure() );
    }

    BOOST_FOREACH(Body::Ptr b, _allbodies){
        _frameToBody[b->getBodyFrame()] = b;
        b->registerStateData( workcell->getStateStructure() );
    }
}

DynamicWorkCell::~DynamicWorkCell()
{
}

DynamicDevice::Ptr DynamicWorkCell::findDevice(const std::string& name) const {
    BOOST_FOREACH(const DynamicDevice::Ptr &ddev, _devices){
        if(ddev->getModel().getName()==name)
            return  ddev;
    }
    return NULL;
}

bool DynamicWorkCell::inDevice(Body::Ptr body){
    BOOST_FOREACH(Body::Ptr b, _bodies){
        if(body==b)
            return false;
    }
    return true;
}

rwlibs::simulation::SimulatedController::Ptr DynamicWorkCell::findController(const std::string& name){
    BOOST_FOREACH(rwlibs::simulation::SimulatedController::Ptr ctrl, _controllers){
        if( ctrl->getControllerName()==name )
            return ctrl;
    }
    return NULL;
}

Body::Ptr DynamicWorkCell::findBody(const std::string& name) const {
    BOOST_FOREACH(const Body::Ptr &body, _bodies){
        if(body->getName()==name)
            return body;
    }
    return NULL;
}

SimulatedSensor::Ptr DynamicWorkCell::findSensor(const std::string& name) {
	//std::cout<<"Find Sensor = "<<name<<std::endl;
	BOOST_FOREACH(SimulatedSensor::Ptr sensor, _sensors){
		if (sensor->getSensor()->getName() == name)
            return sensor;
    }
    return NULL;
}

Body::Ptr DynamicWorkCell::getBody(rw::kinematics::Frame *f){
    if(_frameToBody.find(f)==_frameToBody.end())
        return NULL;
    return _frameToBody[f];
}

void DynamicWorkCell::addBody(Body::Ptr body){
    body->registerStateData( _workcell->getStateStructure() );
    _frameToBody[body->getBodyFrame()] = body;
    _allbodies.push_back(body);
    _bodies.push_back(body);

}


