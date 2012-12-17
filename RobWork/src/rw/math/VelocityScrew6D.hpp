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


#ifndef RW_MATH_VELOCITYSCREW6D_HPP
#define RW_MATH_VELOCITYSCREW6D_HPP

/**
 * @file VelocityScrew6D.hpp
 */

#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_expression.hpp>

#include <Eigen/Eigen>

#include "Transform3D.hpp"
#include "EAA.hpp"
#include "Vector3D.hpp"

namespace rw { namespace math {

    /** @addtogroup math */
    /*@{*/

    /**
     * @brief Class for representing 6 degrees of freedom velocity screws.
     *
     * \f[
     * \mathbf{\nu} =
     * \left[
     *  \begin{array}{c}
     *  v_x\\
     *  v_y\\
     *  v_z\\
     *  \omega_x\\
     *  \omega_y\\
     *  \omega_z
     *  \end{array}
     * \right]
     * \f]
     *
     * A VelocityScrew is the description of a frames linear and rotational velocity
     * with respect to some reference frame.
     *
     */
    template<class T = double>
    class VelocityScrew6D
    {
    public:		
		//! The type of the internal Eigne Vector
		typedef Eigen::Matrix<T, 6, 1> EigenVector6D;
		EigenVector6D _screw;

		/**
         * @brief Constructs a 6 degrees of freedom velocity screw
         *
         * @param vx [in] @f$ v_x @f$
         * @param vy [in] @f$ v_y @f$
         * @param vz [in] @f$ v_z @f$
         * @param wx [in] @f$ \omega_x @f$
         * @param wy [in] @f$ \omega_y @f$
         * @param wz [in] @f$ \omega_z @f$
         */
        VelocityScrew6D(T vx, T vy, T vz, T wx, T wy, T wz);

		template <class R>
		VelocityScrew6D(const Eigen::MatrixBase<R>& v) {
			if (v.cols() != 1 || v.rows() != 6)
				RW_THROW("Unable to initialize VectorND with "<<v.rows()<< " x "<<v.cols()<<" matrix");
			_screw = v;
		}

        /**
         * @brief Default Constructor. Initialized the velocity to 0
         */
		VelocityScrew6D() : _screw(EigenVector6D::Zero(6))
        {}

        /**
         * @brief Constructs a velocity screw in frame @f$ a @f$ from a
         * transform @f$\robabx{a}{b}{\mathbf{T}} @f$.
         *
         * @param transform [in] the corresponding transform.
         */
        explicit VelocityScrew6D(const Transform3D<T>& transform);

        /**
         * @brief Constructs a velocity screw from a linear and angular velocity
         *
         * @param linear [in] linear velocity
         * @param angular [in] angular velocity
         */
        VelocityScrew6D(const Vector3D<T>& linear, const EAA<T>& angular);

        /**
         * @brief Extracts the linear velocity
         *
         * @return the linear velocity
         */
        const Vector3D<T> linear() const {
            return Vector3D<T>(_screw(0), _screw(1), _screw(2));
        }

        /**
         * @brief Extracts the angular velocity and represents it using an
         * equivalent-angle-axis as @f$ \dot{\Theta}\mathbf{k} @f$
         *
         * @return the angular velocity
         */
        const EAA<T> angular() const {
            return EAA<T>(_screw(3), _screw(4), _screw(5));
        }

        /**
         * @brief Returns reference to velocity screw element
         *
         * @param index [in] index in the screw, index must be @f$ < 6 @f$.
         *
         * @return reference to velocity screw element
         */
        T& operator()(std::size_t index) {
            assert(index < 6);
            return _screw(index);
        }

        /**
         * @brief Returns const reference to velocity screw element
         *
         * @param index [in] index in the screw, index must be @f$ < 6 @f$.
         *
         * @return const reference to velocity screw element
         */
        const T& operator()(std::size_t index) const {
            assert(index < 6);
            return _screw(index);
        }

        /**
         * @brief Adds the velocity screw given as a parameter to the velocity screw.
         *
         * @param screw [in] Velocity screw to add
         *
         * @return reference to the VelocityScrew6D to support additional
         * assignments.
         */
        VelocityScrew6D<T>& operator+=(const VelocityScrew6D<T>& screw) {
            _screw += screw.e();
            return *this;
        }

        /**
         * @brief Subtracts the velocity screw given as a parameter from the
         * velocity screw.
         *
         * @param screw [in] Velocity screw to subtract
         *
         * @return reference to the VelocityScrew6D to support additional
         * assignments.
         */
        VelocityScrew6D<T>& operator-=(const VelocityScrew6D<T>& screw) {
            _screw -= screw.e();
            return *this;
        }

