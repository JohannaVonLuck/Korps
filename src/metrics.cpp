/*******************************************************************************
                           Metrics - Implementation
*******************************************************************************/
#include "main.h"
#include "metrics.h"

/*******************************************************************************
    function    :   kVector::kVector()
    arguments   :   <none>
    purpose     :   Constructor (default)
    notes       :   <none>
*******************************************************************************/
kVector::kVector()
{
    dim[0] = 0.0; dim[1] = 0.0; dim[2] = 0.0;
}

/*******************************************************************************
    function    :   kVector::kVector
    arguments   :   ndim[3] - value to use for each dimensional axis.
    purpose     :   Constructor (alternate 1)
    notes       :   <none>
*******************************************************************************/
kVector::kVector(float ndim[3])
{
    dim[0] = ndim[0]; dim[1] = ndim[1]; dim[2] = ndim[2];
}

/*******************************************************************************
    function    :   kVector::kVector
    arguments   :   d1, d2, d3 - values to use for each dimensional axis.
    purpose     :   Constructor (alternate 2)
    notes       :   <none>
*******************************************************************************/
kVector::kVector(float d1, float d2, float d3)
{
    dim[0] = d1; dim[1] = d2; dim[2] = d3;
}

/*******************************************************************************
    function    :   kVector::~kVector()
    arguments   :   <none>
    purpose     :   Deconstructor
    notes       :   <none>
*******************************************************************************/
kVector::~kVector()
{
    // Do nothing for now...
    return;
}

/*******************************************************************************
    function    :   float* kVector::operator()
    arguments   :   <none>
    purpose     :   () operator overload. Returns ptr to float array data.
    notes       :   <none>
*******************************************************************************/
float* kVector::operator()()
{
    return dim;
}

/*******************************************************************************
    function    :   float& kVector::operator[]
    arguments   :   element - array element
    purpose     :   [] operator overload. Returns a passed-by-reference float
                    from the array. Passing by reference assures that the float
                    values can be set using a convention array-like syntax.
    notes       :   <none>
*******************************************************************************/
float& kVector::operator[](unsigned int element)
{
    return dim[element];
}

