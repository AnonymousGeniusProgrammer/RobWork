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


#ifndef RW_TRAJECTORY_CUBICSPLINEFACTORY_HPP
#define RW_TRAJECTORY_CUBICSPLINEFACTORY_HPP

/**
 * @file CubicSplineFactory.hpp
 */

#include "InterpolatorTrajectory.hpp"
#include "CubicSplineInterpolator.hpp"
#include "Path.hpp"

namespace rw {
namespace trajectory {

/** @addtogroup trajectory */
/*@{*/

/**
 * @brief Factory for creating cubic splines
 *
 * http://www.physics.arizona.edu/~restrepo/475A/Notes/sourcea/node35.html
 *
 */
class CubicSplineFactory
{
public:

    /**
     * @brief constructs a free/natural cubic spline
     * @param qStart [in] Start configuration to time 0.
     * @param viapoints [in] a list of viaPoints and associated times
     */
	static
	InterpolatorTrajectory<rw::math::Q>*
		makeNaturalSpline(QPathPtr qpath);

	static
	InterpolatorTrajectory<rw::math::Q>*
		makeNaturalSpline(TimedQPathPtr tqpath, double offset=0);

	/**
	 * @brief creates a clamped spline trajectory with equally spaced
	 * via points. That is time between samples is 1. A clamped spline controls
	 * the velocity in the end points. The acceleration is 0 in the end points.
	 * @param qpath [in] the path over which the spline should be generated.
	 * @param dqStart [in] the velocity in the first point
	 * @param dqEnd [in] the velocity in the last point.
	 * @return a trajectory of CubicSplineInterpolators
	 */
	static
	InterpolatorTrajectory<rw::math::Q>*
		makeClampedSpline(QPathPtr qpath,
			const rw::math::Q& dqStart,
		    const rw::math::Q& dqEnd);

	/**
	 * @brief creates a clamped spline trajectory where the timed label is used
	 * to determine the time between samples. A clamped spline controls
	 * the velocity in the end points. The acceleration is 0 in the end points.
	 * @param qpath [in] the path over which the spline should be generated.
	 * @param dqStart [in] the velocity in the first point
	 * @param dqEnd [in] the velocity in the last point.
	 * @return a trajectory of CubicSplineInterpolators
	 */
	static
	InterpolatorTrajectory<rw::math::Q>*
		makeClampedSpline(TimedQPathPtr tqpath,
			const rw::math::Q& dqStart,
		    const rw::math::Q& dqEnd,
		    double offset=0);

private:
	CubicSplineFactory();

	virtual ~CubicSplineFactory();
};


/** @} */

} //end namespace trajectory
} //end namespace rw

#endif /*RW_TRAJECTORY_CUBICSPLINEFACTORY_HPP_*/