        /**
         * @brief Scales velocity screw with s
         *
         * @param s [in] scaling value
         *
         * @return reference to the VelocityScrew6D to support additional
         * assigments
         */
        VelocityScrew6D<T>& operator *= (T s) {
            _screw *= s;
            return *this;
        }

        /**
         * @brief Scales velocity screw and returns scaled version
         *
         * @param s [in] scaling value
         * @param screw [in] Screw to scale
         * @return Scales screw
         */
        friend const VelocityScrew6D<T> operator*(T s, const VelocityScrew6D& screw) {
            VelocityScrew6D result = screw;
            result *= s;
            return result;
        }

        /**
         * @brief Scales velocity screw and returns scaled version
         *
         * @param screw [in] Screw to scale
         * @param s [in] scaling value
         * @return Scales screw
         */
        const VelocityScrew6D<T> operator*( T s) const {
            VelocityScrew6D result = *this;
            result *= s;
            return result;
        }

        /**
         * @brief Changes frame of reference and velocity referencepoint of
         * velocityscrew: @f$ \robabx{b}{b}{\mathbf{\nu}}\to
         * \robabx{a}{a}{\mathbf{\nu}} @f$
         *
         * The frames @f$ \mathcal{F}_a @f$ and @f$ \mathcal{F}_b @f$ are
         * rigidly connected.
         *
         * @param aTb [in] the location of frame @f$ \mathcal{F}_b @f$ wrt.
         * frame @f$ \mathcal{F}_a @f$: @f$ \robabx{a}{b}{\mathbf{T}} @f$
         *
         * @param bV [in] velocity screw wrt. frame @f$ \mathcal{F}_b @f$: @f$
         * \robabx{b}{b}{\mathbf{\nu}} @f$
         *
         * @return the velocity screw wrt. frame @f$ \mathcal{F}_a @f$: @f$
         * \robabx{a}{a}{\mathbf{\nu}} @f$
         *
         * Transformation of both the velocity reference point and of the base to
         * which the VelocityScrew is expressed
         *
         * \f[
         * \robabx{a}{a}{\mathbf{\nu}} =
         * \left[
         *  \begin{array}{c}
         *  \robabx{a}{a}{\mathbf{v}} \\
         *  \robabx{a}{a}{\mathbf{\omega}}
         *  \end{array}
         * \right] =
         * \left[
         *  \begin{array}{cc}
         *    \robabx{a}{b}{\mathbf{R}} & S(\robabx{a}{b}{\mathbf{p}})
         *    \robabx{a}{b}{\mathbf{R}} \\
         *    \mathbf{0}^{3x3} & \robabx{a}{b}{\mathbf{R}}
         *  \end{array}
         * \right]
         * \robabx{b}{b}{\mathbf{\nu}} =
         * \left[
         *  \begin{array}{c}
         *    \robabx{a}{b}{\mathbf{R}} \robabx{b}{b}{\mathbf{v}} +
         *    \robabx{a}{b}{\mathbf{p}} \times \robabx{a}{b}{\mathbf{R}}
         *    \robabx{b}{b}{\mathbf{\omega}}\\
         *    \robabx{a}{b}{\mathbf{R}} \robabx{b}{b}{\mathbf{\omega}}
         *  \end{array}
         * \right]
         * \f]
         *
         */
        friend const VelocityScrew6D<T> operator*(const Transform3D<T>& aTb,
                                                  const VelocityScrew6D<T>& bV)
        {
            const Vector3D<T>& bv = bV.linear();
            const EAA<T>& bw = bV.angular();
            const EAA<T>& aw = aTb.R() * bw;
            const Vector3D<T>& av = aTb.R() * bv + cross(aTb.P(), aw);
            return VelocityScrew6D<T>(av, aw);
        }

