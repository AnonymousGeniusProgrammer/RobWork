%module rw

%{
#include <rwlibs/swig/ScriptTypes.hpp>
#include <rw/common/Ptr.hpp>
using namespace rwlibs::swig;
%}

%include <std_string.i>
%include <std_vector.i>

%include "carrays.i"
%array_class(double, doubleArray);

%constant double Pi = rw::math::Pi;
%constant double Inch2Meter = rw::math::Inch2Meter;
%constant double Meter2Inch = rw::math::Meter2Inch;
%constant double Deg2Rad = rw::math::Deg2Rad;
%constant double Rad2Deg = rw::math::Rad2Deg;

%include <stl.i>

namespace std {
    %template(StringVector) std::vector <string>;
};

/********************************************
 * COMMON
 */

namespace rw { namespace common {
template<class T> class Ptr {
public:
    Ptr();
    Ptr(T* ptr);
    //Ptr(boost::shared_ptr<T> ptr);
    //Ptr(std::auto_ptr<T> ptr);
    bool isShared();
    bool isNull();
    bool operator==(void* p) const;

    template<class A>
    bool operator==(const rw::common::Ptr<A>& p) const;

    T *operator->() const;
};
}}


%template (WorkCellPtr) rw::common::Ptr<WorkCell>;
%template (DevicePtr) rw::common::Ptr<Device>;
%template (DevicePtrVector) std::vector< rw::common::Ptr<Device> >;
%template (FrameVector) std::vector<Frame*>;
%template (JointVector) std::vector<Joint*>;
%template (Vector3DVector) std::vector<Vector3D>;



/**************************
 * MATH
 */

class Q
{
public:
    // first we define functions that are native to Q
	Q();
	%feature("autodoc", "1")
    Q(int n, double vals[n]);
    int size() const;

    //
    %rename(elem) operator[];
    double& operator[](unsigned int i) ;

    const Q operator-() const;
    Q operator-(const Q& b) const;
    Q operator+(const Q& b) const;
    Q operator*(double s) const;
    Q operator/(double s) const;
    double norm2();
    double norm1();
    double normInf();

    %extend {
        char *__str__() {
             static char tmp[256];
             sprintf(tmp,"%s", toString(*$self).c_str());
             return tmp;
        }
        double __getitem__(int i)const {return (*$self)[i]; }
        void __setitem__(int i,double d){ (*$self)[i] = d; }
    };

};



class Vector3D
{
public:
    Vector3D();
    %feature("autodoc", "1")
    Vector3D(double x,double y, double z);
    size_t size() const;
    Vector3D operator*(double scale) const;
    Vector3D operator+(const Vector3D& other) const;
    Vector3D operator-(const Vector3D& other) const;
    bool operator==(const Vector3D& q);

    double norm2();
    double norm1();
    double normInf();

    //double& operator[](unsigned int i) ;
    %rename(elem) operator[];

    %extend {
        char *__str__() {
             static char tmp[256];
             sprintf(tmp,"%s", toString(*$self).c_str());
             return tmp;
        }
        double __getitem__(int i)const {return (*$self)[i]; }
        void __setitem__(int i,double d){ (*$self)[i] = d; }
    };

};


class Rotation3D
{
public:
    // Lua methods:
    Rotation3D();
    %feature("autodoc", "1")
    Rotation3D(double v0,double v1,double v2,
    			double v3,double v4,double v5,
    			double v6,double v7,double v8);
    			
    explicit Rotation3D(const Rotation3D& R);

    Rotation3D operator*(const Rotation3D& other) const;
    Vector3D operator*(const Vector3D& vec) const;
    //Rotation3D inverse() const;

    static const Rotation3D& identity();
    static Rotation3D skew(const Vector3D& v);
    bool equal(const Rotation3D& rot, double precision);

    //EAA operator*(const EAA& other) const;

    bool operator==(const Rotation3D &rhs) const;
    //double& operator[](unsigned int i) ;

    // std::string __tostring() const;
};

