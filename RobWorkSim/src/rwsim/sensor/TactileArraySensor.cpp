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

#include "TactileArraySensor.hpp"

#include <rw/math/Vector2D.hpp>
#include <rw/math/Vector3D.hpp>
#include <rw/kinematics/Kinematics.hpp>
#include <rw/math/MetricUtil.hpp>
#include <rw/math/Math.hpp>

#include <rw/sensor/Contact3D.hpp>

#include <rw/geometry.hpp>
#include <rw/common.hpp>
#include <rw/proximity/Proximity.hpp>

#include <boost/foreach.hpp>

using namespace rw::math;
using namespace rw::sensor;
using namespace rw::kinematics;
using namespace boost::numeric;
using namespace rw::proximity;
using namespace rw::geometry;
using namespace rw::common;
using namespace rwsim::dynamics;
using namespace rwsim::sensor;

#define MIN_CONTACT_FORCE 0.1

namespace {

    /**
     * @brief gets the average value of a 3x3 vertex matrix and the associated
     * tactile values around the coordinate (i,j).
     * @return the max tactile value and the calculatedcontact point
     */
    std::pair<double,Vector3D<> > getWeightAverage(
                          size_t i, size_t j,
                          const TactileArraySensor::ValueMatrix& tMatrix,
                          const TactileArraySensor::VertexMatrix& vMatrix)
    {
    	size_t i_low = i-1,i_upp=i+1;
        if(i_low<0) i_low = 0;
        if(i_upp>=(size_t)tMatrix.size1()) i_upp = tMatrix.size1()-1;

        int j_low = j-1,j_upp=j+1;
        if(j_low<0) j_low = 0;
        if(j_upp>=tMatrix.size2()) j_upp = tMatrix.size2()-1;

        Vector3D<> wp(0,0,0);
        double valSum = 0, maxVal=0;
        for(size_t k=i_low;k<=i_upp;k++)
            for(size_t l=j_low;l<=j_upp;l++){
                wp += vMatrix[k][l]*tMatrix(k,l);
                valSum += tMatrix(k,l);
                maxVal = std::max(tMatrix(k,l), (float)maxVal);
            }
        return std::make_pair(maxVal, wp/valSum);
    }

    /**
     * @brief the penetration is calculated using a simple linearized model
     * of the sensor. The penetration depth is calculated as linear with the
     * force and in the unit mm
     */
    double calcPenetration(
        size_t i, size_t j,
        const ublas::matrix<float>& tMatrix)
    {
        // the pen depth depends on force like this pendepth = 0.00025 f - 0.0625
        return 0.00025*tMatrix(i,j) - 0.0625;
    }
    typedef boost::multi_array<double, 2> array_type;
/*

    boost::array<array_type::index, 2> getShape(const rwlibs::sensors::TactileMatrix& tMatrix){
        boost::array<array_type::index, 2> shape = {{ tMatrix.getMatrix().size1(), tMatrix.getMatrix().size2() }};
        return shape;
    }
*/
    boost::array<array_type::index, 2> getShape(const TactileArraySensor::ValueMatrix& tMatrix,int x, int y){
        boost::array<array_type::index, 2> shape = {{ tMatrix.size1() +x, tMatrix.size2()+y }};
        return shape;
    }

    // this method takes a grid of distances connected to each cell and the corresponding
    // positions given in the
    /*
    void triangulateGrid(ublas::matrix<double>& dist,
                         const boost::multi_array<DistPoint, 2>& points)
    {
        // first we extrapolate any holes that might be in the
    }
    */

}