/*******************************************************************************
    function    :   kVector kVector::operator=
    arguments   :   v - vector to assign data from
    purpose     :   = operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator=(kVector v)
{
    return kVector(dim[0] = v.dim[0],
                   dim[1] = v.dim[1],
                   dim[2] = v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator=
    arguments   :   v - floater array to assign data from
    purpose     :   = operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator=(float v[3])
{
    return kVector(dim[0] = v[0],
                   dim[1] = v[1],
                   dim[2] = v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator+
    arguments   :   v - vector data
    purpose     :   + operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+(kVector v)
{
    return kVector(dim[0] + v.dim[0],
                   dim[1] + v.dim[1],
                   dim[2] + v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator+
    arguments   :   v - floater array data
    purpose     :   + operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+(float v[3])
{
    return kVector(dim[0] + v[0],
                   dim[1] + v[1],
                   dim[2] + v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator+
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   + operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+(float val)
{
    return kVector(dim[0] + val,
                   dim[1] + val,
                   dim[2] + val);
}

/*******************************************************************************
    function    :   kVector kVector::operator+
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   + operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+(int val)
{
    return kVector(dim[0] + (float)val,
                   dim[1] + (float)val,
                   dim[2] + (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator-
    arguments   :   v - vector data
    purpose     :   - operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-(kVector v)
{
    return kVector(dim[0] - v.dim[0],
                   dim[1] - v.dim[1],
                   dim[2] - v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator-
    arguments   :   v - floater array data
    purpose     :   - operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-(float v[3])
{
    return kVector(dim[0] - v[0],
                   dim[1] - v[1],
                   dim[2] - v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator-
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   - operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-(float val)
{
    return kVector(dim[0] - val,
                   dim[1] - val,
                   dim[2] - val);
}

/*******************************************************************************
    function    :   kVector kVector::operator-
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   - operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-(int val)
{
    return kVector(dim[0] - (float)val,
                   dim[1] - (float)val,
                   dim[2] - (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator*
    arguments   :   v - vector data
    purpose     :   * operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*(kVector v)
{
    return kVector(dim[0] * v.dim[0],
                   dim[1] * v.dim[1],
                   dim[2] * v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator*
    arguments   :   v - floater array data
    purpose     :   * operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*(float v[3])
{
    return kVector(dim[0] * v[0],
                   dim[1] * v[1],
                   dim[2] * v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator*
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   * operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*(float val)
{
    return kVector(dim[0] * val,
                   dim[1] * val,
                   dim[2] * val);
}

/*******************************************************************************
    function    :   kVector kVector::operator*
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   * operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*(int val)
{
    return kVector(dim[0] * (float)val,
                   dim[1] * (float)val,
                   dim[2] * (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator/
    arguments   :   v - vector data
    purpose     :   / operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/(kVector v)
{
    return kVector(dim[0] / v.dim[0],
                   dim[1] / v.dim[1],
                   dim[2] / v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator/
    arguments   :   v - floater array data
    purpose     :   / operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/(float v[3])
{
    return kVector(dim[0] / v[0],
                   dim[1] / v[1],
                   dim[2] / v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator/
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   / operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/(float val)
{
    return kVector(dim[0] / val,
                   dim[1] / val,
                   dim[2] / val);
}

/*******************************************************************************
    function    :   kVector kVector::operator/
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   / operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/(int val)
{
    return kVector(dim[0] / (float)val,
                   dim[1] / (float)val,
                   dim[2] / (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator+=
    arguments   :   v - vector to assign data from
    purpose     :   += operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+=(kVector v)
{
    return kVector(dim[0] += v.dim[0],
                   dim[1] += v.dim[1],
                   dim[2] += v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator+=
    arguments   :   v - floater array to assign data from
    purpose     :   += operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+=(float v[3])
{
    return kVector(dim[0] += v[0],
                   dim[1] += v[1],
                   dim[2] += v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator+=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   += operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+=(float val)
{
    return kVector(dim[0] += val,
                   dim[1] += val,
                   dim[2] += val);
}

/*******************************************************************************
    function    :   kVector kVector::operator+=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   += operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator+=(int val)
{
    return kVector(dim[0] += (float)val,
                   dim[1] += (float)val,
                   dim[2] += (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator-=
    arguments   :   v - vector to assign data from
    purpose     :   -= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-=(kVector v)
{
    return kVector(dim[0] -= v.dim[0],
                   dim[1] -= v.dim[1],
                   dim[2] -= v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator-=
    arguments   :   v - floater array to assign data from
    purpose     :   -= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-=(float v[3])
{
    return kVector(dim[0] -= v[0],
                   dim[1] -= v[1],
                   dim[2] -= v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator-=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   -= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-=(float val)
{
    return kVector(dim[0] -= val,
                   dim[1] -= val,
                   dim[2] -= val);
}

/*******************************************************************************
    function    :   kVector kVector::operator-=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   -= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator-=(int val)
{
    return kVector(dim[0] -= (float)val,
                   dim[1] -= (float)val,
                   dim[2] -= (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator*=
    arguments   :   v - vector to assign data from
    purpose     :   *= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*=(kVector v)
{
    return kVector(dim[0] *= v.dim[0],
                   dim[1] *= v.dim[1],
                   dim[2] *= v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator*=
    arguments   :   v - floater array to assign data from
    purpose     :   *= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*=(float v[3])
{
    return kVector(dim[0] *= v[0],
                   dim[1] *= v[1],
                   dim[2] *= v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator*=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   *= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*=(float val)
{
    return kVector(dim[0] *= val,
                   dim[1] *= val,
                   dim[2] *= val);
}

/*******************************************************************************
    function    :   kVector kVector::operator*=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   *= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator*=(int val)
{
    return kVector(dim[0] *= (float)val,
                   dim[1] *= (float)val,
                   dim[2] *= (float)val);
}

/*******************************************************************************
    function    :   kVector kVector::operator/=
    arguments   :   v - vector to assign data from
    purpose     :   /= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/=(kVector v)
{
    return kVector(dim[0] /= v.dim[0],
                   dim[1] /= v.dim[1],
                   dim[2] /= v.dim[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator/=
    arguments   :   v - floater array to assign data from
    purpose     :   /= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/=(float v[3])
{
    return kVector(dim[0] /= v[0],
                   dim[1] /= v[1],
                   dim[2] /= v[2]);
}

/*******************************************************************************
    function    :   kVector kVector::operator/=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   /= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/=(float val)
{
    return kVector(dim[0] /= val,
                   dim[1] /= val,
                   dim[2] /= val);
}

/*******************************************************************************
    function    :   kVector kVector::operator/=
    arguments   :   val - constant data value (applied to all dimensions)
    purpose     :   /= operator overload.
    notes       :   <none>
*******************************************************************************/
kVector kVector::operator/=(int val)
{
    return kVector(dim[0] /= (float)val,
                   dim[1] /= (float)val,
                   dim[2] /= (float)val);
}