%extend Rotation3D {
    char *__str__() {
         static char tmp[256];
         sprintf(tmp,"%s", toString(*$self).c_str());
         return tmp;
    }

   double __getitem__(int x,int y)const {
       return (*$self)(x,y);
   }
   void __setitem__(int x, int y, double d)
   {
    (*$self)(x,y) = d;
    }
};


//Rotation3D inverse(const Rotation3D& val);

class EAA
{
public:
    // Lua methods:
    EAA();
    %feature("autodoc", "1")
    EAA(const EAA& eaa);
    %feature("autodoc", "1")
    EAA(const Rotation3D& rot);
    %feature("autodoc", "1")
    EAA(const Vector3D& axis, double angle);
    %feature("autodoc", "1")
    EAA(double thetakx, double thetaky, double thetakz);
    %feature("autodoc", "1")
    EAA(const Vector3D& v1, const Vector3D& v2);

    double angle() const;
    Vector3D axis() const;

    //const double& operator[](unsigned int i) const;
	//double& operator[](unsigned int i);
	%rename(elem) operator[];

    //EAA operator*(const Rotation3D& other) const;

    Rotation3D toRotation3D() const;

    //bool operator==(const EAA &rhs) const;
    // std::string __tostring() const;


};

%extend EAA {
    char *__str__() {
         static char tmp[256];
         sprintf(tmp,"%s", toString(*$self).c_str());
         return tmp;
    }
   double __getitem__(int i)const {
       return (*$self)[i];
   }
   void __setitem__(int i,double d)
   {
    (*$self)[i] = d;
    }

};


class RPY
{
public:
    // Lua methods:
    RPY();
    RPY(const RPY& eaa);
    RPY(const Rotation3D& rot);
    RPY(double roll, double pitch, double yaw);
    Rotation3D toRotation3D() const;
    %extend {
        char *__str__() {
             static char tmp[256];
             sprintf(tmp,"%s", toString(*$self).c_str());
             return tmp;
        }
        double __getitem__(int i)const { return (*$self)(i); }
        void __setitem__(int i,double d){ (*$self)(i) = d; }
    };
};


class Quaternion
{
public:
    // Lua methods:
    Quaternion();
    Quaternion(const Quaternion& eaa);
    Quaternion(const Rotation3D& rot);
    Quaternion operator*(double s);

    void normalize();

    Rotation3D toRotation3D() const;
    Quaternion slerp(const Quaternion& v, const double t) const;

    double getQx() const;
    double getQy() const;
    double getQz() const;
    double getQw() const;
    %extend {
        std::string __tostring() { return toString<Quaternion>(*$self); }
        double __getitem__(int i)const { return (*$self)(i); }
        void __setitem__(int i,double d) { (*$self)(i) = d; }
    };

};



//Quaternion operator*(double s, const Quaternion& v);

class Transform3D {
public:
	Transform3D();
    Transform3D(const Transform3D& t3d);
    Transform3D(const Vector3D& position,const Rotation3D& rotation);

    Transform3D operator*(const Transform3D& other) const;
    Vector3D operator*(const Vector3D& other) const;

    static Transform3D DH(double alpha, double a, double d, double theta);
    static Transform3D craigDH(double alpha, double a, double d, double theta);
	
    Vector3D& P();
    Rotation3D& R();

    %extend {
       char *__str__() {
            static char tmp[256];
            sprintf(tmp,"%s", toString(*$self).c_str());
            return tmp;
       }
       Transform3D inverse(){ return inverse(*$self); }
       //Transform3D inverse(const Transform3D& val);
    };
};


class Pose6D {
public:
	Pose6D(const Pose6D& p6d);
    Pose6D(const Vector3D& position,const EAA& rotation);
    Pose6D(const Transform3D& t3d);

    Transform3D toTransform3D();
    %extend {
        char *__str__() {
             static char tmp[256];
             sprintf(tmp,"%s", toString(*$self).c_str());
             return tmp;
        }
    };
};


class Jacobian
{
public:
    Jacobian(int m, int n);

    int size1() const ;
    int size2() const ;

    double& elem(int i, int j);

    %extend {
        char *__str__() {
             static char tmp[256];
             sprintf(tmp,"%s", toString(*$self).c_str());
             return tmp;
        }
    };
};