        /**
         * @brief Changes velocity referencepoint of
         * velocityscrew: @f$ \robabx{b}{b}{\mathbf{\nu}}\to
         * \robabx{a}{a}{\mathbf{\nu}} @f$
         *
         * The frames @f$ \mathcal{F}_a @f$ and @f$ \mathcal{F}_b @f$ are
         * rigidly connected.
         *
         * @param aPb [in] the location of frame @f$ \mathcal{F}_b @f$ wrt.
         * frame @f$ \mathcal{F}_a @f$: @f$ \robabx{a}{b}{\mathbf{T}} @f$
         *
         * @param bV [in] velocity screw wrt. frame @f$ \mathcal{F}_b @f$: @f$
         * \robabx{b}{b}{\mathbf{\nu}} @f$
         *
         * @return the velocity screw wrt. frame @f$ \mathcal{F}_a @f$: @f$
         * \robabx{a}{a}{\mathbf{\nu}} @f$
         *
         * Transformation of both the velocity reference point and of the base to
         * which the VelocityScrew is expressed
         *
         * \f[
         * \robabx{a}{a}{\mathbf{\nu}} =
         * \left[
         *  \begin{array}{c}
         *  \robabx{a}{a}{\mathbf{v}} \\
         *  \robabx{a}{a}{\mathbf{\omega}}
         *  \end{array}
         * \right] =
         * \left[
         *  \begin{array}{cc}
         *    \robabx{a}{b}{\mathbf{R}} & S(\robabx{a}{b}{\mathbf{p}})
         *    \robabx{a}{b}{\mathbf{R}} \\
         *    \mathbf{0}^{3x3} & \robabx{a}{b}{\mathbf{R}}
         *  \end{array}
         * \right]
         * \robabx{b}{b}{\mathbf{\nu}} =
         * \left[
         *  \begin{array}{c}
         *    \robabx{a}{b}{\mathbf{R}} \robabx{b}{b}{\mathbf{v}} +
         *    \robabx{a}{b}{\mathbf{p}} \times \robabx{a}{b}{\mathbf{R}}
         *    \robabx{b}{b}{\mathbf{\omega}}\\
         *    \robabx{a}{b}{\mathbf{R}} \robabx{b}{b}{\mathbf{\omega}}
         *  \end{array}
         * \right]
         * \f]
         *
         */
        friend const VelocityScrew6D<T> operator*(const Vector3D<T>& aPb,
                                                  const VelocityScrew6D<T>& bV)
        {
            const Vector3D<T>& bv = bV.linear();
            const EAA<T>& bw = bV.angular();
            const Vector3D<T>& av = bv + cross(aPb, bw);
            return VelocityScrew6D<T>(av, bw);
        }

        /**
         * @brief Changes frame of reference for velocityscrew: @f$
         * \robabx{b}{i}{\mathbf{\nu}}\to \robabx{a}{i}{\mathbf{\nu}}
         * @f$
         *
         * @param aRb [in] the change in orientation between frame
         * @f$ \mathcal{F}_a @f$ and frame
         * @f$ \mathcal{F}_b @f$: @f$ \robabx{a}{b}{\mathbf{R}} @f$
         *
         * @param bV [in] velocity screw wrt. frame
         * @f$ \mathcal{F}_b @f$: @f$ \robabx{b}{i}{\mathbf{\nu}} @f$
         *
         * @return the velocity screw wrt. frame @f$ \mathcal{F}_a @f$:
         * @f$ \robabx{a}{i}{\mathbf{\nu}} @f$
         *
         * Transformation of the base to which the VelocityScrew is expressed. The velocity
         * reference point is left intact
         *
         * \f[
         * \robabx{a}{i}{\mathbf{\nu}} =
         * \left[
         *  \begin{array}{c}
         *  \robabx{a}{i}{\mathbf{v}} \\
         *  \robabx{a}{i}{\mathbf{\omega}}
         *  \end{array}
         * \right] =
         * \left[
         *  \begin{array}{cc}
         *    \robabx{a}{b}{\mathbf{R}} & \mathbf{0}^{3x3} \\
         *    \mathbf{0}^{3x3} & \robabx{a}{b}{\mathbf{R}}
         *  \end{array}
         * \right]
         * \robabx{b}{i}{\mathbf{\nu}} =
         * \left[
         *  \begin{array}{c}
         *    \robabx{a}{b}{\mathbf{R}} \robabx{b}{i}{\mathbf{v}} \\
         *    \robabx{a}{b}{\mathbf{R}} \robabx{b}{i}{\mathbf{\omega}}
         *  \end{array}
         * \right]
         * \f]
         */
        friend const VelocityScrew6D<T> operator*(const Rotation3D<T>& aRb, const VelocityScrew6D<T>& bV)
        {
            Vector3D<T> bv = bV.linear();
            EAA<T> bw = bV.angular();

            return VelocityScrew6D<T>(aRb*bv, aRb*bw);
        }

        /**
         * @brief Adds two velocity screws together @f$
         * \mathbf{\nu}_{12}=\mathbf{\nu}_1+\mathbf{\nu}_2 @f$
         *
         * @param screw1 [in] @f$ \mathbf{\nu}_1 @f$
         *
         * @param screw2 [in] @f$ \mathbf{\nu}_2 @f$
         *
         * @return the velocity screw @f$ \mathbf{\nu}_{12} @f$
         */
        const VelocityScrew6D<T> operator+(const VelocityScrew6D<T>& screw2) const
        {
            return VelocityScrew6D<T>(_screw+screw2.e());
        }