TactileArraySensor::TactileArraySensor(const std::string& name,
    rw::kinematics::Frame* frame,
    const rw::math::Transform3D<>& fThmap,
    const ValueMatrix& heightMap,
    const rw::math::Vector2D<>& texelSize):
        TactileArray(name),
        _centerMatrix(getShape(heightMap,-1,-1)),
        _normalMatrix(getShape(heightMap,-1,-1)),
        _contactMatrix(getShape(heightMap,-1,-1)),
        _distCenterMatrix(getShape(heightMap,-1,-1)),
        _distMatrix(ublas::zero_matrix<float>(heightMap.size1()-1,heightMap.size2()-1)),
        _distDefMatrix(ublas::zero_matrix<float>(heightMap.size1()-1,heightMap.size2()-1)),
        _accForces(ublas::zero_matrix<float>(heightMap.size1()-1,heightMap.size2()-1)),
        _pressure(ublas::zero_matrix<float>(heightMap.size1()-1,heightMap.size2()-1)),
        _texelSize(texelSize),
        _texelArea(texelSize(0)*texelSize(1)),
        _fThmap(fThmap),
        _hmapTf(inverse(fThmap)),
        _w(heightMap.size1()-1),
        _h(heightMap.size2()-1),
        _vMatrix(getShape(heightMap, 0, 0)),
        _minPressure(0),
        _maxPressure(250),
        _dmask(5*3,5*3),
        _narrowStrategy(new rwlibs::proximitystrategies::ProximityStrategyPQP()),
        _maxPenetration(0.0015),
        _elasticity(700),// KPa ~ 0.0008 GPa
        _tau(0.1),
        _accTime(0),
        _stime(0.005)
{
	this->attachTo(frame);
    // calculate the normals and centers of all texels
    // first calculate the 3D vertexes of the grid from the heightmap specification
    double tw = _texelSize(0), th = _texelSize(1);
    for(int y=0;y<_h+1; y++){
        for(int x=0;x<_w+1; x++){
            _vMatrix[x][y](0) = x*tw;
            _vMatrix[x][y](1) = y*th;
            _vMatrix[x][y](2) = heightMap(x,y);
        }
    }

    for(int j=0;j<_h; j++){
     	for(int i=0;i<_w; i++){
            Vector3D<> p = _vMatrix[i][j]  +_vMatrix[i+1][j]+
                           _vMatrix[i][j+1]+_vMatrix[i+1][j+1];
            _centerMatrix[i][j] = p/4;
            // now calculate unit normal
            Vector3D<> n = cross(_vMatrix[i+1][j]-_vMatrix[i][j],
                                 _vMatrix[i+1][j+1]-_vMatrix[i][j] );
            _normalMatrix[i][j] = normalize(n);
        }
    }

    // calculate the distribution matrix, let it span 4x4 texels
    _maskWidth = 3*_texelSize(0);
    _maskHeight = 3*_texelSize(1);

    // we use the distribution function 1/(1+x^2+y^2) where x and y is described
    // relative to the center of the mask
    // the resolution of the mask is 5 points per texel which yields a mask of 20x20
    _dmask = ublas::zero_matrix<float>(5*3,5*3);

    double dmaskSum = 0;
    double cx = 1.0/2.0;
    double cy = 1.0/2.0;

    double tSize1 = 1.0/_dmask.size1();
    double tSize2 = 1.0/_dmask.size2();

    for(size_t i=0;i<_dmask.size1();i++){
        for(size_t j=0;j<_dmask.size2();j++){
            double x = i*tSize1-cx;
            double y = j*tSize2-cy;
            double r = 4.0*sqrt(x*x+y*y);
            double val = -0.3476635514018692+1.224299065420561/(1.0+r*r);
            _dmask(i,j) = std::max(val,0.0);
            dmaskSum += std::max(val,0.0);
            std::cout << r << ",";
        }
    }

    _dmask = _dmask/dmaskSum;

    // create a geometry of the normals

    const TactileArray::VertexMatrix& normals = _normalMatrix;
    const TactileArray::VertexMatrix& centers = _centerMatrix;

    Vector3D<> offsetDir = (centers[1][0]-centers[0][0])/5;
    //std::cout << "offesetDir: " << offsetDir << std::endl;

    PlainTriMesh<Triangle<> > *trimesh = new PlainTriMesh<Triangle<> >(_h*_w);
    for(int x=0;x<_w;x++){
    	for(int y=0;y<_h;y++){
    		int i=x*_h+y;
    		(*trimesh)[i][0] = centers[x][y];
    		(*trimesh)[i][1] = centers[x][y]+offsetDir;
    		(*trimesh)[i][2] = centers[x][y]+normals[x][y]*0.008;
        }
    }

    _ntrimesh = ownedPtr(trimesh);
    _ngeom = ownedPtr( new Geometry( _ntrimesh ));
    _nmodel = _narrowStrategy->createModel();

    _narrowStrategy->addGeometry(_nmodel.get(), *_ngeom );
    _narrowStrategy->addModel(this->getFrame());
	_frameToGeoms[this->getFrame()] = Proximity::getGeometry(this->getFrame());
	std::vector<GeometryPtr> &geoms = _frameToGeoms[this->getFrame()];
    _narrowStrategy->setFirstContact(false);
    //std::cout << "DMask: " << _dmask << std::endl;
    //std::cout << "Finger pad dimensions: (" << _texelSize(0)*(_w+1) << "," << (_h+1)*_texelSize(1) <<")" <<  std::endl;

    /// now build the map of contacts between normals and surface of finger

	Transform3D<> wTb = Transform3D<>::identity();
	//ProximityModelPtr modelA = _narrowStrategy->getModel(tframe);
	ProximityModelPtr model = _narrowStrategy->getModel(this->getFrame());
	CollisionData data;
	_narrowStrategy->collides(_nmodel, _fThmap, model, wTb, data);
	if(data._collidePairs.size()>0){
		int bodyGeomId = data._collidePairs[0].geoIdxB;
		//std::cout << "BodyGeomId: " << std::endl;
		TriMesh *mesh = dynamic_cast<TriMesh*> (geoms[bodyGeomId]->getGeometryData().get());
		if(mesh){
			for(int i = 0; i<data._collidePairs[0]._geomPrimIds.size();i++){
				std::pair<int,int> &pids = data._collidePairs[0]._geomPrimIds[i];
				//std::cout << "Colliding pairs: " << pids.first << " <---> " << pids.second << std::endl;
				RW_ASSERT(0<=pids.first);
				RW_ASSERT(pids.first<_ntrimesh->getSize());

				// for each colliding pair we find the closest intersection
				// get the triangle
				Triangle<> triA = _ntrimesh->getTriangle(pids.first);
				Triangle<> tri = mesh->getTriangle(pids.second);
/*
				std::cout << "POINTS1:"
						  << "\n  " <<  triA[0]
						  << "\n  " <<  triA[1]
						  << "\n  " <<  triA[2] << std::endl;

				std::cout << "POINTS2:"
						  << "\n  " <<  data._aTb*tri[0]
						  << "\n  " <<  data._aTb*tri[1]
						  << "\n  " <<  data._aTb*tri[2] << std::endl;
*/
				Vector3D<> point;
				if( !IntersectUtil::intersetPtRayPlane(triA[0], triA[2], data._aTb*tri[0], data._aTb*tri[1], data._aTb*tri[2], point) )
					continue;

				// now we have the point of intersection, now save it in the contact array
				// if its closer than the existing point

				double ndist = MetricUtil::dist2( triA[0], point);
				//std::cout << "Intersect: " << point << std::endl;
				//std::cout << "NDist: " << ndist << std::endl;
				if( ndist < (&_distMatrix(0,0))[pids.first] )
					continue;
				(&_distDefMatrix(0,0))[pids.first] = ndist;
				(&_contactMatrix[0][0])[pids.first] = point;
			}
		}
	}
	//std::cout << "Dist" << _distDefMatrix << std::endl;


}
/*
TactileArraySensor::TactileArraySensor(const VertexMatrix& vMatrix):
        _centerMatrix(getShape(tMatrix)),-1,-1),
        _normalMatrix(getShape(tMatrix),-1,-1),
        _w(tMatrix.getMatrix().size1()+1),
        _h(tMatrix.getMatrix().size2()+1),
        _vMatrix(vMatrix)
{
    // calculate center and normal of each tactil
    for(int i=0;i<_w-1; i++){
        for(int j=0;j<_h-1; j++){
            Vector3D<> p = vMatrix[i][j]+vMatrix[i+1][j]+vMatrix[i][j+1]+vMatrix[i+1][j+1];
            _centerMatrix[i][j] = p/4;
            // now calculate unit normal
            Vector3D<> n = cross(vMatrix[i+1][j]-vMatrix[i][j],
                                 vMatrix[i+1][j+1]-vMatrix[i][j] );
            _normalMatrix[i][j] = normalize(n);
        }
    }
}
*/