class VelocityScrew6D
{
public:
	VelocityScrew6D();
	VelocityScrew6D(const VelocityScrew6D& p6d);
    VelocityScrew6D(const Vector3D& position,const EAA& rotation);
    VelocityScrew6D(const Transform3D& t3d);

    // lua functions
    VelocityScrew6D operator*(double scale) const;
    VelocityScrew6D operator+(const VelocityScrew6D& other) const;
    VelocityScrew6D operator-(const VelocityScrew6D& other) const;
    //bool operator==(const VelocityScrew6D& q);

    double norm2();
    double norm1();
    double normInf();

    %extend {
        char *__str__() {
             static char tmp[256];
             sprintf(tmp,"%s", toString(*$self).c_str());
             return tmp;
        }
    };


    //Transform3D toTransform3D();
    // std::string __tostring() const;
};

/********************************************************************
// now we go on to geometry
*********************************************************************/


%template(GeometryDataPtr) rw::common::Ptr<GeometryData>;
%template(TriMeshPtr) rw::common::Ptr<TriMesh>;

class GeometryData {
    typedef enum {PlainTriMesh,
                  IdxTriMesh,
                  SpherePrim, BoxPrim, OBBPrim, AABBPrim,
                  LinePrim, PointPrim, PyramidPrim, ConePrim,
                  TrianglePrim, CylinderPrim, PlanePrim, RayPrim,
                  UserType} GeometryType;

    virtual GeometryType getType() const = 0;
    virtual rw::common::Ptr<TriMesh> getTriMesh(bool forceCopy=true) = 0;
    static std::string toString(GeometryType type);
};



class TriMesh: public GeometryData {
public:
    virtual Triangle getTriangle(size_t idx) const = 0;
    virtual void getTriangle(size_t idx, Triangle& dst) const = 0;
    virtual void getTriangle(size_t idx, Trianglef& dst) const = 0;
    virtual size_t getSize() const = 0;
    virtual size_t size() const = 0;
    virtual rw::common::Ptr<TriMesh> clone() const = 0;
    rw::common::Ptr<TriMesh> getTriMesh(bool forceCopy=true);
    //rw::common::Ptr<const TriMesh> getTriMesh(bool forceCopy=true) const;
};


class Primitive: public GeometryData {
public:
    rw::common::Ptr<TriMesh> getTriMesh(bool forceCopy=true);
    virtual rw::common::Ptr<TriMesh> createMesh(int resolution) const = 0;
    virtual Q getParameters() const = 0;
};

class Sphere: public Primitive {
public:
    //! constructor
    Sphere(const Q& initQ);
    Sphere(double radi):_radius(radi);
    double getRadius();
    rw::common::Ptr<TriMesh> createMesh(int resolution) const;
    Q getParameters() const;
    GeometryData::GeometryType getType() const;
};

class Box: public Primitive {
public:
    Box();
    Box(double x, double y, double z);
    Box(const Q& initQ);
    rw::common::Ptr<TriMesh> createMesh(int resolution) const;
    Q getParameters() const;
    GeometryType getType() const;
};

class Cone: public Primitive {
public:
    Cone(const Q& initQ);
    Cone(double height, double radiusTop, double radiusBot);
    double getHeight();
    double getTopRadius();
    double getBottomRadius();
    rw::common::Ptr<TriMesh> createMesh(int resolution) const;
    Q getParameters() const;
    GeometryType getType() const;
};

class Plane: public Primitive {
public:
    Plane(const Q& q);
    Plane(const Vector3D& n, double d);
    Plane(const Vector3D& p1,
          const Vector3D& p2,
          const Vector3D& p3);

    Vector3D& normal();
    //const Vector3D& normal() const;
    double& d();
    //double d() const;
    double distance(const Vector3D& point);
    double refit( std::vector<Vector3D >& data );
    rw::common::Ptr<TriMesh> createMesh(int resolution) const ;
    Q getParameters() const;
    GeometryType getType() const{ return PlanePrim; };
};

