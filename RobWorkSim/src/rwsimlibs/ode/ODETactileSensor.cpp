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


#include "ODETactileSensor.hpp"
#include <rw/math/MetricUtil.hpp>
#include <rwsim/sensor/BodyContactSensor.hpp>

#include "ODEUtil.hpp"

using namespace rw::math;
using namespace rwsim::simulator;
using namespace rwsim::dynamics;

ODETactileSensor::ODETactileSensor(sensor::SimulatedTactileSensor *sens) :
    _rwsensor(sens)
{
}

void ODETactileSensor::addFeedback(const std::vector<dJointFeedback*>& fback,
                                   const std::vector<dContactGeom> &g,
                                   Body* body,
                                   int bodyIdx)
{
    _feedback.push_back(fback);
    _geoms.push_back(g);
    _bodyIdx.push_back(bodyIdx);
    //if(body==NULL)
    //    RW_WARN("BODY IS NULL HERE!");
    _rwBody.push_back(body);
}

void ODETactileSensor::clear()
{
    _feedback.clear();
    _geoms.clear();
    _bodyIdx.clear();
    //_contacts.clear();
    _wTa.clear();
    _wTb.clear();
    _rwBody.clear();
}

void ODETactileSensor::update(double dt, rw::kinematics::State& state)
{
    RW_ASSERT(_feedback.size() == _geoms.size());
    RW_ASSERT(_feedback.size() == _bodyIdx.size());
    RW_ASSERT(_feedback.size() == _rwBody.size());
    RW_ASSERT(_rwsensor);


    //std::cout << "update sensor"<< std::endl;
    //if( dynamic_cast<BodyContactSensor*>(_rwsensor)){

        for(size_t midx=0;midx<_feedback.size();midx++){
            int bodyIdx=_bodyIdx[midx];
            std::vector<dJointFeedback*>& feedback = _feedback[midx];
            std::vector<dContactGeom>& geoms = _geoms[midx];
             for(size_t i=0;i<feedback.size(); i++){
                 rw::math::Vector3D<> force,snormal;
                 if(bodyIdx==0){
                     force = ODEUtil::toVector3D( feedback[i]->f1 );
                     snormal = ODEUtil::toVector3D( geoms[i].normal );
                 } else {
                     force = ODEUtil::toVector3D(feedback[i]->f2);
                     snormal = -ODEUtil::toVector3D(geoms[i].normal);
                 }
                 _rwsensor->addForceW( ODEUtil::toVector3D(geoms[i].pos), force, snormal, state, _rwBody[midx]);
             }
        }
        clear();
        _rwsensor->update(dt, state);
    //    return;
    //}
/*
    for(size_t midx=0;midx<_feedback.size();midx++){
        int bodyIdx=_bodyIdx[midx];
        std::vector<dJointFeedback*>& feedback = _feedback[midx];
        std::vector<dContactGeom>& geoms = _geoms[midx];
        // calculate the total force added
        rw::math::Vector3D<> totalForce;
        rw::math::Vector3D<> gnormal;
        for (size_t i = 0; i < feedback.size(); i++) {
            if (bodyIdx == 0) {
                //std::cout << "f1" << ODEUtil::toVector3D( feedback[i]->f1 ) << std::endl;
                //std::cout << "f2" << ODEUtil::toVector3D( feedback[i]->f2 ) << std::endl;
                totalForce += ODEUtil::toVector3D(feedback[i]->f1);
                gnormal = ODEUtil::toVector3D(geoms[i].normal);
            } else {
                totalForce += ODEUtil::toVector3D(feedback[i]->f2);
                gnormal = -ODEUtil::toVector3D(geoms[i].normal);
            }
        }
        //if( rw::math::MetricUtil::norm2(totalForce)<0.00001 ){
        //    continue;
        //}

        double distSum = 0, maxDist = 0;
        std::vector<rw::math::Vector3D<> > normals;
        std::vector<rw::math::Vector3D<> > p;
        rw::proximity::MultiDistanceResult &cm = _contacts[midx];
        for (size_t i = 0; i < cm.distances.size(); i++) {
            distSum += cm.distances[i];
            maxDist = std::max(cm.distances[i], maxDist);
        }
        maxDist += 0.01;
        double dsum = maxDist * cm.distances.size() - distSum;
        //std::cout << "DistSum: " << distSum << " maxDist: " << maxDist << std::endl;
        // each force is scaled with (maxDist-dist)/distSum * totalForce
        rw::math::Vector3D<> pos, normal, force;
        for (size_t i = 0; i < cm.distances.size(); i++) {
            double dist = cm.distances[i];
            force = (maxDist - dist) / dsum * totalForce;
            //std::cout << dist << std::endl;
            //std::cout << "- force: " << force << std::endl;
            if (bodyIdx == 0) {
                pos = _wTa[midx] * cm.p1s[i];
                //normal = _wTb.R()*cm.p2s[i]-_wTa.R()*cm.p1s[i];
            } else {
                pos = _wTa[midx] * cm.p2s[i];
                //normal = _wTa.R()*cm.p1s[i]-_wTb.R()*cm.p2s[i];
            }

            _rwsensor->addForceW(pos, force, gnormal);
        }

    }
    */

/*
    for(size_t i=0;i<_feedback.size(); i++){
        Vector3D<> force, snormal;
        if(_bodyIdx[i]==0){
            force = ODEUtil::toVector3D( _feedback[i]->f1 );
            snormal = ODEUtil::toVector3D( _geoms[i].normal );
        } else {
            force = ODEUtil::toVector3D(_feedback[i]->f2);
            snormal = sign*ODEUtil::toVector3D(_geoms[i].normal);
        }
        _rwsensor->addForceW( ODEUtil::toVector3D(_geoms[i].pos), force, snormal, _rwBody[i]);
    }

    clear();
    _rwsensor->update(dt, state);
    */
}

/*
void ODETactileSensor::setContacts(
                                   const rw::proximity::MultiDistanceResult& res,
                                   rw::math::Transform3D<> wTa,
                                   rw::math::Transform3D<> wTb)
{
    //_contacts.push_back(res);
    _wTa.push_back(wTa);
    _wTb.push_back(wTb);
}
*/
