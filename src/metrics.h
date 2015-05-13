/*******************************************************************************
                             Metrics - Definition
*******************************************************************************/

#ifndef METRICS_H
#define METRICS_H

/*  Commonly Used Values & Constant Declarations  */
#define radToDeg                57.2958
#define degToRad                0.017453
#define PI                      3.141592
#define TWOPI                   6.283185
#define PIHALF                  1.570796
#define PIFOURTH                0.7853982
#define THREEPIHALF             4.712389

#define CS_CARTESIAN            0
#define CS_SPHERICAL            1
#define CS_YAW_ONLY             2

#define FP_ERROR                0.00001

/*******************************************************************************
    class       :   kVector
    purpose     :   The building blocks of anything being done in 3D relies on
                    a versatile and powerful vector class.
    notes       :   1) It is assumed that the user of the class will decern the
                       different coordinate systems being used, e.g. this class
                       does NOT track the coordinate system in use.
                    2) This class offers a wide variety of operator overloads:
                       +, -, *. /, +=, -=, *=, /=. ==, !=, and <</>> io streams.
                    3) Vector functions are both declared as member and non-
                       member functions. Member functions will in effect call
                       the corresponding non-member function.
                    4) This class does offer conversion between the cartesian
                       and spherical coord. sys. Use of the CS_ variable
                       declares are used in passing what CS to convert to.
                    5) Vector functions assume that the current CS in use is
                       the cartesian CS, and will NOT work on Spherical systems.
                    6) Vector functions are described as follows:
                       - vectorIn: Returns a form of the vector in the specified
                            CS. Note that this doesn't modify the source vector.
                       - convertTo: Converts the vector to the specified CS. Do
                            note that this does modify the source vector.
                       - magnitude: Returns the magnitude of the vector. Note
                            that this doesn't modify the source vector.
                       - normalized: Returns the normalized form of the vector.
                            Note that this doesn't modify the source vector.
                       - normalize: Normalizes the vector. Do note that this
                            does modify the source vector.
                       - transformed: Returns the linear transformation form
                            if the vector was to be passed through a 4x4 OpenGL
                            based matrix (such as the MODELVIEW matrix).
                       - transform: Applys a linear transformation to the vector
                            by passing it through a 4x4 OpenGL based matrix
                            (such as the MODELVIEW matrix). Do note that this
                            does modify the source vector.
                       - crossProduct: Returns the crossProduct of two vectors.
                            Note that this doesn't modify the source vectors.
                       - dotProduct: Returns the dotProduct of two vectors.
                            Note that this doesn't modify the source vectors.
                       - angleBetween: Returns the angle formed between two
                            vectors (in radians). Multiply times radToDeg to
                            get the corresponding measurement in degrees. Note
                            that this doesn't modify the source vectors.                       
                    7) Although there could be more work done which could expand
                       the functionality of the vector functions to cover simple
                       float arrays, the memory usage question comes into play
                       and thus currently it is recommended to "promote" any
                       float arrays to kVector types, via: kVector(float_array),
                       and then use the vector functions.
*******************************************************************************/
class kVector
{
    private:
        float dim[3];

    public:
        kVector();                                      // Constructors
        kVector(float ndim[3]);
        kVector(float d1, float d2, float d3);
        ~kVector();                                     // Deconstructor
        
        float* getArray() { return dim; }