class ConvexHull3D {
public:
    virtual void rebuild(const std::vector<Vector3D>& vertices) = 0;
    virtual bool isInside(const Vector3D& vertex) = 0;
    virtual double getMinDistInside(const Vector3D& vertex) = 0;
    virtual double getMinDistOutside(const Vector3D& vertex) = 0;
    virtual PlainTriMeshN1* toTriMesh() = 0;
};

%template(GeometryPtr) rw::common::Ptr<Geometry>;
//typedef rw::common::Ptr<Geometry> GeometryPtr;

class Geometry {
public:
    Geometry(rw::common::Ptr<GeometryData> data, double scale=1.0);

    Geometry(rw::common::Ptr<GeometryData> data,
             const Transform3D& t3d,
             double scale=1.0);

    double getScale() const;
    void setScale(double scale);
    void setTransform(const Transform3D& t3d);
    const Transform3D& getTransform() const;
    rw::common::Ptr<GeometryData> getGeometryData();
    const rw::common::Ptr<GeometryData> getGeometryData() const;
    void setGeometryData(rw::common::Ptr<GeometryData> data);
    const std::string& getName() const;
    const std::string& getId() const;
    void setName(const std::string& name);
    void setId(const std::string& id);
    static rw::common::Ptr<Geometry> makeSphere(double radi);
    static rw::common::Ptr<Geometry> makeBox(double x, double y, double z);
    static rw::common::Ptr<Geometry> makeCone(double height, double radiusTop, double radiusBot);
    static rw::common::Ptr<Geometry> makeCylinder(float radius, float height);
};

%template(PlainTriMeshN1fPtr) rw::common::Ptr<PlainTriMeshN1f>;

class STLFile {
public:
    static void save(const TriMesh& mesh, const std::string& filename);
    static rw::common::Ptr<PlainTriMeshN1f> load(const std::string& filename);
};

/**************************************************************************
 *  KINEMATICS
 *
 *************************************************************************/

class StateData {
protected:
    StateData(int size, const std::string& name);
public:
    const std::string& getName();
    int size() const;
    double* getData(State& state);
    void setData(State& state, const double* vals) const;
};

class Frame : public StateData
{
public:

    Transform3D getTransform(const State& state) const;
    const PropertyMap& getPropertyMap() const ;
    PropertyMap& getPropertyMap();
    int getDOF() const ;
    const Frame* getParent() const ;
    Frame* getParent() ;
    Frame* getParent(const State& state);
    const Frame* getParent(const State& state) const;
    const Frame* getDafParent(const State& state) const;
    Frame* getDafParent(const State& state);
/*
    typedef rw::common::ConcatVectorIterator<Frame> iterator;
    typedef rw::common::ConstConcatVectorIterator<Frame> const_iterator;

    typedef std::pair<iterator, iterator> iterator_pair;
    typedef std::pair<const_iterator, const_iterator> const_iterator_pair;

    const_iterator_pair getChildren() const;
    iterator_pair getChildren();
    const_iterator_pair getChildren(const State& state) const;
    iterator_pair getChildren(const State& state);
    const_iterator_pair getDafChildren(const State& state) const;
    iterator_pair getDafChildren(const State& state);
*/
    void attachTo(Frame* parent, State& state);
    bool isDAF();

private:
    // Frames should not be copied.
    Frame(const Frame&);
    Frame& operator=(const Frame&);
};

/**************************************************************************
 *  SENSOR
 *
 *************************************************************************/



/**************************************************************************
 *  MODELS
 *
 *************************************************************************/
class WorkCell {
public:
    WorkCell(const std::string& name);
    std::string getName() const;
    Frame* getWorldFrame() const;
    void addDevice(rw::common::Ptr<Device> device);
    const std::vector<rw::common::Ptr<Device> >& getDevices() const;
    Frame* findFrame(const std::string& name) const;
    std::vector<Frame*> getFrames() const;
    rw::common::Ptr<Device> findDevice(const std::string& name) const;
    State getDefaultState() const;

    //rw::common::Ptr<StateStructure> getStateStructure();

    PropertyMap& getPropertyMap();
private:
    WorkCell(const WorkCell&);
    WorkCell& operator=(const WorkCell&);
};

class Device
{
public:
    typedef std::pair<Q, Q> QBox;

