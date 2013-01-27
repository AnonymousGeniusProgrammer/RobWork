/*
* JacobianBase.cpp
*
*  Created on: Nov 22, 2012
*      Author: bing
*/

#include "JacobianBase.hpp"

namespace rwlibs {
	namespace calibration {

		JacobianBase::~JacobianBase() {

		}

		int JacobianBase::getColumnCount() const {
			if (!_calibration->isEnabled())
				return 0;
			
			return _calibration->getParameterSet().getEnabledCount();
		}

		Eigen::MatrixXd JacobianBase::computeJacobian(rw::kinematics::Frame::Ptr referenceFrame, rw::kinematics::Frame::Ptr targetFrame,
			const rw::kinematics::State& state) {
				RW_ASSERT(!referenceFrame.isNull());
				RW_ASSERT(!targetFrame.isNull());
				RW_ASSERT(getColumnCount() != 0);
				RW_ASSERT(_calibration->isApplied());
				return doComputeJacobian(referenceFrame, targetFrame, state);
		}

		void JacobianBase::takeStep(const Eigen::VectorXd& step) {
			RW_ASSERT(step.rows() == getColumnCount());
			
			const CalibrationParameterSet parameterSet = _calibration->getParameterSet();
			const int parameterCount = parameterSet.getCount();
			Eigen::VectorXd fullStep = Eigen::VectorXd(parameterCount);
			int parameterIndex, stepIndex;
			for (parameterIndex = 0, stepIndex = 0; parameterIndex < parameterCount; parameterIndex++) {
				if (parameterSet(parameterIndex).isEnabled()) {
					fullStep(parameterIndex) = step(stepIndex);
					stepIndex++;
				}
			}

			RW_ASSERT(stepIndex == step.rows());

			_calibration->revert();

			doTakeStep(fullStep);

			_calibration->apply();
		}

		JacobianBase::JacobianBase(Calibration::Ptr calibration) :
			_calibration(calibration) {

		}

		void JacobianBase::doTakeStep(const Eigen::VectorXd& step) {	
			CalibrationParameterSet parameterSet = _calibration->getParameterSet();
			const int parameterCount = parameterSet.getCount();
			for (int parameterIndex = 0; parameterIndex < parameterCount; parameterIndex++) {
				if (parameterSet(parameterIndex).isEnabled()) {
					parameterSet(parameterIndex) += step(parameterIndex);
				}
			}
			_calibration->setParameterSet(parameterSet);
		}

	}
}