namespace {

    double getValueOfTexel(double tx, double ty, // texel position data
						   double tw, double th, // texel width
                           double cx, double cy,
                           ublas::matrix<float>& distM, double mwidth, double mheight)
    {
        // all values in distM that is inside the bounds of the texel need to be summed
        float texelVal = 0;

        // the distM matrix is centered around the coordinate (cx,cy)
        //double mwidth = tw*distM.size1();
        //double mheight = th*distM.size2();
        // transforming texel coordinates to
        double x = mwidth/2+(tx-cx);
        double y = mheight/2+(ty-cy);
        //std::cout << mwidth/2 << "+("<<tx<<"-"<<cx<<")" << std::endl;
        //std::cout << mheight/2 << "+("<<ty<<"-"<<cy<<")" << std::endl;
        //std::cout << "TexCoords: " << x << " , "  << y << std::endl;

        // now calculate the start and end index
        // the point must lie inside the texel so we round up
        int xStartIdx = (int)ceil( x/mwidth*distM.size1() );
        int yStartIdx = (int)ceil( y/mheight*distM.size2() );

        int xEndIdx = (int)ceil( (x+tw)/mwidth*distM.size1() );
        int yEndIdx = (int)ceil( (y+tw)/mheight*distM.size2() );

        // now make sure we don't exeed the matrix bounds
        xStartIdx = std::max(xStartIdx,0);
        yStartIdx = std::max(yStartIdx,0);
        xEndIdx = std::min(xEndIdx,(int)distM.size1());
        yEndIdx = std::min(yEndIdx,(int)distM.size2());


        for(int i=xStartIdx; i<xEndIdx; i++){
            for(int j=yStartIdx; j<yEndIdx; j++){
                texelVal += distM(i,j);
            }
        }
        return texelVal;
    }
    using namespace boost;
    double determinePenetration(double c, double initPen, std::vector<TactileArraySensor::DistPoint>& points, double force){
        // this should be full filled
        // force == SUM[ points[i] ]*c

        //std::cout << "init penetration: " << initPen << std::endl;
        double penetration = initPen;
        for(int i=0;i<10;i++){
            double pensum = 0;
            int nrOfPoints = 0;
            BOOST_FOREACH(TactileArraySensor::DistPoint& dp, points){
                if(dp.dist<0)
                    continue;
                pensum += dp.dist;
                nrOfPoints++;
            }
            //std::cout << "NrOfPoints: " << nrOfPoints << "\n";
            //std::cout << "pensum: " << pensum << "\n";

            double nforce = pensum*c;
            double diff = force-nforce;
            double diffStep = Math::clamp(diff/(nrOfPoints*2), -initPen/10, initPen/10);
            penetration += diffStep;
            //std::cout << "DIFF: " << diff << " " << diffStep << std::endl;
            BOOST_FOREACH(TactileArraySensor::DistPoint& dp, points){
                dp.dist = dp.dist + diffStep;
                //std::cout << "Dist: " << dp.dist << std::endl;
            }
        }
        return penetration;
    }

}


void TactileArraySensor::acquire(){

}

ublas::matrix<float> TactileArraySensor::getTexelData()  const {
    return _pressure;
}

void TactileArraySensor::addForceW(const Vector3D<>& point,
                                    const Vector3D<>& force,
                                    const Vector3D<>& snormal,
                                    dynamics::Body *body)
{
    addForce(_fTw*point, _fTw.R()*force, _fTw.R()*snormal, body);
}


void TactileArraySensor::addForce(const Vector3D<>& point,
                                   const Vector3D<>& force,
                                   const Vector3D<>& snormal,
                                   dynamics::Body *body)
{
    //std::cout << "ADDING FORCE.... " << snormal << force << std::endl;
    //std::cout << "ADDING FORCE dot.... " << dot(snormal,force) << std::endl;
    //if( dot(force,snormal)<0 ){
    //    return;
    //}
    //std::cout << "1";
    Contact3D c3d(_wTf*point,_wTf.R()*snormal,_wTf.R()*force);
    _allAccForces.push_back( c3d );

    //std::cout << "3";
    // remember to test if the force direction is in the negative z-direction of each texel
    Vector3D<> f = _hmapTf.R() * force;


    // test if point lie in (+x,+y,+z) relative to the sensor base transform
    // though we give a little slack to allow forces on the boundary in
    Vector3D<> p = _hmapTf * point;
    if( /*p(0)<-_texelSize(0) || p(1)<-_texelSize(1) ||*/ p(2)<-0.002){
        //std::cout << "1pos wrong: " << p << std::endl;
        return;
    }

    //std::cout << "2";
    // and also if it lie inside the width and height of the grid
    //if(p(0)>(_w+1)*_texelSize(0) || p(1)>(_h+1)*_texelSize(1)){
        ////std::cout << "1pos wrong: " << p << std::endl;
    //    return;
    //}



    // oki so we know that the force acts on the grid somewhere
    // now locate all at most 15 texels points within
    int xIdx = (int)( floor(p(0)/_texelSize(0)));
    int yIdx = (int)( floor(p(1)/_texelSize(1)));

    // if the index is boundary then pick the closest texel normal
    int xIdxTmp = Math::clamp(xIdx, 0, _w-1);
    int yIdxTmp = Math::clamp(yIdx, 0, _h-1);

    //std::cout << "4";
    Vector3D<> normal = _normalMatrix[xIdxTmp][yIdxTmp];
    if( dot(f, normal)>=0 ){
        //std::cout << "SAME DIRECTION" << std::endl;
        return;
    }

    //std::cout << "5";
    // so heres the point force...
    //double forceVal = fabs( dot(f,normal) );
    //double scaleSum = 0;


    // we save all forces until the update is called
    //,_wTf.R()*snormal,_wTf.R()*force
    Contact3D con(p, normal, f);
    _forces[body].push_back(con);
}