    Device(const std::string& name);
    //void registerStateData(rw::kinematics::StateStructure::Ptr sstruct);
    virtual void setQ(const Q& q, State& state) const = 0;
    virtual Q getQ(const State& state) const = 0;
    virtual QBox getBounds() const = 0;
    virtual Q getVelocityLimits() const = 0;
    virtual void setVelocityLimits(const Q& vellimits) = 0;
    virtual Q getAccelerationLimits() const = 0;
    virtual void setAccelerationLimits(const Q& acclimits) = 0;
    virtual size_t getDOF() const = 0;
    std::string getName() const;
    void setName(const std::string& name);
    virtual Frame* getBase() = 0;
    virtual const Frame* getBase() const = 0;
    virtual Frame* getEnd() = 0;
    virtual const Frame* getEnd() const = 0;
    Transform3D baseTframe(const Frame* f, const State& state) const;
    Transform3D baseTend(const State& state) const;
    Transform3D worldTbase(const State& state) const;
    virtual Jacobian baseJend(const State& state) const = 0;
    virtual Jacobian baseJframe(const Frame* frame,const State& state) const;
    virtual Jacobian baseJframes(const std::vector<Frame*>& frames,const State& state) const;
    //virtual rw::common::Ptr<JacobianCalculator> baseJCend(const kinematics::State& state) const;
    //virtual JacobianCalculatorPtr baseJCframe(const kinematics::Frame* frame, const kinematics::State& state) const;
    //virtual JacobianCalculatorPtr baseJCframes(const std::vector<kinematics::Frame*>& frames, const kinematics::State& state) const = 0;
private:
    Device(const Device&);
    Device& operator=(const Device&);
};


class JointDevice: public Device
{
public:
    const std::vector<Joint*>& getJoints() const;
    void setQ(const Q& q, State& state) const;
    Q getQ(const State& state) const;
    size_t getDOF() const;
    std::pair<Q, Q> getBounds() const;
    void setBounds(const std::pair<Q, Q>& bounds);
    Q getVelocityLimits() const;
    void setVelocityLimits(const Q& vellimits);
    Q getAccelerationLimits() const;
    void setAccelerationLimits(const Q& acclimits);
    Jacobian baseJend(const State& state) const;

    //JacobianCalculatorPtr baseJCframes(const std::vector<kinematics::Frame*>& frames,
    //                                   const kinematics::State& state) const;

    Frame* getBase();
    const Frame* getBase() const;
    virtual Frame* getEnd();
    virtual const Frame* getEnd() const;

};

/******************************************************************************
 *  PROXIMITY
 *
 * *************************************************************************/


/******************************************************************************
 *  LOADERS
 *
 * *************************************************************************/

class WorkCellLoader{
public:
    static rw::common::Ptr<WorkCell> load(const std::string& filename);
private:
    WorkCellLoader();
};
%template(ImagePtr) rw::common::Ptr<Image>;
class ImageFactory{
public:
    static rw::common::Ptr<Image> load(const std::string& filename);
private:
    ImageFactory();
};

class XMLTrajectoryLoader
{
public:
    XMLTrajectoryLoader(const std::string& filename, const std::string& schemaFileName = "");
    XMLTrajectoryLoader(std::istream& instream, const std::string& schemaFileName = "");

    enum Type { QType = 0, Vector3DType, Rotation3DType, Transform3DType};
    Type getType();
    rw::common::Ptr<rw::trajectory::QTrajectory> getQTrajectory();
    rw::common::Ptr<rw::trajectory::Vector3DTrajectory> getVector3DTrajectory();
    rw::common::Ptr<rw::trajectory::Rotation3DTrajectory> getRotation3DTrajectory();
    rw::common::Ptr<rw::trajectory::Transform3DTrajectory> getTransform3DTrajectory();
};