        /* Base Operator Overloads */
        float* operator()();                            // Base float Array Get
        float& operator[](unsigned int element);        // Index
        kVector operator=(kVector v);                   // Assignment
        kVector operator=(float v[3]);
        kVector operator+(kVector v);                   // Addition
        kVector operator+(float v[3]);
        kVector operator+(float val);
        kVector operator+(int val);
        kVector operator-(kVector v);                   // Subtraction
        kVector operator-(float v[3]);
        kVector operator-(float val);
        kVector operator-(int val);
        kVector operator*(kVector v);                   // Multiplication
        kVector operator*(float v[3]);
        kVector operator*(float val);
        kVector operator*(int val);
        kVector operator/(kVector v);                   // Division
        kVector operator/(float v[3]);
        kVector operator/(float val);
        kVector operator/(int val);
        kVector operator+=(kVector v);                  // Assign Addition
        kVector operator+=(float v[3]);
        kVector operator+=(float val);
        kVector operator+=(int val);
        kVector operator-=(kVector v);                  // Assign Subtraction
        kVector operator-=(float v[3]);
        kVector operator-=(float val);
        kVector operator-=(int val);
        kVector operator*=(kVector v);                  // Assign Multiplication
        kVector operator*=(float v[3]);
        kVector operator*=(float val);
        kVector operator*=(int val);
        kVector operator/=(kVector v);                  // Assign Division
        kVector operator/=(float v[3]);
        kVector operator/=(float val);
        kVector operator/=(int val);

        /* Equality Overloads */
        friend bool operator==(kVector v1, kVector v2);
        friend bool operator==(kVector v1, float v2[3]);
        friend bool operator==(float v1[3], kVector v2);
        friend bool operator!=(kVector v1, kVector v2);
        friend bool operator!=(kVector v1, float v2[3]);
        friend bool operator!=(float v1[3], kVector v2);

        /* Stream Overloads */
        friend ostream& operator<<(ostream &out, kVector v);
        friend istream& operator>>(istream &in, kVector &v);

        /* Vector Functions */
        // Note: These are declared both as member and non-member functions
        //       purely in an attempt to maximize code flexibility.
        //       !! THESE FUNCTIONS ONLY APPLY TO CARTESIAN COORD. SYSTEMS !!
        
        // Vector Form In Coord.Sys. (does not effect vector)
        friend kVector vectorIn(kVector v, int coord_system);
        inline kVector vectorIn(int coord_system)
            {return ::vectorIn(*this, coord_system); }

        // Convert Vector To Coord.Sys. (effects vector!)
        friend kVector convertTo(kVector &v, int coord_system);
        inline kVector convertTo(int coord_system)
            { return ::convertTo(*this, coord_system); }

        // Magnitude of Vector (does not effect vector)
        friend float magnitude(kVector v);
        inline float magnitude()
            { return ::magnitude(*this); }

        // Normalized Form of Vector (does not effect vector)
        friend kVector normalized(kVector v);
        inline kVector normalized()
            { return ::normalized(*this); }
        
        // Normalize Vector (effects vector!)
        friend kVector normalize(kVector &v);
        inline kVector normalize()
            { return ::normalize(*this); }
            
        // Form of Linear Transform with a 4x4 Matrix (does not effects vector)
        friend kVector transformed(kVector v, float* matrix);
        inline kVector transformed(float* matrix)
            { return ::transform(*this, matrix); }
            
        // Linear Transformation with a 4x4 Matrix (effects vector!)
        friend kVector transform(kVector &v, float* matrix);
        inline kVector transform(float* matrix)
            { return ::transform(*this, matrix); }

        // Cross Product of two Vectors (does not effect either vector)
        friend kVector crossProduct(kVector v1, kVector v2);
        inline kVector crossProduct(kVector v)
            { return ::crossProduct(*this, v); }

        // Dot Product of two Vectors (does not effect either vector)
        friend float dotProduct(kVector v1, kVector v2);
        inline float dotProduct(kVector v)
            { return ::dotProduct(*this, v); }
            
        // Angle Between two Vectors (does not effect either vector)
        friend float angleBetween(kVector v1, kVector v2);
        inline float angleBetween(kVector v)
            { return ::angleBetween(*this, v); }
        friend float angleBetweenNormals(kVector v1, kVector v2);
        inline float angleBetweenNormals(kVector v)
            { return ::angleBetweenNormals(*this, v); }
        
        // Angle Between two Vectors (does not effect either vector)
        friend float distanceBetween(kVector v1, kVector v2);
        inline float distanceBetween(kVector v)
            { return ::distanceBetween(*this, v); }
};

#endif