/*******************************************************************************
    function    :   bool operator==
    arguments   :   v1 - first vector data to compare with
                    v2 - second vector data to compare with
    purpose     :   == operator overload.
    notes       :   <none>
*******************************************************************************/
bool operator==(kVector v1, kVector v2)
{
    if(fabsf(v1.dim[0] - v2.dim[0]) <= FP_ERROR &&
       fabsf(v1.dim[1] - v2.dim[1]) <= FP_ERROR &&
       fabsf(v1.dim[2] - v2.dim[2]) <= FP_ERROR)
       return true;
    return false;
}

/*******************************************************************************
    function    :   bool operator==
    arguments   :   v1 - first vector data to compare with
                    v2 - second floater array to compare data with
    purpose     :   == operator overload.
    notes       :   <none>
*******************************************************************************/
bool operator==(kVector v1, float v2[3])
{
    if(fabsf(v1.dim[0] - v2[0]) <= FP_ERROR &&
       fabsf(v1.dim[1] - v2[1]) <= FP_ERROR &&
       fabsf(v1.dim[2] - v2[2]) <= FP_ERROR)
       return true;
    return false;
}

/*******************************************************************************
    function    :   bool operator==
    arguments   :   v1 - first floater array to compare data with
                    v2 - second vector data to compare with
    purpose     :   == operator overload.
    notes       :   <none>
*******************************************************************************/
bool operator==(float v1[3], kVector v2)
{
    if(fabsf(v1[0] - v2.dim[0]) <= FP_ERROR &&
       fabsf(v1[1] - v2.dim[1]) <= FP_ERROR &&
       fabsf(v1[2] - v2.dim[2]) <= FP_ERROR)
       return true;
    return false;
}

/*******************************************************************************
    function    :   bool operator!=
    arguments   :   v1 - first vector data to compare with
                    v2 - second vector data to compare with
    purpose     :   != operator overload.
    notes       :   <none>
*******************************************************************************/
bool operator!=(kVector v1, kVector v2)
{
    if(fabsf(v1.dim[0] - v2.dim[0]) <= FP_ERROR &&
       fabsf(v1.dim[1] - v2.dim[1]) <= FP_ERROR &&
       fabsf(v1.dim[2] - v2.dim[2]) <= FP_ERROR)
       return false;
    return true;
}

/*******************************************************************************
    function    :   bool operator!=
    arguments   :   v1 - first vector data to compare with
                    v2 - second floater array to compare data with
    purpose     :   != operator overload.
    notes       :   <none>
*******************************************************************************/
bool operator!=(kVector v1, float v2[3])
{
    if(fabsf(v1.dim[0] - v2[0]) <= FP_ERROR &&
       fabsf(v1.dim[1] - v2[1]) <= FP_ERROR &&
       fabsf(v1.dim[2] - v2[2]) <= FP_ERROR)
       return false;
    return true;
}

/*******************************************************************************
    function    :   bool operator!=
    arguments   :   v1 - first floater array to compare data with
                    v2 - second vector data to compare with
    purpose     :   != operator overload.
    notes       :   <none>
*******************************************************************************/
bool operator!=(float v1[3], kVector v2)
{
    if(fabsf(v1[0] - v2.dim[0]) <= FP_ERROR &&
       fabsf(v1[1] - v2.dim[1]) <= FP_ERROR &&
       fabsf(v1[2] - v2.dim[2]) <= FP_ERROR)
       return false;
    return true;
}