// contact normal in a's coordinates. describe the contact normal from a to b

std::vector<TactileArraySensor::DistPoint> TactileArraySensor::generateContacts(dynamics::Body *body, const Vector3D<>& cnormal, const State& state){
    Frame *tframe = this->getFrame();
    Frame *bframe = &body->getBodyFrame();
    RW_ASSERT(tframe);
    RW_ASSERT(bframe);

    Transform3D<> wTa = Kinematics::worldTframe(tframe, state);
    Transform3D<> wTb = Kinematics::worldTframe(bframe, state);
    //std::cout << "getting models" << std::endl;
    //std::cout << "Frame name: " << tframe->getName();
    ProximityModelPtr modelA = _narrowStrategy->getModel(tframe);
    //std::cout << "getting models" << std::endl;
    ProximityModelPtr modelB = _narrowStrategy->getModel(bframe);

    //double stepSize = _maxPenetration;
    Vector3D<> step = _maxPenetration * (wTa.R()*cnormal);
    //std::cout << "collides" << std::endl;
    // we first need to make sure that the boddies are not penetrating
    // so we move the bodies in the opposite direction of the contact normal
    // until they are not colliding
    bool colliding = _narrowStrategy->collides(modelA, wTa , modelB, wTb);
    int loopcount =0;
    if( colliding ){
        while(colliding){
            wTb.P() += step;
            colliding = _narrowStrategy->collides(modelA, wTa, modelB, wTb);
            //std::cout << "Step: " << wTa.P() << " " << wTb.P() << std::endl;
            loopcount++;
            if(loopcount>20){
            	RW_WARN("PENETRATING: Body contact normal is incorrect!!!");
            	return std::vector<TactileArraySensor::DistPoint>();
            }
            RW_ASSERT(loopcount<100);
        }
    }
    // if they where not colliding then we make sure that they at least is
    // very close
    else {
        while(!colliding){
            wTb.P() -= step;
            colliding = _narrowStrategy->collides(modelA, wTa, modelB, wTb);
            loopcount++;
            if(loopcount>10){
            	RW_WARN("Body contact normal is incorrect!!!");
            	return std::vector<TactileArraySensor::DistPoint>();
            }
            if(loopcount>90){
            	//std::cout << "step: " << step << std::endl;
                //std::cout << "Step: " << wTa.P() << " " << wTb.P() << std::endl;
            }
            RW_ASSERT(loopcount<100);
        }
        wTb.P() += step;
    }

    // okay, so now we have the bodies that is within one mm of
    // each other. Assuming the colliding body is rigid and that
    // we know the maximum possible penetration (because of elasticity)
    // we find the worst case area that is in contact by looking for contacts
    // within a distance of 2 mm
    //std::cout << "calculating distances" << std::endl;
    MultiDistanceResult result;
    _narrowStrategy->calcDistances(result,
                                              modelA,wTa,
                                              modelB,wTb,
                                              _maxPenetration*2);
    //std::cout << " distance result: " << result.distances.size() << std::endl;
    // the shortest distance between the models
    double sdistance = result.distance;
    //std::cout << "sdistance: " << sdistance << std::endl;
    // now all contacts that is further away than sdistance+0.001 cannot
    // be in collision so we remove these.
    std::vector<DistPoint> validResult;
    for(size_t i=0;i<result.distances.size();i++){
        //std::cout << "d: " << result.distances[i] << std::endl;
        if(result.distances[i] < sdistance+_maxPenetration){
            DistPoint dp;
            dp.p1 = result.p1s[i];
            dp.p2 = result.p2s[i];
            dp.dist = result.distances[i]-sdistance;
            validResult.push_back(dp);
        }
    }
    //std::cout << "ValidResults: " << validResult.size() << std::endl;
    return validResult;
}

namespace {

/*	getContactPoints(){

	}
	*/


}