class XMLTrajectorySaver
{
public:
    static bool save(const rw::trajectory::QTrajectory& trajectory, const std::string& filename);
    static bool save(const rw::trajectory::Vector3DTrajectory& trajectory, const std::string& filename);
    static bool save(const rw::trajectory::Rotation3DTrajectory& trajectory, const std::string& filename);
    static bool save(const rw::trajectory::Transform3DTrajectory& trajectory, const std::string& filename);
    static bool write(const rw::trajectory::QTrajectory& trajectory, std::ostream& outstream);
    static bool write(const rw::trajectory::Vector3DTrajectory& trajectory, std::ostream& outstream);
    static bool write(const rw::trajectory::Rotation3DTrajectory& trajectory, std::ostream& outstream);
    static bool write(const rw::trajectory::Transform3DTrajectory& trajectory, std::ostream& outstream);
private:
    XMLTrajectorySaver();
};




/******************************************************************************
 *  LOADERS
 *
 * *************************************************************************/

%template(InvKinSolverPtr) rw::common::Ptr<InvKinSolver>;
class InvKinSolver
{
public:
    virtual std::vector<Q> solve(const Transform3D& baseTend, const State& state) const = 0;
    virtual void setCheckJointLimits(bool check) = 0;
};

%template(IterativeIKPtr) rw::common::Ptr<IterativeIK>;
class IterativeIK: public InvKinSolver
{
public:
    virtual void setMaxError(double maxError);

    virtual double getMaxError() const;

    virtual void setMaxIterations(int maxIterations);

    virtual int getMaxIterations() const;

    virtual PropertyMap& getProperties();

    virtual const PropertyMap& getProperties() const;

    static rw::common::Ptr<IterativeIK> makeDefault(rw::common::Ptr<Device> device, const State& state);
};


class JacobianIKSolver : public IterativeIK
{
public:
    typedef enum{Transpose, SVD, DLS, SDLS} JacobianSolverType;

    JacobianIKSolver(rw::common::Ptr<Device> device, const State& state);

    JacobianIKSolver(rw::common::Ptr<Device> device, Frame *foi, const State& state);

    std::vector<Q> solve(const Transform3D& baseTend, const State& state) const;

    void setInterpolatorStep(double interpolatorStep);

     void setEnableInterpolation(bool enableInterpolation);

     bool solveLocal(const Transform3D &bTed,
                     double maxError,
                     State &state,
                     int maxIter) const;

     void setClampToBounds(bool enableClamping);

     void setSolverType(JacobianSolverType type);

     void setCheckJointLimits(bool check);

};


//typedef rw::invkin::IterativeMultiIK IterativeMultiIK;
//typedef rw::invkin::JacobianIKSolverM JacobianIKSolverM;
//typedef rw::invkin::IKMetaSolver IKMetaSolver;
%template(CollisionDetectorPtr) rw::common::Ptr<CollisionDetector>;

class IKMetaSolver: public IterativeIK
{
public:
    IKMetaSolver(rw::common::Ptr<IterativeIK> iksolver,
        const rw::common::Ptr<Device> device,
        rw::common::Ptr<CollisionDetector> collisionDetector = NULL);

    //IKMetaSolver(rw::common::Ptr<IterativeIK> iksolver,
    //    const rw::common::Ptr<Device> device,
    //    rw::common::Ptr<QConstraint> constraint);

    std::vector<Q> solve(const Transform3D& baseTend, const State& state) const;

    void setMaxAttempts(size_t maxAttempts);

    void setStopAtFirst(bool stopAtFirst);

    void setProximityLimit(double limit);

    void setCheckJointLimits(bool check);

    std::vector<Q> solve(const Transform3D& baseTend, const State& state, size_t cnt, bool stopatfirst) const;

};


%template(ClosedFormIKPtr) rw::common::Ptr<ClosedFormIK>;

class ClosedFormIK: public InvKinSolver
{
public:
    static rw::common::Ptr<ClosedFormIK> make(const Device& device, const State& state);
};


class PieperSolver: public ClosedFormIK {
public:
    PieperSolver(const std::vector<DHParameterSet>& dhparams,
                 const Transform3D& joint6Tend,
                 const Transform3D& baseTdhRef = Transform3D::identity());

    PieperSolver(SerialDevice& dev, const Transform3D& joint6Tend, const State& state);

    virtual std::vector<Q> solve(const Transform3D& baseTend, const State& state) const;

    virtual void setCheckJointLimits(bool check);

};