/*******************************************************************************
    function    :   ostream& operator<<
    arguments   :   out - outstream
                    v - vector data to output
    purpose     :   << out-stream operator overload.
    notes       :   <none>
*******************************************************************************/
ostream& operator<<(ostream &out, kVector v)
{
    // Output vector
    out << '<' << v.dim[0] << ", " << v.dim[1] << ", " << v.dim[2] << '>';

    return out;
}

/*******************************************************************************
    function    :   istream& operator>>
    arguments   :   in - instream
                    v - vector data to place input into
    purpose     :   >> in-stream operator overload.
    notes       :   <none>
*******************************************************************************/
istream& operator>>(istream &in, kVector &v)
{
    // Skip past all white space and all opening braces
    while(in.peek() == '(' || in.peek() == '<' || in.peek() <= ' ')
        in.ignore(1);

    in >> v.dim[0];     // Read 1st dimension

    // Skip past all white space and all seperating marks
    while(in.peek() == ',' || in.peek() <= ' ')
        in.ignore(1);

    in >> v.dim[1];     // Read 2nd dimension

    // Skip past all white space and all seperating marks
    while(in.peek() == ',' || in.peek() <= ' ')
        in.ignore(1);

    in >> v.dim[2];     // Read 3rd dimension

    // Skip ending brace if it exists
    if(in.peek() == ')' || in.peek() == '>')
    {
        in.ignore(1);
    }
    else if(in.peek() == ' ')
    {
        // Otherwise, if there is whitespace, skip past all whitespace
        while(in.peek() == ' ')
            in.ignore(1);

        // And if we eventually reach an ending brace, skip past that too
        if(in.peek() == ')' || in.peek() == '>')
            in.ignore(1);
    }

    return in;
}

/*******************************************************************************
    function    :   kVector vectorIn
    arguments   :   v - vector data
                    coord_system - CS_CARTESIAN or CS_SPHERICAL
    purpose     :   Returns a vector doing a conversion to the supplied coord
                    system.
    notes       :   It is assumed that the coord system being converted to is
                    the opposite of what is stored, e.g. converting to spher-
                    ical coord sytem implies that a cartesian coord system is
                    currently being stored, and visa versa.
*******************************************************************************/
kVector vectorIn(kVector v, int coord_system)
{
    float new_dim[3];
    float value_plug;

    // Follows the Spherical<->Cartesian conversion formulas found at:
    // www.math.montana.edu/frankw/ccp/multiworld/multipleIVP/spherical/body.htm
    // Note: (rho, phi, theta) == (magnitude, pitch, yaw)
    switch(coord_system)
    {
        case CS_CARTESIAN:
            // It is assumed then that the current vector stored is set up
            // in the spherical coordinate system of (rho, phi, theta).
            new_dim[0] = v.dim[0] * sin(v.dim[1]) * sin(v.dim[2]);
            new_dim[1] = v.dim[0] * cos(v.dim[1]);
            new_dim[2] = v.dim[0] * sin(v.dim[1]) * cos(v.dim[2]);
            break;

        case CS_SPHERICAL:
            // Magnitude
            new_dim[0] = sqrt((v.dim[0] * v.dim[0]) + (v.dim[1] * v.dim[1])
                + (v.dim[2] * v.dim[2]));
                
            // Compute Yaw and Pitch
            if(fabsf(v.dim[0]) > FP_ERROR && fabsf(v.dim[2]) > FP_ERROR)
            {
                // Yaw
                if(v.dim[0] >= 0.0)
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + PIHALF;
                else
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + THREEPIHALF;
                
                // Pitch
                value_plug = v.dim[1] / new_dim[0];
                if(value_plug >= 1.0 - FP_ERROR)
                    new_dim[1] = 0.0;
                if(value_plug <= -1.0 + FP_ERROR)
                    new_dim[1] = PI;
                else
                    new_dim[1] = acos(value_plug);
            }
            else
            {
                // No Yaw (since no X or Z)
                new_dim[2] = 0.0;
                
                // Pitch based on sign of Y
                if(v.dim[1] >= -FP_ERROR)
                    new_dim[1] = 0.0;
                else
                    new_dim[1] = PI;
            }
            break;
        
        case CS_YAW_ONLY:
            if(fabsf(v.dim[0]) > FP_ERROR && fabsf(v.dim[2]) > FP_ERROR)
            {
                // Yaw
                if(v.dim[0] >= 0.0)
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + PIHALF;
                else
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + THREEPIHALF;
            }
            else
            {
                // No Yaw (since no X or Z)
                new_dim[2] = 0.0;
            }
            new_dim[0] = 1.0;
            new_dim[1] = PIHALF;
            break;
            
        default:
            new_dim[0] = v.dim[0];
            new_dim[1] = v.dim[1];
            new_dim[2] = v.dim[2];
            break;
    }

    return kVector(new_dim);
}

