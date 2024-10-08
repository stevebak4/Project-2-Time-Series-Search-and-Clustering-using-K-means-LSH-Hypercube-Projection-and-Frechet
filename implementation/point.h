#ifndef POINT_H_H
#define POINT_H_H


#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <sstream>
#include <type_traits>


#include "./types.h"
#include "./interval.h"


class Point : public Coordinates {
public:    
    inline Point(const dimensions_t d) : Coordinates(d) {}
    
    inline dimensions_t dimensions() const {
        return size();
    }
    
    inline coordinate_t get(const dimensions_t i) const { 
        return Coordinates::operator[](i); 
    }
    
    inline void set(const dimensions_t i, coordinate_t val) {
        Coordinates::operator[](i) = val;
    }
    
    #pragma omp declare simd
    inline const coordinate_t& operator[](const dimensions_t i) const { 
        return Coordinates::operator[](i); 
    }
    
    #pragma omp declare simd
    inline coordinate_t& operator[](const dimensions_t i) { 
        return Coordinates::operator[](i); 
    }
    
    inline Point& operator+=(const Point &point) {
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            operator[](i) += point[i];
        }
        return *this;
    }
    
    inline Point& operator-=(const Point &point) {
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            operator[](i) -= point[i];
        }
        return *this;
    }
    
    inline Point& operator/=(const distance_t distance) {
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            operator[](i) /= distance;
        }
        return *this;
    }
    
    inline Point operator+(const Point &point) const {
        Point result = *this;
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            result[i] += point[i];
        }
        return result;
    }
    
    inline Point operator-(const Point &point) const {
        Point result = *this;
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            result[i] -= point[i];
        }
        return result;
    }
    
    template<typename T>
    inline Point operator*(const T mult) const {
        Point result = *this;
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            result[i] *= mult;
        }
        return result;
    }
    
    inline distance_t operator*(const Point &p) const {
        distance_t result = 0;
        #pragma omp simd reduction(+: result)
        for (dimensions_t i = 0; i < dimensions(); ++i) {
            result += operator[](i) * p[i];
        }
        return result;
    }
    
    inline Point operator/(const distance_t dist) const {
        Point result = *this;
        #pragma omp simd
        for (dimensions_t i = 0; i < dimensions(); ++i){
            result[i] /= dist;
        }
        return result;
    }
    
    inline distance_t dist_sqr(const Point &point) const {
        distance_t result = 0, temp;
        #pragma omp simd private(temp) reduction(+: result)
        for (dimensions_t i = 0; i < dimensions(); ++i){
            temp = operator[](i) - point[i];
            result += temp * temp;
        }
        return result;
    }
    
    inline distance_t dist(const Point &point) const {
        return std::sqrt(dist_sqr(point));
    }
    
    inline distance_t length_sqr() const {
        distance_t result = 0;
        #pragma omp simd reduction(+: result)
        for (dimensions_t i = 0; i < dimensions(); ++i){
            result += operator[](i) * operator[](i);
        }
        return result;
    }
    
    inline distance_t length() const {
        return std::sqrt(length_sqr());
    }
    
    inline distance_t line_segment_dist_sqr(const Point &p1, const Point &p2) const {
        const Vector u = p2 - p1;
        parameter_t projection_param = (*this - p1) * u / (u * u);
        if (projection_param < parameter_t(0)) projection_param = parameter_t(0);
        else if (projection_param > parameter_t(1)) projection_param = parameter_t(1);
        const Point projection = p1 + u * projection_param;
        return projection.dist_sqr(*this);
    }
    
    inline distance_t line_segment_dist(const Point &p1, const Point &p2) const {
        return std::sqrt(line_segment_dist(p1, p2));
    }
    
    inline Interval ball_intersection_interval(const distance_t distance_sqr, const Point &line_start, const Point &line_end) const {
        const Vector u = line_end-line_start, v = *this - line_start;
        const parameter_t ulen_sqr = u.length_sqr(), vlen_sqr = v.length_sqr();
        
        if (near_eq(ulen_sqr, parameter_t(0))) {
            if (vlen_sqr <= distance_sqr) return Interval(parameter_t(0), parameter_t(1));
            else return Interval();
        }
                
        const parameter_t p =  -2. * ((u * v) / ulen_sqr), q = vlen_sqr / ulen_sqr - distance_sqr / ulen_sqr;
        
        const parameter_t phalf_sqr = p * p / 4., discriminant = phalf_sqr - q;
        
        if (discriminant < 0) return Interval();
        
        const parameter_t discriminant_sqrt = std::sqrt(discriminant);
        
        const parameter_t minus_p_h = - p / 2., r1 = minus_p_h + discriminant_sqrt, r2 = minus_p_h - discriminant_sqrt;
        const parameter_t lambda1 = std::min(r1, r2), lambda2 = std::max(r1, r2);
                
        return Interval(std::max(parameter_t(0), lambda1), std::min(parameter_t(1), lambda2));
    }
    
    std::string str() const;
    
    std::string repr() const;
};

class Points : public std::vector<Point> {
    dimensions_t dim;
    
public:
    inline Points(const dimensions_t dim) : dim{dim} {}
    inline Points(const curve_size_t m, const dimensions_t dim) : std::vector<Point>(m, Point(dim)), dim{dim} {}
    inline Points(const curve_size_t m, const Point& p) : std::vector<Point>(m, p), dim{p.dimensions()} {}
    
    inline Point centroid() const {
        if (empty()) return Point(0);
        
        Point mean = operator[](0);
        for (curve_size_t i = 1; i < size(); ++i) {
            mean += operator[](i);
        }
        mean /= size();
        return mean;
    }
    
    inline void add(Point &point) {
        if (point.dimensions() != dim) {
            std::cerr << "Wrong number of dimensions; expected " << dim << " dimensions and got " << point.dimensions() << " dimensions." << std::endl;
            return;
        }
        push_back(point);
    }
    
    inline Point& get(const curve_size_t i) { 
        return Points::operator[](i); 
    }
    
    inline curve_size_t number() const {
        return size();
    }
    
    inline dimensions_t dimensions() const {
        return dim;
    }
    
    std::string str() const;
    
    std::string repr() const;
};

std::ostream& operator<<(std::ostream&, const Point&);
std::ostream& operator<<(std::ostream&, const Points&);




#endif