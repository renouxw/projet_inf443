#include "interpolation.hpp"

using namespace vcl;

/** Compute the linear interpolation p(t) between p1 at time t1 and p2 at time t2*/
vcl::vec3 linear_interpolation(float t, float t1, float t2, const vcl::vec3& p1, const vcl::vec3& p2);

/** Compute the cardinal spline interpolation p(t) with the polygon [p0,p1,p2,p3] at time [t0,t1,t2,t3]
*  - Assume t \in [t1,t2] */
vcl::vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, vcl::vec3 p0, vcl::vec3  p1, vcl::vec3 p2, vcl::vec3 p3, float K);


/** Find the index k such that intervals[k] < t < intervals[k+1] 
* - Assume intervals is a sorted array of N time values
* - Assume t \in [ intervals[0], intervals[N-1] [       */
size_t find_index_of_interval(float t, vcl::buffer<float> const& intervals);


vec3 const interpolation(float t, buffer<vec3> key_positions, buffer<float> key_times)
{
    float const K = 0.5;
    // Find idx such that key_times[idx] < t < key_times[idx+1]
    //if ( t < key_times[1] || t > key_times[3]){
        //return t;
    //}

    if ( t < key_times[1]){
        vec3 const p = cardinal_spline_interpolation(t, key_times[0], key_times[1], key_times[2], key_times[3], 
        key_positions[0], key_positions[1], key_positions[2], key_positions[3], K);
        return p;
    }

    if ( t > key_times[1]){

        const float x = rand_interval(0,0.2);
        const float y = rand_interval(0,0.2);
        vec3 const v = {x,y,0};
        key_positions[0] = key_positions[1];
        key_positions[1] = key_positions[2];
        key_positions[2] = key_positions[3];
        key_positions[3] = 2 * key_positions[3]-key_positions[1]+v;
        
        vec3 const p = cardinal_spline_interpolation(t, key_times[0], key_times[1], key_times[2], key_times[3], key_positions[0], key_positions[1], key_positions[2], key_positions[3], K);
        return p;
    }
}



vec3 linear_interpolation(float t, float t1, float t2, const vec3& p1, const vec3& p2)
{
    float const alpha = (t-t1)/(t2-t1);
    vec3 const p = (1-alpha)*p1 + alpha*p2;

    return p;
}

vec3 cardinal_spline_interpolation(float t, float t0, float t1, float t2, float t3, vec3 p0, vec3 p1, vec3 p2, vec3 p3, float K)
{

    float s = (t-t1)/(t2-t1);

    vec3 const d0 = 2*K*(p2-p0)/(t2-t0);
    vec3 const d1 = 2*K*(p3-p1)/(t3-t1);

    float s3 = s*s*s;
    float s2 = s*s;

    return (2*s3 - 3*s2 + 1)*p1 + (s3 - 2*s2 + s)*d0 + (3*s2 - 2*s3)*p2 + (s3-s2)*d1;
}

size_t find_index_of_interval(float t, buffer<float> const& intervals)
{
    size_t const N = intervals.size();
    bool error = false;
    if (intervals.size() < 2) {
        std::cout<<"Error: Intervals should have at least two values; current size="<<intervals.size()<<std::endl;
        error = true;
    }
    if (N>0 && t < intervals[0]) {
        std::cout<<"Error: current time t is smaller than the first time of the interval"<<std::endl;
        error = true;
    }
    if(N>0 && t > intervals[N-1]) {
        std::cout<<"Error: current time t is greater than the last time of the interval"<<std::endl;
        error = true;
    }
    if (error == true) {
        std::string const error_str = "Error trying to find interval for t="+str(t)+" within values: ["+str(intervals)+"]";
        error_vcl( error_str );
    }


    size_t k=0;
    while( intervals[k+1]<t )
        ++k;
    return k;
}