/*******************************************************************************
    function    :   kVector convertTo
    arguments   :   v - vector data to modify
                    coord_system - CS_CARTESIAN or CS_SPHERICAL
    purpose     :   Modifys a vector by doing a conversion to the supplied coord
                    system.
    notes       :   It is assumed that the coord system being converted to is
                    the opposite of what is stored, e.g. converting to spher-
                    ical coord sytem implies that a cartesian coord system is
                    currently being stored, and visa versa.
*******************************************************************************/
kVector convertTo(kVector &v, int coord_system)
{
    float new_dim[3];
    float value_plug;

    // Follows the Spherical<->Cartesian conversion formulas found at:
    // www.math.montana.edu/frankw/ccp/multiworld/multipleIVP/spherical/body.htm
    // Note: (rho, phi, theta) == (magnitude, pitch, yaw)
    switch(coord_system)
    {
        case CS_CARTESIAN:
            // It is assumed then that the current vector stored is set up
            // in the spherical coordinate system of (rho, phi, theta).
            new_dim[0] = v.dim[0] * sin(v.dim[1]) * sin(v.dim[2]);
            new_dim[1] = v.dim[0] * cos(v.dim[1]);
            new_dim[2] = v.dim[0] * sin(v.dim[1]) * cos(v.dim[2]);
            break;
            
        case CS_SPHERICAL:
            // Magnitude
            new_dim[0] = sqrt((v.dim[0] * v.dim[0]) + (v.dim[1] * v.dim[1])
                + (v.dim[2] * v.dim[2]));
                
            // Compute Yaw and Pitch
            if(fabsf(v.dim[0]) > FP_ERROR && fabsf(v.dim[2]) > FP_ERROR)
            {
                // Yaw
                if(v.dim[0] >= 0.0)
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + PIHALF;
                else
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + THREEPIHALF;
                
                // Pitch
                value_plug = v.dim[1] / new_dim[0];
                if(value_plug >= 1.0 - FP_ERROR)
                    new_dim[1] = 0.0;
                if(value_plug <= -1.0 + FP_ERROR)
                    new_dim[1] = PI;
                else
                    new_dim[1] = acos(value_plug);
            }
            else
            {
                // No Yaw (since no X or Z)
                new_dim[2] = 0.0;
                
                // Pitch based on sign of Y
                if(v.dim[1] >= -FP_ERROR)
                    new_dim[1] = 0.0;
                else
                    new_dim[1] = PI;
            }
            break;
        
        case CS_YAW_ONLY:
            if(fabsf(v.dim[0]) > FP_ERROR && fabsf(v.dim[2]) > FP_ERROR)
            {
                // Yaw
                if(v.dim[0] >= 0.0)
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + PIHALF;
                else
                    new_dim[2] = atan(-v.dim[2] / v.dim[0]) + THREEPIHALF;
            }
            else
            {
                // No Yaw (since no X or Z)
                new_dim[2] = 0.0;
            }
            new_dim[0] = 1.0;
            new_dim[1] = PIHALF;
            break;
        
        default:
            new_dim[0] = v.dim[0];
            new_dim[1] = v.dim[1];
            new_dim[2] = v.dim[2];
            break;
    }

    v.dim[0] = new_dim[0];
    v.dim[1] = new_dim[1];
    v.dim[2] = new_dim[2];

    return kVector(new_dim);
}