        /**
         * @brief Subtracts two velocity screws
         * \f$\mathbf{\nu}_{12}=\mathbf{\nu}_1-\mathbf{\nu}_2\f$
         *
         * \param screw1 [in] \f$\mathbf{\nu}_1\f$
         * \param screw2 [in] \f$\mathbf{\nu}_2\f$
         * \return the velocity screw \f$\mathbf{\nu}_{12} \f$
         */
        const VelocityScrew6D<T> operator-(const VelocityScrew6D<T>& screw2) const
        {
            return VelocityScrew6D<T>(_screw-screw2.e());
        }

        /**
         * @brief Ouputs velocity screw to stream
         *
         * @param os [in/out] stream to use
         * @param screw [in] velocity screw
         * @return the resulting stream
         */
        friend std::ostream& operator<<(std::ostream& os, const VelocityScrew6D<T>& screw)
        {
            return os << screw.e();
        }

        /**
         * @brief Takes the 1-norm of the velocity screw. All elements both
         * angular and linear are given the same weight.
         *
         * @param screw [in] the velocity screw
         * @return the 1-norm
         */
        friend T norm1(const VelocityScrew6D& screw)
        {
            return screw.norm1();
        }

        /**
         * @brief Takes the 1-norm of the velocity screw. All elements both
         * angular and linear are given the same weight.
         *
         * @param screw [in] the velocity screw
         * @return the 1-norm
         */
        T norm1() const {
            return _screw.lpNorm<1>();
        }


        /**
         * @brief Takes the 2-norm of the velocity screw. All elements both
         * angular and linear are given the same weight
         *
         * @param screw [in] the velocity screw
         * @return the 2-norm
         */
        friend T norm2(const VelocityScrew6D& screw)
        {
            return screw.norm2();
        }

        /**
         * @brief Takes the 2-norm of the velocity screw. All elements both
         * angular and linear are given the same weight
         *
         * @param screw [in] the velocity screw
         * @return the 2-norm
         */
        T norm2() const {
            return _screw.norm();
        }

        /**
         * @brief Takes the infinite norm of the velocity screw. All elements
         * both angular and linear are given the same weight.
         *
         * @param screw [in] the velocity screw
         *
         * @return the infinite norm
         */
        friend T normInf(const VelocityScrew6D& screw)
        {
            return screw.normInf();
        }

        /**
         * @brief Takes the infinite norm of the velocity screw. All elements
         * both angular and linear are given the same weight.
         *
         * @return the infinite norm
         */
        T normInf() const {
			return _screw.lpNorm<Eigen::Infinity>();
        }

        /**
         * @brief Casts VelocityScrew6D<T> to VelocityScrew6D<Q>
         *
         * @param vs [in] VelocityScrew6D with type T
         *
         * @return VelocityScrew6D with type Q
         */
        template<class Q>
        friend const VelocityScrew6D<Q> cast(const VelocityScrew6D<T>& vs)
        {
            return VelocityScrew6D<Q>(
                static_cast<Q>(vs(0)),
                static_cast<Q>(vs(1)),
                static_cast<Q>(vs(2)),
                static_cast<Q>(vs(3)),
                static_cast<Q>(vs(4)),
                static_cast<Q>(vs(5)));
        }

        /**
           @brief Construct a velocity screw from a Boost vector expression.
        */
        template <class R>
        explicit VelocityScrew6D(const boost::numeric::ublas::vector_expression<R>& r)            
        {
			boost::numeric::ublas::bounded_vector<T, 6> v(r);
			for (size_t i = 0; i<6; i++) {
				_screw(i) = v(i);
			}
		}


        /**
           @brief Converter to Boost velocity screw state.
         */
        boost::numeric::ublas::bounded_vector<T, 6> m() const { 
			boost::numeric::ublas::bounded_vector<T, 6> m;
			for (size_t i = 0; i<6; i++)
				m(i) = _screw(i);
			return m; 
		}

        /**
           @brief Accessor for the internal Eigen velocity screw state.
         */
		EigenVector6D& e() {
			return _screw;
		}

        /**
           @brief Accessor for the internal Eigen velocity screw state.
         */
		const EigenVector6D& e() const {
			return _screw;
		}


	private:


    };

    /*@}*/
}} // end namespaces

#endif // end include guard
