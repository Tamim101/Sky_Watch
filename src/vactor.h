#pragma once
#include <Arduino.h>
class vactor : public Printable {
    public:
         float x,y,z ;
         vactor():x(0),y(0),z(0){

         };
         vactor(float x, float y, float z): x(x),y(y),z(z){};
         bool zero() const{
            return x == 0 && y == 0 && z == 0;
         }
         bool finite() const{
            return isfinite(x) && isfinite(y) && isfinite(z);
         }
         bool valid() const{
            return finite;
         }
         bool invalid() const{
            return !valid();
         }
         void invalidate(){
            x = NAN;
            y = NAN;
            z = NAN;
         }
         float norm() const{
            return sqrt(x* x + y * y + z * z);
         }
         void normalize(){
            float n = norm();
            x /= n;
            y /= n;
            z /= n;
         }
         vactor operator + (const float b) const{
            return vactor(x + b, y + b, z + b);
         }
         vactor operator * (const float b) const{
            return vactor(x * b, y * b, z * b);
         }
         vactor operator / (const float b) const{
            return vactor(x / b, y / b, z / b);
         }
         vactor operator + (const vactor& b) const{
            return vactor (x + b.x, y + b.y, z + b.z);
         }
         vactor operator - (const vactor& b) const{
            return vactor (x - b.x, y - b.y, z -b.z);
         }
         vactor operator += (const vactor& b){
            return *this = *this + b;
         }
         vactor operator -= (const vactor& b){
            return *this = *this - b;
         }
         vactor operator * (const vactor& b ) const{
            return vactor(x * b.x, y * b.y, z * b.z);   // element wise multiplication
         }
         vactor operator / (const vactor & b) const{
            return vactor(x / b.x , y / b.y, z / b.z);  // element wise division
         }
         bool operator == (const vactor& b) const{
            return x == b.x && y == b.y && z == b.z;
         }
         bool operator != (const vactor& b) const {
            return !(*this == b);
         }
         static float dot(const vactor& a , const vactor& b){
            return a.x * b.x + a.y * b.y + a.z * b.z; 
         }
         static vactor cross(const vactor& a, const vactor& b){
            return vactor(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,a.x * b.y - a.y * b.x);
         }
         static float angleBetween(const vactor a, const vactor& b){
            return acos(constrain(dot(a,b)/ (a.norm() * b.norm()),-1,1));
         }
         static vactor rotationvectorBetween(const vactor& a , const vactor& b){
            vactor direction = cross(a,b);
            if (direction.zero()){
                return cross(a,vactor(1,0,0));
            }
            direction.normalize();
            float angle = angleBetween(a,b);
            return direction * angle;

         }
         size_t printTo(Print& p) const{
            return 
                 p.print(x,15) + p.print(" ") + 
                 p.print(y,15) + p.print (" ")+
                 p.print(z,15);
         }

};
vactor operator * (const float a, const vactor& b ){
    return b * a;
}
vactor operator + (const float a, const vactor& b){
    return b + a ;
}