/*******************************************************************************
    function    :   float magnitude
    arguments   :   v - vector data
    purpose     :   Returns magnitude of vector.
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
float magnitude(kVector v)
{
    return sqrt((v.dim[0] * v.dim[0]) +
                (v.dim[1] * v.dim[1]) +
                (v.dim[2] * v.dim[2]));
}

/*******************************************************************************
    function    :   kVector normalized
    arguments   :   v - vector data
    purpose     :   Returns a normalized form of the vector (magnitude = 1.0).
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
kVector normalized(kVector v)
{
    float mag = magnitude(v);

    return kVector(v.dim[0] / mag,
                   v.dim[1] / mag,
                   v.dim[2] / mag);
}

/*******************************************************************************
    function    :   kVector normalize
    arguments   :   v - vector data to modify
    purpose     :   Normalizes the vector (magnitude = 1.0).
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
kVector normalize(kVector &v)
{
    return (v = normalized(v));
}

/*******************************************************************************
    function    :   kVector transformed
    arguments   :   v - vector data to modify
                    matrix - OpenGL based 4x4 matrix
    purpose     :   Returns a linear transformation applied to the passed vector
                    based on the passed OpenGL based 4x4 matrix (which exists
                    in column major form as an array of 16 elements).
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
kVector transformed(kVector v, float* matrix)
{
    float resultant[3];
    
    resultant[0] = matrix[0]  * v[0] +
                   matrix[4]  * v[1] +
                   matrix[8]  * v[2] +
                   matrix[12];
    resultant[1] = matrix[1]  * v[0] +
                   matrix[5]  * v[1] +
                   matrix[9]  * v[2] +
                   matrix[13];
    resultant[2] = matrix[2]  * v[0] +
                   matrix[6]  * v[1] +
                   matrix[10] * v[2] +
                   matrix[14];
                   
    // In homogenous coordinates, resultant[3] *should* always compute out to
    // be 1.0. In tests ran it has shown to be correct with the above code, so
    // the computation of the 4th row has been left out since it's a needless
    // computation.
    
    return kVector(resultant);
}

/*******************************************************************************
    function    :   kVector transform
    arguments   :   v - vector data to modify
                    matrix - OpenGL based 4x4 matrix
    purpose     :   Applies a linear transformation to the passed vector based
                    on the passed OpenGL based 4x4 matrix (which exists in
                    column major form as an array of 16 elements).
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
kVector transform(kVector &v, float* matrix)
{
    return (v = transformed(v, matrix));
}

/*******************************************************************************
    function    :   kVector crossProduct
    arguments   :   v1 - first vector data
                    v2 - second vector data
    purpose     :   Produces the cross product of v1 with v2.
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
kVector crossProduct(kVector v1, kVector v2)
{
    return kVector(
            (v1.dim[1] * v2.dim[2]) - (v1.dim[2] * v2.dim[1]),
            (v1.dim[2] * v2.dim[0]) - (v1.dim[0] * v2.dim[2]),
            (v1.dim[0] * v2.dim[1]) - (v1.dim[1] * v2.dim[0]));
}

/*******************************************************************************
    function    :   float dotProduct
    arguments   :   v1 - first vector data
                    v2 - second vector data
    purpose     :   Produces the dot product of v1 with v2.
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
float dotProduct(kVector v1, kVector v2)
{
    return (v1.dim[0] * v2.dim[0]) +
           (v1.dim[1] * v2.dim[1]) +
           (v1.dim[2] * v2.dim[2]);
}

/*******************************************************************************
    function    :   float angleBetween
    arguments   :   v1 - first vector data
                    v2 - second vector data
    purpose     :   Produces the compound angle between v1 and v2.
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
float angleBetween(kVector v1, kVector v2)
{
    return acos(dotProduct(v1, v2) / (magnitude(v1) * magnitude(v2)));
}

/*******************************************************************************
    function    :   float angleBetweenNormals
    arguments   :   v1 - first vector data
                    v2 - second vector data
    purpose     :   Produces the compound angle between v1 and v2.
    notes       :   1) Vector must be based in the cartesian coordinate system.
                    2) Both vectors MUST be normalized.
*******************************************************************************/
float angleBetweenNormals(kVector v1, kVector v2)
{
    return acos(dotProduct(v1, v2));
}

/*******************************************************************************
    function    :   float distanceBetween
    arguments   :   v1 - first vector data
                    v2 - second vector data
    purpose     :   Produces the distance between v1 and v2.
    notes       :   Vector must be based in the cartesian coordinate system.
*******************************************************************************/
float distanceBetween(kVector v1, kVector v2)
{
    return magnitude(v2 - v1);
}