void TactileArraySensor::update(double dt, rw::kinematics::State& state){
	// make sure not to sample more often than absolutely necessary
	_accTime+=dt;
	if(_accTime<_stime){
	    _wTf = Kinematics::worldTframe( getFrame(), state);
	    _fTw = inverse(_wTf);
	    _forces.clear();
	    _allAccForces.clear();
		return;
	}
	double rdt = _accTime;
	_accTime -= _stime;

	//std::cout << "update!" << std::endl;
    // we have collected all forces that affect the sensor.
    // Now we need to extrapolate the area of the force since the
    // surface is elastic. We do this by checking for collision between
	// the touching object and the normal trimesh

	bool hasCollision = false;
	double totalNormalForce = 0;
    typedef std::map<dynamics::Body*, std::vector<Contact3D> > BodyForceMap;
    BodyForceMap::iterator iter = _forces.begin();
    for(;iter!=_forces.end();++iter){
    	//std::cout << "BODY" << std::endl;
    	Body *body = (*iter).first;


		Frame *tframe = this->getFrame();
		Frame *bframe = &body->getBodyFrame();
		RW_ASSERT(tframe);
		RW_ASSERT(bframe);

		Transform3D<> wTa = Kinematics::worldTframe(tframe, state)*_fThmap;
		Transform3D<> wTb = Kinematics::worldTframe(bframe, state);
		//ProximityModelPtr modelA = _narrowStrategy->getModel(tframe);
		ProximityModelPtr modelB = _narrowStrategy->getModel(bframe);
		CollisionData data;
		bool collides = _narrowStrategy->collides(_nmodel, wTa, modelB, wTb, data); //wTa*_fThmap

		if( !collides )
			continue;
		if( !hasCollision ){
			// initialize variables
			_distMatrix =  ValueMatrix(_distMatrix.size1(),_distMatrix.size2(), 100);
			//std::cout << "DIMENSIONS: " << _distMatrix.size1() << " " << _distMatrix.size2()<< std::endl;
		}
		hasCollision = true;
		//std::cout << "Yes it really collides!" << bframe->getName() << std::endl;

		if(_frameToGeoms.find(bframe)==_frameToGeoms.end())
			_frameToGeoms[bframe] = Proximity::getGeometry(bframe);

		std::vector<GeometryPtr> &geoms = _frameToGeoms[bframe];
		// now we try to get the contact information
		if(data._collidePairs.size()>0){
			int bodyGeomId = data._collidePairs[0].geoIdxB;
			//std::cout << "BodyGeomId: " << std::endl;
			TriMesh *mesh = dynamic_cast<TriMesh*> (geoms[bodyGeomId]->getGeometryData().get());
			if(mesh){

				for(size_t i = 0; i<data._collidePairs[0]._geomPrimIds.size();i++){
					std::pair<int,int> &pids = data._collidePairs[0]._geomPrimIds[i];
					//std::cout << "Colliding pairs: " << pids.first << " <---> " << pids.second << std::endl;
					RW_ASSERT(0<=pids.first);
					RW_ASSERT(pids.first<_ntrimesh->getSize());

					// for each colliding pair we find the closest intersection
					// get the triangle
					Triangle<> tri = mesh->getTriangle(pids.second);
					Triangle<> triA = _ntrimesh->getTriangle(pids.first);

					Vector3D<> point;
					if( !IntersectUtil::intersetPtRayPlane(triA[0], triA[2], data._aTb*tri[0], data._aTb*tri[1], data._aTb*tri[2], point) )
						continue;

					// now we have the point of intersection, now save it in the contact array
					// if its closer than the existing point

					double ndist = MetricUtil::dist2( triA[0], point);
					//std::cout << "Intersect: " << point << std::endl;
					//std::cout << "NDist: " << ndist << std::endl;
					if( ndist > (&_distMatrix(0,0))[pids.first] )
						continue;
					(&_distMatrix(0,0))[pids.first] = ndist;
					//(&_contactMatrix[0][0])[pids.first] = point;
				}
			}
	    	std::vector<rw::sensor::Contact3D> &bforce = (*iter).second;
	    	// add to total force
	    	BOOST_FOREACH(rw::sensor::Contact3D& c, bforce){
	    		//std::cout << "TotalNormalForce += " << (-c.normalForce) << std::endl;
	    		totalNormalForce += -c.normalForce;
	    	}

		}
    }
    _forces.clear();

    double closest = 100;
    if( hasCollision ){
		for(size_t y=0; y<_distMatrix.size2(); y++){
			for(size_t x=0; x<_distMatrix.size1(); x++){
				_distMatrix(x,y) = _distMatrix(x,y) - _distDefMatrix(x,y);
				if(_distMatrix(x,y)<closest)
					closest = _distMatrix(x,y);
			}
		}
		//std::cout << "\n\n" << _distMatrix << "\n\n"<< std::endl;
    }
    // now we need to convert the _distMatrix to pressure values
    ValueMatrix pressure =  boost::numeric::ublas::zero_matrix<float>(_pressure.size1(),_pressure.size2());
    //
    // we only have the total normal force, we expect the position between finger and
    // object to be error prone so we iteratively determine the area of contact such
    // that area*Ftotal = deformed_volume * defToStress
    //
    if( hasCollision ){
		double offset = 0;
		double bestOffset = 0;
		double bestScore = 100000;
		for(int i=0;i<20;i++){
			//std::cout << i << " offset: " << offset << std::endl;
			// 1. we aallready have total force
			// 2. calculate area and "volume"
			double area = 0;
			double volume = 0;
			double totalVolume = 0;
			for(size_t y=0; y<_distMatrix.size2(); y++){
				for(size_t x=0; x<_distMatrix.size1(); x++){
					//std::cout << _distMatrix(x,y) << "\n";
					double val = _distMatrix(x,y)-closest+offset;
					if( val<0 ){
						area += _texelArea;
						volume += -val*_texelArea;
						totalVolume += 0.002*_texelArea;
					}
				}
			}
			//std::cout << "area: " << area << std::endl;
			if(area>0){
				// (area*Ftotal) < (deformed_volume * defToStress)
				// increase offset
				// and if
				// (area*Ftotal) < (deformed_volume * defToStress)
				//   <   GPa * unitless = stress

				double score = totalNormalForce/(area*1000) - (volume * _elasticity)/totalVolume;

				//std::cout << "Score: " << score << std::endl;
				if(fabs(score)>bestScore)
					continue;
				bestScore = fabs(score);
				bestOffset = offset;

				//std::cout << offset << ":" << score << " = " << totalNormalForce/(area*1000)
				//			<< "kPa -" << ((volume*_elasticity)/totalVolume) << "kPa" << std::endl;
				//std::cout << offset << ":" << score << " = " << totalNormalForce<< "/" << (area*1000)
				//		<< "-" << "(" << volume << "*" <<_elasticity << ")"
				//		<< "/" << totalVolume << std::endl;

			}
			offset -= 0.001/20;
		}
		double totalDist = 0;
		for(size_t y=0; y<_distMatrix.size2(); y++){
			for(size_t x=0; x<_distMatrix.size1(); x++){
				double dist = _distMatrix(x,y)-closest+bestOffset;
				if( dist>0 )
					continue;
				totalDist +=  dist;
			}
		}
		//std::cout << "TOTAL DIST: " << totalDist << std::endl;
		double distToForce = totalNormalForce/totalDist;
		if(distToForce>10000){
			std::cout << "Dist force: " << distToForce << std::endl;
			std::cout << "totalNormalForce: " << totalNormalForce << std::endl;
			std::cout << "totalDist: " << totalDist << std::endl;

		} else {
			//std::cout << "BestScore: " << bestScore << std::endl;
			// now we now the actual penetration/position we can apply point force function
			// to calculate the pressure on the sensor surface
			for(size_t y=0; y<_distMatrix.size2(); y++){
				for(size_t x=0; x<_distMatrix.size1(); x++){

					//std::cout << _distMatrix(x,y) << "\n";
					double dist = _distMatrix(x,y)-closest+bestOffset;
					if( dist>0 )
						continue;

					double force = dist*distToForce;

					//Vector3D<> p = _hmapTf * dp.p1;
					Vector3D<> p = _centerMatrix[x][y];//_contactMatrix[x][y];


					//int xIdx = (int)( floor(p(0)/_texelSize(0)));
					//int yIdx = (int)( floor(p(1)/_texelSize(1)));
					int xIdx = x;
					int yIdx = y;
					for(int xi=std::max(0,xIdx-1); xi<std::min(xIdx+2,_w); xi++){
						for(int yi=std::max(0,yIdx-1); yi<std::min(yIdx+2,_h); yi++){
							double texelScale = getValueOfTexel(xi*_texelSize(0),yi*_texelSize(1),
																_texelSize(0),_texelSize(1),
																p(0),p(1),
																_dmask, _maskWidth, _maskHeight);
							RW_ASSERT(texelScale<100000);
							_accForces(xi,yi) += force*texelScale;
							//std::cout << "accForce +="<< force << "*" << texelScale << std::endl;
						}
					}
				}
			}
		}

	    // copy and convert accumulated forces into pressure values
	    //double texelArea = _texelSize(0)*_texelSize(1);
		double totalPressure = 0, totalArea = 0;
	    for(size_t x=0; x<_accForces.size1(); x++){
	        for(size_t y=0; y<_accForces.size2(); y++){
	            // clamp to max pressure
	            //if( _accForces(x,y)/texelArea>1.0 )
	            //    std::cout << "Pressure: ("<<x<<","<<y<<") " << _accForces(x,y)/texelArea << " N/m^2 " << std::endl;
	        	if(_accForces(x,y)<=0){
	        		continue;
	        	}

	            pressure(x,y) = std::min( _accForces(x,y)/(_texelArea*1000), _maxPressure );
	            totalArea += _texelArea;
	            totalPressure += pressure(x,y);
	        }
	    }
	    //std::cout << "totalPressure: "
	    //		<< totalPressure/(totalNormalForce/(totalArea*1000)) << " --> "
	    //		<< totalPressure << "kPa == "
	    //		<< (totalNormalForce/(totalArea*1000))
	    //		<< "kPa == "<< totalNormalForce << "/" << totalArea << std::endl;

	    // the actual pressure should not be more than totForce/(totalArea*1000)
	    // so we scale it down to fit-....
	    double presScale = (totalNormalForce/(totalArea*1000))/totalPressure;
	    double ntotpressure = 0;
	    for(size_t x=0; x<pressure.size1(); x++){
	        for(size_t y=0; y<pressure.size2(); y++){
	        	if(pressure(x,y)>0){
	        		// and convert it to pascal
	        		pressure(x,y) *= presScale*1000;
	        		ntotpressure += pressure(x,y)/1000;
	        	}
	        }
	    }

	    //std::cout << "New total pressure: " << ntotpressure << std::endl;

	    _accForces = ublas::zero_matrix<double>(_accForces.size1(),_accForces.size2());
	    _allForces = _allAccForces;
	    _allAccForces.clear();
    }


    // Low pass filtering is done in the end
    // for i from 1 to n
    //    y[i] := y[i-1] + alpha * (x[i] - y[i-1])
    ValueMatrix &ym = _pressure;
    ValueMatrix &xm = pressure;
    const double alpha = rdt/(_tau+rdt);
    for(size_t y=0; y<ym.size2(); y++){
    	for(size_t x=0; x<ym.size1(); x++){
        	ym(x,y) += alpha * (xm(x,y)-ym(x,y));
        }
    }

    _pressure = pressure;
    //std::cout << _pressure << std::endl;

    // update aux variables
    _wTf = Kinematics::worldTframe( getFrame(), state);
    _fTw = inverse(_wTf);
}

