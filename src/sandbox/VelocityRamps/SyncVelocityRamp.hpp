#ifndef RW_SANDBOX_SYNCVELOCITYRAMP_HPP
#define RW_SANDBOX_SYNCVELOCITYRAMP_HPP

#include <rw/models/Device.hpp>
#include <rw/math/Q.hpp>

namespace rw {
namespace sandbox {

/**
 * @brief Provides generation of synchronized velocity ramps for a device
 *
 * The synchronized velocity ramp calculates a profile for moving linearly
 * between two configurations a quick as possible without violating the
 * dynamic limitations specified.
 */
class SyncVelocityRamp
{
public:
    /**
     * @brief Constructs a ramp generator a device
     * @param device [in] Device to construct ramp for
     */
    SyncVelocityRamp(rw::models::Device* device);

    /**
     * @brief Constructs a ramp generator with specified velocity and acceleration limits
     *
     * The limits are assumed to be symmetric and specified with the positive bound
     *
     * @param vellimits [in] Velocity limits
     * @param acclimits [in] Acceleration limits
     */
    SyncVelocityRamp(const rw::math::Q& vellimits, const rw::math::Q& acclimits);

    /**
     * @brief Destructor
     */
	virtual ~SyncVelocityRamp();

	/**
	 * @brief Specifies from and to where a ramp should be generated.
	 * @param qstart [in] The start configuration
	 * @param qend [in] The end configuration
	 */
	//TODO: Do we have a better name for this one?
	void setTarget(const rw::math::Q& qstart, const rw::math::Q& qend);

	/**
	 * @brief Returns the time it takes to run the profile
	 * @return The duration
	 */

	double duration();

	/**
	 * @brief Returns the extremum of the velocities for the joints
	 * @return Joint extremum velocities
	 */
	rw::math::Q getExtremumVelocities();

    /**
     * @brief Returns the extremum of the acceleration for the joints
     * @return Joint extremum accelerations
     */
	rw::math::Q getExtremumAccelerations();

	/**
	 * @brief Returns the configuration for time \b t
	 *
	 * If t<0 or t>duration() it returns the start and end configurations respectively.
	 *
	 * @param t [in] time
	 * @return Configuration at time \b t
	 */
	rw::math::Q x(double t);


   /**
     * @brief Returns the velocities for time \b t
     *
     * If t<0 or t>duration() it returns velocities corresponding to t=0 and t=duration(), respectively.
     *
     * @param t [in] time
     * @return Velocities at time \b t
     */
	rw::math::Q dx(double t);

   /**
     * @brief Returns the accelerations for time \b t
     *
     * If t<0 or t>duration() it returns accelerations corresponding to t=0 and t=duration(), respectively.
     *
     * @param t [in] time
     * @return Accelerations at time \b t
     */
	rw::math::Q ddx(double t);

private:
    rw::math::Q _vellimits;
    rw::math::Q _acclimits;
    rw::math::Q _taus;
    rw::math::Q _qstart;
    rw::math::Q _qend;
    double _maxtime;


    double _ws;
    double _wmax;
    double _dwmax;
    double _duration;
    double _tau_s;
    double _tau_e;

    double calcMaxTime(const rw::math::Q& dists);
    void calcRamp();



    double getStartAccTime();

    double getEndAccTime();

    double s(double t);
    double ds(double t);
    double dds(double t);

};

} //end namespace sandbox
} //end namespace rw

#endif /*RW_SANDBOX_SYNCVELOCITYRAMP_HPP*/
