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

#ifndef RWSIM_SIMULATOR_PHYSICSENGINE_HPP_
#define RWSIM_SIMULATOR_PHYSICSENGINE_HPP_

#include <rwlibs/simulation/SimulatedController.hpp>
#include <rwlibs/simulation/SimulatedSensor.hpp>

#include <rw/kinematics/State.hpp>
#include <rwsim/dynamics/RigidBody.hpp>
#include <rwsim/dynamics/DynamicDevice.hpp>

#include <rwsim/drawable/SimulatorDebugRender.hpp>

namespace rwsim {
namespace simulator {
	//! @addtogroup simulator @{

	/**
	 * @brief A general physics engine interface for simulating dynamics
	 * of objects and robot devices.
	 *
	 * The general step looks like this:;
	 *
	 * Foreach controller=_controllers
	 *  controller->update(dt,state);
     *
     * collision detection (contact detection)
	 * constraint solving
	 * update device/body positions (integration)
	 *
	 * Foreach sensor=_sensors
	 *  sensor->update(dt,state)
	 *
	 *
	 *
	 *
	 */
	class PhysicsEngine
	{
	public:

	    typedef rw::common::Ptr<PhysicsEngine> Ptr;

		/**
		 * @brief destructor
		 */
		virtual ~PhysicsEngine(){};

		/**
		 * @brief Performs a step and updates the state
		 */
		virtual void step(double dt, rw::kinematics::State& state) = 0;

		/**
		 * @brief reset velocity and acceleration of all bodies to 0. And sets the position of all bodies
		 * to that described in state
		 */
		virtual void resetScene(rw::kinematics::State& state) = 0;

		/**
		 * @brief initialize simulator physics with state
		 */
		virtual void initPhysics(rw::kinematics::State& state) = 0;

		/**
		 * @brief cleans up the allocated storage fo bullet physics
		 */
		virtual void exitPhysics() = 0;

		/**
		 * @brief gets the the current simulated time
		 */
		virtual double getTime() = 0;

		/**
		 * Enables or disables a body
		 * @param body
		 * @param enabled
		 */
		virtual void setEnabled(dynamics::Body* body, bool enabled) = 0;

		/**
		 * @brief create a debug render for the specific implementation
		 * @return NULL if no render is available else a valid render
		 */
		virtual drawable::SimulatorDebugRender* createDebugRender() = 0;

		/**
		 * @brief
		 */
		virtual rw::common::PropertyMap& getPropertyMap() = 0;

		/**
		 * @brief
		 */
		virtual void emitPropertyChanged() = 0;

		/**
		 * @brief add a simulated controller to this simulator
		 */
		virtual void addController(rwlibs::simulation::SimulatedController::Ptr controller) = 0;

		/**
		 * @brief removes a simulated controller from this simulator
		 * @param controller
		 */
		virtual void removeController(rwlibs::simulation::SimulatedController::Ptr controller) = 0;

		virtual void addBody(rwsim::dynamics::Body::Ptr body, rw::kinematics::State &state) = 0;
		virtual void addDevice(rwsim::dynamics::DynamicDevice::Ptr body, rw::kinematics::State &state) = 0;

		/**
		 * @brief add a simulated sensor to this simulator
		 */
		virtual void addSensor(rwlibs::simulation::SimulatedSensor::Ptr sensor, rw::kinematics::State &state) = 0;

		/**
		 * @brief add a simulated sensor to this simulator
		 */
		virtual void removeSensor(rwlibs::simulation::SimulatedSensor::Ptr sensor) = 0;

		/**
		 * @brief get the list of simulated sensors
		 * @return
		 */
		virtual std::vector<rwlibs::simulation::SimulatedSensor::Ptr> getSensors() = 0;
	};

	//! @}
}
}
#endif /*Simulator_HPP_*/