#ifdef OLD_STUFF
void TactileArraySensor::update(double dt, rw::kinematics::State& state){
    // we have collected all forces that affect the sensor.
    // Now we need to extrapolate the area of the force since the
    // surface is elastic. We do this by finding all point to point distances
    // between the two bodies.
    //std::cout << "Update tactile sensor array" << std::endl;
	ValueMatrix lastpres = _pressure;
    for(size_t x=0; x<_pressure.size1(); x++){
        for(size_t y=0; y<_pressure.size2(); y++){
        	_pressure(x,y) = lastpres(x,y)*0.8;
        }
    }

    // We first calulate the common contact normal
    Vector3D<> cnormal(0,0,0);
    double totalForce = 0;
    dynamics::Body* body = NULL;
    typedef std::map<dynamics::Body*, std::vector<Contact3D> > BodyForceMap;
    BodyForceMap::iterator iter = _forces.begin();
    for(;iter!=_forces.end();++iter){
        //std::cout << "ITER" << std::endl;
        BOOST_FOREACH(const Contact3D& c, (*iter).second){
            //std::cout << "N: " << c.n << "\n";
            //std::cout << "F: " << c.f << "\n";
            if(MetricUtil::norm2(c.f)<0.0000001)
            	continue;
            cnormal += normalize(c.n);
            //cnormal += normalize(c.f);
            totalForce += MetricUtil::norm2(c.f);
        }
        body = (*iter).first;
    }
    //std::cout << "TOTAL FORCE: " << totalForce << "N" << std::endl;

    //std::cout << "Clear forces" << std::endl;
    _forces.clear();
    if(body!=NULL && totalForce>0.0000001 ){
        cnormal = normalize(cnormal);
        //std::cout << "Generate contacts: " << cnormal << std::endl;
        RW_ASSERT(MetricUtil::norm2(cnormal)>0.1);
        std::vector<DistPoint> res = generateContacts(body, -cnormal, state);

        boost::numeric::ublas::matrix<DistPoint> contacts(_accForces.size1(),_accForces.size2());
        double weightedArea = 0;
        // now filter away the distances that are double in cells
        BOOST_FOREACH(DistPoint& dp, res){
            Vector3D<> p = _hmapTf * dp.p1;

            int xIdx = (int)( floor(p(0)/_texelSize(0)));
            int yIdx = (int)( floor(p(1)/_texelSize(1)));

            if( xIdx<0 || yIdx<0 ){
                //std::cout << xIdx << "<" << 0 << " && " << yIdx << "<" << 0<< std::endl;
            }
            if( !(xIdx<contacts.size1() && yIdx<contacts.size2()) ){
                //std::cout << xIdx << "<" << contacts.size1() << " && " << yIdx << "<" << contacts.size2()<< std::endl;
            }

            if( xIdx<0 || yIdx<0 )
                continue;
            if( contacts.size1()<=xIdx || contacts.size2()<=yIdx )
                continue;
            RW_ASSERT(0<=xIdx && 0<=yIdx);
            RW_ASSERT(xIdx<contacts.size1() && yIdx<contacts.size2());
        	if( contacts(xIdx,yIdx).dist > dp.dist ){
        	    //std::cout << "ADD DIST!" << dp.dist << std::endl;
        		contacts(xIdx,yIdx) = dp;
        	}
        }
        double totalDist = 0;
        int nrOfActiveTexels = 0;
        res.clear();
        for(int x=0;x<contacts.size1();x++){
            for(int y=0;y<contacts.size2();y++){
            	if(contacts(x,y).dist>1){
            	    // check if any of the neighbors are high
            	    if( 0<x && x<contacts.size1()-1 && contacts(x-1,y).dist<1 && contacts(x+1,y).dist<1){
            	        contacts(x,y).p1 = (contacts(x+1,y).p1-contacts(x-1,y).p1)/2+contacts(x-1,y).p1;
                        contacts(x,y).dist = (contacts(x+1,y).dist-contacts(x-1,y).dist)/2+contacts(x-1,y).dist;
            	    } else if(0<y && y<contacts.size2()-1 && contacts(x,y-1).dist<1 && contacts(x,y+1).dist<1){
            	        contacts(x,y).p1 = (contacts(x,y+1).p1-contacts(x,y-1).p1)/2+contacts(x,y-1).p1;
                        contacts(x,y).dist = (contacts(x,y+1).dist-contacts(x,y-1).dist)/2+contacts(x,y-1).dist;
                    } else if(0<x && x<contacts.size1()-1 && 0<y && y<contacts.size2()-1){
                        if( contacts(x-1,y-1).dist<1 &&  contacts(x+1,y+1).dist<1 ){
                            contacts(x,y).p1 = (contacts(x-1,y-1).p1-contacts(x+1,y+1).p1)/2+contacts(x+1,y+1).p1;
                            contacts(x,y).dist = (contacts(x-1,y-1).dist-contacts(x+1,y+1).dist)/2+contacts(x+1,y+1).dist;
                        } else if (contacts(x-1,y+1).dist<1 &&  contacts(x+1,y-1).dist<1){
                            contacts(x,y).p1 = (contacts(x-1,y+1).p1-contacts(x+1,y-1).p1)/2+contacts(x+1,y-1).p1;
                            contacts(x,y).dist = (contacts(x-1,y+1).dist-contacts(x+1,y-1).dist)/2+contacts(x+1,y-1).dist;
                        } else {
                            continue;
                        }
                    } else {
            	        continue;
            	    }

            	}

                /*if(contacts(x,y).dist>1){
                    continue;
                }*/
                //std::cout << "ADD CONTACT: " <<  x << " " << y << " " << contacts(x,y).dist << std::endl;

                contacts(x,y).dist = _maxPenetration-contacts(x,y).dist;
                res.push_back(contacts(x,y));
            	totalDist += contacts(x,y).dist;
            	nrOfActiveTexels++;
            	weightedArea += (contacts(x,y).dist*1.0/_maxPenetration);
            }
        }
/*
        BOOST_FOREACH(DistPoint& dp, res){
            totalDist += 0.001-dp.dist;
        }*/
        //std::cout << "TOTAL DIST: " << totalDist << std::endl;
        //RW_ASSERT(totalDist==0);
        double maxArea = nrOfActiveTexels*_texelSize(0)*_texelSize(1);
        //std::cout << "Worst case area: " << maxArea << std::endl;
        //std::cout << "Estimated area : " << (weightedArea*_texelSize(0)*_texelSize(1)) << std::endl;

        // area should be used to determine the actual penetration depth
        // area*force == pressure | pressure*k = penetration

        // realPenetration = (realarea*force)/k

        // realArea = perhaps using parabol fitted to points.

        double penetration = determinePenetration(_elasticity, _maxPenetration, res, totalForce);
        totalDist = 0;
        BOOST_FOREACH(DistPoint& dp, res){
            double dist = dp.dist;
            if(dist<=0)
                continue;
            totalDist += dist;
        }
        //double penetration = _maxPenetration*weightedArea/maxArea;

        if( totalDist>0 ){
            double scale = totalForce/totalDist;
            //std::cout << "scale: " << scale << std::endl;
            BOOST_FOREACH(DistPoint& dp, res){
                double dist = dp.dist;
                if(dist<=0)
                    continue;
                //if(dist>_maxPenetration)
                //    dist = _maxPenetration;

                double force = dist*scale;

                Vector3D<> p = _hmapTf * dp.p1;

                int xIdx = (int)( floor(p(0)/_texelSize(0)));
                int yIdx = (int)( floor(p(1)/_texelSize(1)));

                //std::cout << xIdx << " " << yIdx << " " << force <<  std::endl;

                for(int x=std::max(0,xIdx-1); x<std::min(xIdx+2,_w); x++){
                    for(int y=std::max(0,yIdx-1); y<std::min(yIdx+2,_h); y++){
                        double texelScale = getValueOfTexel(x*_texelSize(0),y*_texelSize(1),
                                                            _texelSize(0),_texelSize(1),
                                                            p(0),p(1),
                                                            _dmask, _maskWidth, _maskHeight);
                        _accForces(x,y) += force*texelScale;
                        //std::cout << force << "*" << texelScale << std::endl;
                    }
                }
            }


            // copy and convert accumulated forces into pressure values
            double texelArea = _texelSize(0)*_texelSize(1);
            for(size_t x=0; x<_accForces.size1(); x++){
                for(size_t y=0; y<_accForces.size2(); y++){
                    // clamp to max pressure
                    //if( _accForces(x,y)/texelArea>1.0 )
                    //    std::cout << "Pressure: ("<<x<<","<<y<<") " << _accForces(x,y)/texelArea << " N/m^2 " << std::endl;
                    _pressure(x,y) = std::min( _accForces(x,y)/texelArea, _maxPressure );
                }
            }
            //std::cout << "_pressure" << std::endl;
            //std::cout << _pressure << std::endl;

            _accForces = ublas::zero_matrix<double>(_accForces.size1(),_accForces.size2());
            _allForces = _allAccForces;
            _allAccForces.clear();
        }
    }

    // update aux variables
    _wTf = Kinematics::worldTframe( getFrame(), state);
    _fTw = inverse(_wTf);
}
#endif

void TactileArraySensor::setTexelData(const boost::numeric::ublas::matrix<float>& data){
	if(data.size1()!=_pressure.size1()){
		RW_WARN("dimension mismatch: " << data.size1() <<"!=" <<_pressure.size1());
		return;
	}
	if(data.size2()!=_pressure.size2()){
		RW_WARN("dimension mismatch: " << data.size2() <<"!=" <<_pressure.size2());
		return;
	}

	_pressure = data;
};



#ifdef OLD_ADDFORCE

void TactileArraySensor::addForce(const Vector3D<>& point,
                                   const Vector3D<>& force,
                                   const Vector3D<>& snormal,
                                   dynamics::Body *body)
{
    if( dot(force,snormal)<0 ){
        return;
    }

    Contact3D c3d(_wTf*point,_wTf.R()*force,_wTf.R()*snormal);
    _allAccForces.push_back( c3d );


    // test if point lie in (+x,+y,+z) relative to the sensor base transform
    // though we give a little slack to allow forces on the boundary in
    Vector3D<> p = _hmapTf * point;
    if(p(0)<-_texelSize(0) || p(1)<-_texelSize(1) || p(2)<-0.002){
        //std::cout << "1pos wrong: " << p << std::endl;
        return;
    }

    // and also if it lie inside the width and height of the grid
    if(p(0)>(_w+1)*_texelSize(0) || p(1)>(_h+1)*_texelSize(1)){
        //std::cout << "1pos wrong: " << p << std::endl;
        return;
    }

    // remember to test if the force direction is in the negative z-direction of each texel
    Vector3D<> f = _hmapTf.R() * force;

    // oki so we know that the force acts on the grid somewhere
    // now locate all at most 15 texels points within
    int xIdx = (int)( floor(p(0)/_texelSize(0)));
    int yIdx = (int)( floor(p(1)/_texelSize(1)));

    // if the index is boundary then pick the closest texel normal
    int xIdxTmp = std::min( std::max(xIdx,0), _w-1);
    int yIdxTmp = std::min( std::max(yIdx,0), _h-1);

    Vector3D<> normal = _normalMatrix[xIdxTmp][yIdxTmp];
    if( dot(f, normal)>0 ){
        //std::cout << "SAME DIRECTION" << std::endl;
        return;
    }

    // so heres the point force...
    double forceVal = fabs( dot(f,normal) );
    double scaleSum = 0;
    //std::cout << "ADDING FORCE to: (" << p(0) << "," << p(1)<< ")" << std::endl;
    // and now to distribute it on the different texels
    //int x=std::min(std::max(0,xIdx),_w-1);
    //int y=std::min(std::max(0,yIdx),_h-1);
    //_accForces(x,y) = 10000;//forceVal*texelScale;

    for(int x=std::max(0,xIdx-1); x<std::min(xIdx+2,_w); x++){
        for(int y=std::max(0,yIdx-1); y<std::min(yIdx+2,_h); y++){
            double texelScale = getValueOfTexel(x*_texelSize(0),y*_texelSize(1),
                                                _texelSize(0),_texelSize(1),
                                                p(0),p(1),
                                                _dmask, _maskWidth, _maskHeight);
            _accForces(x,y) += forceVal*texelScale;
            scaleSum += texelScale;

        }
    }

    if( scaleSum>1.01 )
        std::cout << "SCALE SUM: " << scaleSum << std::endl;

}

void TactileArraySensor::update(double dt, rw::kinematics::State& state){
    // all forces accumulated

    // copy and convert accumulated forces into pressure values
    double texelArea = _texelSize(0)*_texelSize(1);
    for(size_t x=0; x<_accForces.size1(); x++){
        for(size_t y=0; y<_accForces.size2(); y++){
            // clamp to max pressure
            //if( _accForces(x,y)/texelArea>1.0 )
            //    std::cout << "Pressure: ("<<x<<","<<y<<") " << _accForces(x,y)/texelArea << " N/m^2 " << std::endl;
            _pressure(x,y) = std::min( _accForces(x,y)/texelArea, _maxPressure );
        }
    }
    //std::cout << _pressure << std::endl;
    _accForces = ublas::zero_matrix<double>(_accForces.size1(),_accForces.size2());

    _allForces = _allAccForces;
    _allAccForces.clear();

    // update aux variables
    _wTf = Kinematics::worldTframe( getFrame(), state);
    _fTw = inverse(_wTf);

    //std::cout << _pressure << std::endl;
}
#endif
