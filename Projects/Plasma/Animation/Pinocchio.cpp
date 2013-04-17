/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#include <Plasma/Animation/Pinocchio.h>
#include <Plasma/Geometry/SurfaceGeometry.h>
#include <Plasma/Geometry/MeshGeometry.h>
#include <Plasma/Animation/Skeleton.h>
#include <Plasma/Intersector.h>

#include <CGMath/CGMath.h>
#include <CGMath/Grid.h>
#include <CGMath/Vec3.h>

#include <Base/Util/RCP.h>
#include <Base/ADT/Pair.h>

#include <queue>
#include <set>
#include <vector>
#include <map>
#include <algorithm>
#include <assert.h>

#include <math.h>
//#include <functional>

using namespace std;

//==============================================================================
//==============================================================================
// hashutils.h (start)
//==============================================================================
//==============================================================================
/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined( _MSC_VER )
#include <ext/hash_map>
#include <ext/hash_set>

#define _HASH_NAMESPACE __gnu_cxx

using namespace _HASH_NAMESPACE;

namespace _HASH_NAMESPACE {
    template<class T1, class T2> struct hash<pair<T1, T2> >
    {
        size_t operator()(const pair<T1, T2> &p) const { return hash<T1>()(p.first) + 37 * hash<T2>()(p.second); }
    };

    template<class T> struct hash<T *>
    {
        size_t operator()(T *p) const { return (size_t)p; }
    };
}

#define MAKE_HASH(type, code) \
    namespace _HASH_NAMESPACE { \
        template<> struct hash<type> \
        { \
            size_t operator()(const type &p) const { code } \
        }; \
    }
#else //MICROSOFT VC 2005
#include <hash_map>
#include <hash_set>

#define _HASH_NAMESPACE stdext

using namespace _HASH_NAMESPACE;

namespace _HASH_NAMESPACE {
#if (_MSC_VER >= 1700)
#else
    template<class T> struct hash
    {
        size_t operator()(const T &p) { return hash_compare<T>()(p); }
    };
#endif

    template<class T1, class T2> struct hash_compare<std::pair<T1, T2> >
    {
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        size_t operator()(const std::pair<T1, T2> &p) const { return hash_compare<T1>()(p.first) + 37 * hash_compare<T2>()(p.second); }
        bool operator()(const std::pair<T1, T2> &p1, const std::pair<T1, T2> &p2) const { return p1 < p2; }
    };

    template<class T> struct hash_compare<T *>
    {
        static const size_t bucket_size = 4;
        static const size_t min_buckets = 8;
        size_t operator()(T *p) const { return (size_t)p; }
        bool operator()(T *p1, T *p2) const { return p1 < p2; }
    };
}

#define MAKE_HASH(type, code) \
    namespace _HASH_NAMESPACE { \
        template<> struct hash_compare<type> \
        { \
            static const size_t bucket_size = 4; \
            static const size_t min_buckets = 8; \
            size_t operator()(const type &p) const { code } \
            bool operator()(const type &p1, const type &p2) const { return p1 < p2; } \
        }; \
    }
#endif

//==============================================================================
//==============================================================================
// hashutils.h (end)
//==============================================================================
//==============================================================================


//------------------------------------------------------------------------------
//!
UNNAMESPACE_BEGIN

//==============================================================================
//==============================================================================
// kinda vecutils.h (start)
//==============================================================================
//==============================================================================

float distsqToSeg( const Vec3f& v, const Vec3f& p1, const Vec3f& p2 )
{
   Vec3f dir   = p2 - p1;
   Vec3f difp2 = p2 - v;

   if( difp2.dot(dir) <= 0.0f )
   {
      return difp2.sqrLength();
   }

   Vec3f difp1 = v - p1;
   float dot   = difp1.dot(dir);

   if( dot <= 0.0f )
   {
      return difp1.sqrLength();
   }

   return CGM::max( 0.0f, difp1.sqrLength() - (dot*dot) / dir.sqrLength() );
}

//------------------------------------------------------------------------------
//!
Vec3f projToSeg( const Vec3f& v, const Vec3f& p1, const Vec3f& p2 )
{
   Vec3f dir = p2 - p1;

   if( (p2 - v).dot(dir) <= 0.0f )
   {
      return p2;
   }

   float dot = (v - p1).dot(dir);

   if( dot <= 0.0f )
   {
      return p1;
   }

   return p1 + dir * (dot / dir.sqrLength());
}
//==============================================================================
//==============================================================================
// kinda vecutils.h (end)
//==============================================================================
//==============================================================================


//==============================================================================
//==============================================================================
// mathutils.h (start)
//==============================================================================
//==============================================================================
/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


template<class T> T SQR(const T & x) { return x * x; }

//==============================================================================
//==============================================================================
// mathutils.h (end)
//==============================================================================
//==============================================================================


//==============================================================================
//==============================================================================
// lsqsolver.h (start)
//==============================================================================
//==============================================================================
/*  This file is part of the Pinocchio automatic rigging library.
    Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


/**
* Represents a factored spd matrix -- primary intended use is inside LSQSystem
*/
class LLTMatrix
{
public:
    virtual ~LLTMatrix() {}
    virtual bool solve(std::vector<double> &b) const = 0;
    virtual int size() const = 0;
};

/**
* Represents a symmetric positive definite (spd) matrix --
* primary intended use is inside LSQSystem (because it's symmetric, only the lower triangle
* is stored)
*/
class SPDMatrix
{
public:
    SPDMatrix(const std::vector<std::vector<std::pair<int, double> > > &inM) : m(inM) {}
    LLTMatrix *factor() const;

private:
    std::vector<int> computePerm() const; //computes a fill-reduction permutation

    std::vector<std::vector<std::pair<int, double> > > m; //rows -- lower triangle
};

/**
* Sparse linear least squares solver -- with support for hard constraints
* Intended usage:
*    LSQSystem<V, C> s;
*    [
*      s.addConstraint(...); //boolean flag specifies whether soft or hard
*    ]
*    s.factor();
*    [
*      [
*        s.setRhs(...);
*      ]
*      s.solve();
*      [
*        ... = s.getResult(...);
*      ]
*    ]
* Where the stuff in brackets [] may be repeated multiple times
*/
template<class V, class C> class LSQSystem
{
public:
    LSQSystem() : factoredMatrix(NULL) {}
    ~LSQSystem() { if(factoredMatrix) delete factoredMatrix; }

    void addConstraint(bool hard, const std::map<V, double> &lhs, const C &id)
    {
        constraints[make_pair(id, -1)] = Constraint(hard, lhs);
    }

    void addConstraint(bool hard, double rhs, const std::map<V, double> &lhs)
    {
        constraints[make_pair(C(), (int)constraints.size())] = Constraint(hard, lhs, rhs);
    }

    void setRhs(const C &id, double rhs)
    {
        assert(constraints.count(make_pair(id, -1)));
        constraints[make_pair(id, -1)].rhs = rhs;
    }

    bool factor()
    {
        int i;
        typename std::map<std::pair<C, int>, Constraint>::iterator it;

        //This is a mess.  This function needs to assign indices to both variables and
        //constraints, row-reduce the hard-constraint matrix, and construct the soft constraint
        //matrix by substituting the reduced hard-constraints into the soft constraints.
        //All while keeping track of what's happening to the right hand side of the
        //system.
        //The result is somewhat difficult to follow.
        //One possible optimization (optimize factorization and especially solving in
        //the presence of large amounts of equality constraints and the like)
        //is to treat the constraints whose rhs will not change separately.
        //The way to make it prettier (should this become necessary) is to
        //make structures for the different types of indices (instead of using ints and pairs)
        //also put hard constraints in front rather than at the end.

        //init
        varIds.clear();
        constraintMap.clear();
        substitutedHard.clear();
        rhsTransform.clear();
        softMatrix.clear();
        softNum = 0;

        //assign indices to soft constraints
        for(it = constraints.begin(); it != constraints.end(); ++it) {
            if(!it->second.hard) {
                constraintMap[it->first] = softNum++;
            }
        }

        //isolate hard constraints
        std::vector<std::map<V, double> > hardConstraints;
        std::vector<std::pair<C, int> > hardConstraintIds;
        int hardNum = 0;
        for(it = constraints.begin(); it != constraints.end(); ++it) {
            if(it->second.hard) {
                hardConstraints.push_back(it->second.lhs);
                hardConstraintIds.push_back(it->first);
                ++hardNum;
            }
        }

        std::map<V, std::map<V, double> > substitutions; //"substitutions[x] = (y, 3.), (z, 2.)" means "x = 3y+2z + c"
        std::map<V, std::map<std::pair<C, int>, double> > substitutionsRhs; //the "c" in the above equation as a lin.comb. of rhs's
        std::map<V, int> substitutionConstraintIdx; //keeps track of which constraint a substitution came from
        std::vector<std::map<std::pair<C, int>, double> > hardRhs(hardConstraints.size());
        for(i = 0; i < (int)hardConstraints.size(); ++i)
            hardRhs[i][hardConstraintIds[i]] = 1.;
        while(hardConstraints.size()) {
            typename std::map<V, double>::iterator it;
            typename std::map<std::pair<C, int>, double>::iterator rit;
            //find best variable and equation -- essentially pivoting
            V bestVar;
            int bestEq = -1;
            double bestVal = 0;
            for(i = 0; i < (int)hardConstraints.size(); ++i) {
                for(it = hardConstraints[i].begin(); it != hardConstraints[i].end(); ++it) {
                    //take the variable with the max absolute weight, but also heavily
                    //prefer variables with simple substitutions
                    double val = fabs(it->second) / (double(hardConstraints[i].size()) - 0.9);
                    if(val > bestVal) {
                        bestVal = val;
                        bestEq = i;
                        bestVar = it->first;

                        //an equality or hard assignment constraint is always good enough
                        if(val > .5 && hardConstraints[i].size() <= 2) {
                            i = hardConstraints.size(); //break from the outer loop as well
                            break;
                        }
                    }
                }
            }

            if(bestVal < 1e-10)
                return false; //near-singular matrix

            substitutionConstraintIdx[bestVar] = substitutions.size();
            substitutionsRhs[bestVar] = hardRhs[bestEq];
            constraintMap[hardConstraintIds[bestEq]] = softNum + substitutions.size();

            swap(hardConstraints[bestEq], hardConstraints.back());
            swap(hardConstraintIds[bestEq], hardConstraintIds.back());
            swap(hardRhs[bestEq], hardRhs.back());
            double factor = -1. / hardConstraints.back()[bestVar];
            //figure out the substitution
            std::map<V, double> &curSub = substitutions[bestVar];
            std::map<std::pair<C, int>, double> &curSubRhs = substitutionsRhs[bestVar];
            for(it = hardConstraints.back().begin(); it != hardConstraints.back().end(); ++it) {
                if(it->first != bestVar)
                    curSub[it->first] = it->second * factor;
            }
            for(rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit)
                rit->second *= -factor;

            //now substitute into the unprocessed hard constraints
            hardConstraints.pop_back();
            hardConstraintIds.pop_back();
            hardRhs.pop_back();
            for(i = 0; i < (int)hardConstraints.size(); ++i) {
                if(hardConstraints[i].count(bestVar) == 0)
                    continue;
                double varWeight = hardConstraints[i][bestVar];
                hardConstraints[i].erase(bestVar);
                for(it = curSub.begin(); it != curSub.end(); ++it)
                    hardConstraints[i][it->first] += it->second * varWeight;
                //and the rhs
                for(rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit)
                    hardRhs[i][rit->first] -= rit->second * varWeight;
            }

            //now substitute into the other substitutions
            typename std::map<V, std::map<V, double> >::iterator sit;
            for(sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
                if(sit->second.count(bestVar) == 0)
                    continue;
                double varWeight = sit->second[bestVar];
                sit->second.erase(bestVar);
                for(it = curSub.begin(); it != curSub.end(); ++it)
                    sit->second[it->first] += it->second * varWeight;
                //and the rhs
                std::map<std::pair<C, int>, double> &srhs = substitutionsRhs[sit->first];
                for(rit = curSubRhs.begin(); rit != curSubRhs.end(); ++rit) {
                    srhs[rit->first] += rit->second * varWeight;
                }
            }
        }

        //now that we know which variables are determined by hard constraints, give indices to the rest
        std::map<V, int> varMap; //maps variables to indices
        //variables from soft constraints first
        for(it = constraints.begin(); it != constraints.end(); ++it) {
            if(it->second.hard)
                continue;
            typename std::map<V, double>::iterator it2;
            for(it2 = it->second.lhs.begin(); it2 != it->second.lhs.end(); ++it2) {
                if(varMap.count(it2->first) || substitutions.count(it2->first))
                    continue;
                varMap[it2->first] = varIds.size();
                varIds.push_back(it2->first);
            }
        }
        int softVars = varIds.size();
        //then the hard constraint variables
        varIds.resize(softVars + hardNum);
        typename std::map<V, std::map<V, double> >::iterator sit;
        for(sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
            int idx = substitutionConstraintIdx[sit->first] + softVars;
            varMap[sit->first] = idx;
            varIds[idx] = sit->first;
        }

        //now compute substitutedHard -- the substitutions with respect to the indices
        substitutedHard.resize(substitutions.size());
        for(sit = substitutions.begin(); sit != substitutions.end(); ++sit) {
            typename std::map<V, double>::iterator it;
            int idx = substitutionConstraintIdx[sit->first];
            for(it = sit->second.begin(); it != sit->second.end(); ++it) {
                if(varMap.count(it->first) == 0)
                    return false; //variable is left free by both hard and soft constraints--bad system
                substitutedHard[idx].push_back(make_pair(varMap[it->first], it->second));
            }
        }

        //compute the softMatrix (and the rhs transform)
        std::vector<std::map<int, double> > rhsTransformMap(hardNum); //the rhsTransform matrix as a map
        softMatrix.resize(softNum);
        for(it = constraints.begin(); it != constraints.end(); ++it) {
            if(it->second.hard)
                continue;
            std::map<V, double> modLhs = it->second.lhs;
            typename std::map<V, double>::iterator it2, it3;
            int idx = constraintMap[it->first];
            for(it2 = it->second.lhs.begin(); it2 != it->second.lhs.end(); ++it2) {
                if(substitutions.count(it2->first) == 0)
                    continue;
                double fac = it2->second;
                std::map<V, double> &curSub = substitutions[it2->first];
                for(it3 = curSub.begin(); it3 != curSub.end(); ++it3) {
                    modLhs[it3->first] += fac * it3->second;
                }
                std::map<std::pair<C, int>, double> &curRhsSub = substitutionsRhs[it2->first];
                typename std::map<std::pair<C, int>, double>::iterator it4;
                for(it4 = curRhsSub.begin(); it4 != curRhsSub.end(); ++it4) {
                    rhsTransformMap[constraintMap[it4->first] - softNum][idx] -= fac * it4->second;
                }
            }
            for(it2 = modLhs.begin(); it2 != modLhs.end(); ++it2) { //write modLhs into the right form
                if(substitutions.count(it2->first))
                    continue;
                softMatrix[idx].push_back(make_pair(varMap[it2->first], it2->second));
            }
            sort(softMatrix[idx].begin(), softMatrix[idx].end());
        }

        //add the rhs transforms for the hard constraints
        //and get the rhsTransform into the right form
        typename std::map<V, std::map<std::pair<C, int>, double> >::iterator rit;
        for(rit = substitutionsRhs.begin(); rit != substitutionsRhs.end(); ++rit) {
            typename std::map<std::pair<C, int>, double>::iterator it;
            int idx = substitutionConstraintIdx[rit->first] + softNum;
            for(it = rit->second.begin(); it != rit->second.end(); ++it) {
                rhsTransformMap[constraintMap[it->first] - softNum][idx] += it->second;
            }
        }
        for(i = 0; i < hardNum; ++i) {
            rhsTransform.push_back(std::vector<std::pair<int, double> >(rhsTransformMap[i].begin(),
                                                              rhsTransformMap[i].end()));
        }

        //multiply the softMatrix by its transpose to get an SPDMatrix
        std::vector<std::vector<std::pair<int, double> > > spdm; //the lower triangle of A^T * A
        std::vector<std::map<int, double> > spdMap(softVars); //the lower triangle of A^T * A as a map
        for(i = 0; i < (int)softMatrix.size(); ++i) {
            int j, k;
            for(j = 0; j < (int)softMatrix[i].size(); ++j) for(k = 0; k <= j; ++k) {
                spdMap[softMatrix[i][j].first][softMatrix[i][k].first] +=
                    softMatrix[i][j].second * softMatrix[i][k].second;
            }
        }
        for(i = 0; i < softVars; ++i)
            spdm.push_back(std::vector<std::pair<int, double> >(spdMap[i].begin(), spdMap[i].end()));

        //factor the SPDMatrix to get the LLTMatrix
        SPDMatrix spdMatrix(spdm);
        if(factoredMatrix)
            delete factoredMatrix;
        factoredMatrix = spdMatrix.factor();
        if(factoredMatrix->size() != softVars)
            return false;

        return true;
    }

    bool solve()
    {
        result.clear();
        typename std::map<std::pair<C, int>, Constraint>::const_iterator it;
        std::vector<double> rhs0, rhs1;

        //grab the rhs's of the constraints
        for(it = constraints.begin(); it != constraints.end(); ++it) {
            int idx = constraintMap[it->first];
            if((int)rhs0.size() <= idx)
                rhs0.resize(idx + 1, 0.);
            rhs0[idx] = it->second.rhs;
        }

        rhs1 = rhs0;
        int i, j;
        for(i = softNum; i < (int)rhs1.size(); ++i)
            rhs1[i] = 0; //for hard constraints, transform is absolute, not "additive"
        //transform them (as per hard constraints substitution)
        for(i = 0; i < (int)rhsTransform.size(); ++i) {
            for(j = 0; j < (int)rhsTransform[i].size(); ++j) {
                rhs1[rhsTransform[i][j].first] += rhsTransform[i][j].second * rhs0[softNum + i];
            }
        }

        //multiply by A^T (as in (A^T A)^-1 x = A^T b )
        std::vector<double> rhs2(factoredMatrix->size(), 0);
        for(i = 0; i < (int)softMatrix.size(); ++i) { //i is row
            for(j = 0; j < (int)softMatrix[i].size(); ++j) { //softMatrix[i][j].first is column
                int col = softMatrix[i][j].first;
                //but the matrix is transposed :)
                rhs2[col] += softMatrix[i][j].second * rhs1[i];
            }
        }

        if(!factoredMatrix->solve(rhs2))
            return false;

        for(i = 0; i < (int)rhs2.size(); ++i) {
            result[varIds[i]] = rhs2[i];
        }

        //now solve for the hard constraints
        int hardNum = (int)varIds.size() - (int)rhs2.size();
        for(i = 0; i < hardNum; ++i) {
            double cur = rhs1[softNum + i];
            for(j = 0; j < (int)substitutedHard[i].size(); ++j) {
                cur += substitutedHard[i][j].second * rhs2[substitutedHard[i][j].first];
            }
            result[varIds[i + rhs2.size()]] = cur;
        }

        return true;
    }

    double getResult(const V &var) const
    {
        assert(result.find(var) != result.end());
        return result.find(var)->second;
    }

private:
    struct Constraint {
        Constraint() {}
        Constraint(bool inHard, const std::map<V, double> &inLhs, double inRhs = 0.)
            : hard(inHard), lhs(inLhs), rhs(inRhs) {}

        bool hard;
        std::map<V, double> lhs;
        double rhs;
    };

    std::map<std::pair<C, int>, Constraint> constraints;

    //set during solve
    std::map<V, double> result;

    //variables set during factor
    int softNum; //number of soft constraints
    std::vector<V> varIds; //first the variables softly solved for, then the ones substituted
    std::map<std::pair<C, int>, int> constraintMap;
    std::vector<std::vector<std::pair<int, double> > > substitutedHard;
    std::vector<std::vector<std::pair<int, double> > > rhsTransform;
    std::vector<std::vector<std::pair<int, double> > > softMatrix;
    LLTMatrix *factoredMatrix;
};

class MyLLTMatrix : public LLTMatrix
{
public:
    bool solve(std::vector<double> &b) const; //solves it in place
    int size() const { return (int)m.size(); }

private:
    void initMt();
    std::vector<std::vector<std::pair<int, double> > > m; //off-diagonal values stored by rows
    std::vector<std::vector<std::pair<int, double> > > mt; //off-diagonal values transposed stored by rows
    std::vector<double> diag; //values on diagonal
    std::vector<int> perm; //permutation

    friend class SPDMatrix;
};


std::vector<int> SPDMatrix::computePerm() const
{
    int i, j;

    std::vector<int> out;
    int sz = (int)m.size();

#if 0 //No permutation
    out.resize(sz);
    for(i = 0; i < sz; ++i)
        out[i] = i;
    return out;
    random_shuffle(out.begin(), out.end());
    return out;
#endif

    //initialize
    std::set<std::pair<int, int> > neighborSize;
    std::vector<hash_set<int> > neighbors(sz);
    for(i = 0; i < sz; ++i) {
        for(j = 0; j < (int)m[i].size() - 1; ++j) {
            neighbors[i].insert(m[i][j].first);
            neighbors[m[i][j].first].insert(i);
        }
    }
    for(i = 0; i < sz; ++i)
        neighborSize.insert(std::make_pair((int)neighbors[i].size(), i));

    //iterate
    while(!neighborSize.empty()) {
        //remove the neighbor of minimum degree
        int cur = (neighborSize.begin())->second;
        neighborSize.erase(neighborSize.begin());

        out.push_back(cur);

        //collect the neighbors of eliminated vertex
        std::vector<int> nb(neighbors[cur].begin(), neighbors[cur].end());
        //erase them from the neighborSize set because their neighborhood sizes are about
        //to change
        for(i = 0; i < (int)nb.size(); ++i)
            neighborSize.erase(std::make_pair((int)neighbors[nb[i]].size(), nb[i]));
        //erase the eliminated vertex from their neighbor lists
        for(i = 0; i < (int)nb.size(); ++i)
            neighbors[nb[i]].erase(neighbors[nb[i]].find(cur));
        //make the neighbors all adjacent
        for(i = 0; i < (int)nb.size(); ++i) for(j = 0; j < i; ++j) {
            if(neighbors[nb[i]].count(nb[j]) == 0) {
                neighbors[nb[i]].insert(nb[j]);
                neighbors[nb[j]].insert(nb[i]);
            }
        }
        //and put them back into the neighborSize set
        for(i = 0; i < (int)nb.size(); ++i)
            neighborSize.insert(std::make_pair((int)neighbors[nb[i]].size(), nb[i]));
    }

    std::vector<int> oout = out;
    for(i = 0; i < sz; ++i) //invert the permutation
        out[oout[i]] = i;

    return out;
}

LLTMatrix *SPDMatrix::factor() const
{
    int i, j, k;
    MyLLTMatrix *outP = new MyLLTMatrix();
    MyLLTMatrix &out = *outP;
    int sz = (int)m.size();
    out.m.resize(sz);
    out.diag.resize(sz);

    out.perm = computePerm();

    //permute matrix according to the permuation
    std::vector<std::vector<std::pair<int, double> > > pm(sz);
    for(i = 0; i < sz; ++i) {
        for(j = 0; j < (int)m[i].size(); ++j) {
            int ni = out.perm[i], nidx = out.perm[m[i][j].first];
            if(ni >= nidx)
                pm[ni].push_back(std::make_pair(nidx, m[i][j].second));
            else
                pm[nidx].push_back(std::make_pair(ni, m[i][j].second));
        }
    }
    for(i = 0; i < sz; ++i)
        sort(pm[i].begin(), pm[i].end());

    //prepare for decomposition
    std::vector<std::vector<std::pair<int, double> > > cols(sz);
    std::vector<double> dinv(sz); //inverses of out.diag

    std::vector<bool> added(sz, false);

    //Sparse cholesky decomposition
    for(i = 0; i < sz; ++i) { //current row
        std::vector<int> columnsAdded;
        columnsAdded.reserve(2 * pm[i].size());

        //compute columnsAdded (nonzero indices of factor in current row)
        int inCA = 0;
        for(j = 0; j < (int)pm[i].size() - 1; ++j) {
            added[pm[i][j].first] = true;
            columnsAdded.push_back(pm[i][j].first);
        }
        while(inCA < (int)columnsAdded.size()) {
            int idx = columnsAdded[inCA];
            ++inCA;
            for(k = 0; k < (int)cols[idx].size(); ++k) {
                int curCol = cols[idx][k].first;
                if(!added[curCol]) {
                    added[curCol] = true;
                    columnsAdded.push_back(curCol);
                }
            }
        }
        sort(columnsAdded.begin(), columnsAdded.end());

        for(j = 0; j < (int)columnsAdded.size(); ++j) {//add the columns and clear added
            added[columnsAdded[j]] = false;
            cols[columnsAdded[j]].push_back(std::make_pair(i, 0.));
        }

        for(j = 0; j < (int)pm[i].size() - 1; ++j) { //initialize it with m's entries
            int curCol = pm[i][j].first;
            cols[curCol].back().second = pm[i][j].second * dinv[curCol];
        }
        for(j = 0; j < (int)columnsAdded.size(); ++j) { //current column
            int idx = columnsAdded[j];
            int csz = (int)cols[idx].size() - 1;
            for(k = 0; k < csz; ++k) { //index in column above current row -- inner loop
                int tidx = cols[idx][k].first; //index into current row
                double prod = cols[idx][k].second * cols[idx].back().second * dinv[tidx];
                cols[tidx].back().second -= prod;
            }
        }
        //now diagonal
        out.diag[i] = pm[i].back().second;
        for(j = 0; j < (int)columnsAdded.size(); ++j) {
            double val = cols[columnsAdded[j]].back().second;
            out.diag[i] -= SQR(val);
            out.m[i].push_back(std::make_pair(columnsAdded[j], val)); //also add rows to output
        }
        if(out.diag[i] <= 0.) { //not positive definite
            assert(false && "Not positive definite matrix (or ill-conditioned)");
            delete outP;
            return new MyLLTMatrix();
        }
        out.diag[i] = sqrt(out.diag[i]);
        dinv[i] = 1. / out.diag[i];
    }

    out.initMt();

    /* Error check
    double totErr = 0;
    for(i = 0; i < m.size(); ++i) {
        for(j = 0; j < m[i].size(); ++j) {
            int q = m[i][j].first;
            double total = -m[i][j].second;
            out.m[i].push_back(make_pair(i, out.diag[i]));
            if(i != q) out.m[q].push_back(make_pair(q, out.diag[q]));
            for(k = 0; k < out.m[i].size(); ++k) {
                for(int z = 0; z < out.m[q].size(); ++z) {
                    if(out.m[i][k].first != out.m[q][z].first)
                        continue;
                    total += out.m[i][k].second * out.m[q][z].second;
                }
            }
            out.m[i].pop_back();
            if(i != q) out.m[q].pop_back();

            totErr += fabs(total);
        }
    }
    */

    return outP;
}

void MyLLTMatrix::initMt()  //compute the transposed entries (by rows)
{
    int i, j;

    mt.clear();
    mt.resize(m.size());

    for(i = 0; i < (int)m.size(); ++i) {
        for(j = 0; j < (int)m[i].size(); ++j) {
            mt[m[i][j].first].push_back(std::make_pair(i, m[i][j].second));
        }
    }
}

bool MyLLTMatrix::solve(std::vector<double> &b) const
{
    int i, j;

    if(b.size() != m.size())
        return false;

    std::vector<double> bp(b.size());
    //permute
    for(i = 0; i < (int)b.size(); ++i)
        bp[perm[i]] = b[i];

    //solve L (L^T x) = b for (L^T x)
    for(i = 0; i < (int)bp.size(); ++i) {
        for(j = 0; j < (int)m[i].size(); ++j)
            bp[i] -= bp[m[i][j].first] * m[i][j].second;
        bp[i] /= diag[i];
    }

    //solve L^T x = b for x
    for(i = (int)bp.size() - 1; i >= 0; --i) {
        for(j = 0; j < (int)mt[i].size(); ++j)
            bp[i] -= bp[mt[i][j].first] * mt[i][j].second;
        bp[i] /= diag[i];
    }

    //unpermute
    for(i = 0; i < (int)b.size(); ++i)
        b[i] = bp[perm[i]];

    return true;
}

//------------------------------------------------------------------------------
//!

struct BoneStruct {
   Vec3f _start;
   Vec3f _end;
};

//------------------------------------------------------------------------------
//!
bool canSee( SurfaceGeometry* surface, const Vec3f& p1, const Vec3f& p2 )
{
   Intersector::Hit hit;
   Rayf ray( p1, p2-p1 );
   if( Intersector::trace( surface, ray, hit ) && hit._t < 0.99f )
   {
      return false;
   }

   return true;
}

//------------------------------------------------------------------------------
//!
bool intersectTri( const Rayf& ray, uint id, float& t, void* data )
{
   MeshGeometry* geom = (MeshGeometry*)data;
   Intersector::Hit hit;
   hit._t = t;
   if( Intersector::trace(
      geom->position( geom->indices()[id*3] ),
      geom->position( geom->indices()[id*3+1] ),
      geom->position( geom->indices()[id*3+2] ),
      ray, hit
   ) )
   {
      t = hit._t;
      return true;
   }
   return false;
}

//------------------------------------------------------------------------------
//!
bool canSee( MeshGeometry* geom, BIH& bih, const Vec3f& p1, const Vec3f& p2 )
{
   Rayf ray( p1, p2-p1 );
   BIH::Hit hit;
   hit._t = 0.99f;
   return !bih.trace( ray, hit, &intersectTri, geom );
}

//------------------------------------------------------------------------------
//!
bool vectorInCone( const Vec3f& v, const Vec3f& n )
{
   return v.getNormalized().dot( n ) > 0.5f;
}



UNNAMESPACE_END

/*==============================================================================
   CLASS Pinocchio
==============================================================================*/

NAMESPACE_BEGIN

//------------------------------------------------------------------------------
//!
void
Pinocchio::recomputeVertexWeights( SurfaceGeometry* surface, Skeleton* skeleton )
{
   uint i, j;
   const float threshold = 0.04f;
   uint nv = surface->numVertices();
   uint numBones = skeleton->numBones();

   // Computation acceleration structure for ray tracing.
   surface->computeBIH();

   // Compute edges.
   Vector< Vector<uint> > edges( nv );
   for( i = 0; i < nv; ++i )
   {
      uint cur, start;
      cur = start = surface->starFirst(i);
      do
      {
         edges[i].pushBack( surface->dstVertexID( cur ) );
         cur = surface->starNextCCW( cur );
      } while( cur != start );
   }

   // Precompute start/end point of each bone in global space.
   Vector<BoneStruct> bonePts( numBones );
   for( i = 0; i < numBones; ++i )
   {
      Reff localToGlobalRef = skeleton->bone(i).globalToLocalReferential().getInversed();
      bonePts[i]._start     = localToGlobalRef.position();
      bonePts[i]._end       = localToGlobalRef.toMatrix() * skeleton->bone(i).endPoint();
   }

   // Prepare per vertex data.
   Vector< Vector<uint> > verticesByBone( numBones );
   Vector< Vector<float> > vdistByBone( numBones );
   Vector<float> boneDists( numBones );
   uint canSeeResults[2] = {0, 0};

   for( i = 0; i < nv; ++i )
   {
      Vec3f pos = surface->vertex(i);

      // Compute face normals.
      Vec3f avgNormal(0.0f);
      for( j = 0; j < edges[i].size(); ++j )
      {
         uint nj    = (j+1) % edges[i].size();
         Vec3f v1   = surface->vertex(edges[i][j])  - pos;
         Vec3f v2   = surface->vertex(edges[i][nj]) - pos;
         avgNormal += v1.cross(v2).normalize();
      }
      avgNormal.normalize();

      // Compute vertex to bone distances.
      float minDist = 1e10;
      for( j = 0; j < numBones; ++j )
      {
         boneDists[j] = CGM::sqrt( distsqToSeg( pos, bonePts[j]._start, bonePts[j]._end ) );
         minDist      = CGM::min( minDist, boneDists[j] );
      }

      // Compute nearest bone(s).
      for( j = 0; j < numBones; ++j )
      {
         // The reason we don't just pick the closest bone is so that if two are
         // equally close, both are factored in.
         if( boneDists[j] < minDist * 1.0001f )
         {
            Vec3f p  = projToSeg( pos, bonePts[j]._start, bonePts[j]._end );
            bool vis = canSee( surface, p, pos ) && vectorInCone( pos - p, avgNormal );

            if( vis )
            {
               verticesByBone[j].pushBack(i);
               vdistByBone[j].pushBack( boneDists[j] );
            }

            if( vis )  ++canSeeResults[1];
            else       ++canSeeResults[0];
         }
      }
   }

   printf( "CanSee: %d f, %d t\n", canSeeResults[0], canSeeResults[1] );

   // We have -Lw+Hw=HI, same as (H-L)w=HI, with (H-L)=DA (with D=diag(1./area))
   // so w = A^-1 (HI/D).
   vector<vector<pair<int, double> > > A(nv);
   vector<double> D(nv, 0.), H(nv, 0.);

   for( j = 0; j < numBones; ++j )
   {
      for( i = 0; i < verticesByBone[j].size(); ++i )
      {
         H[verticesByBone[j][i]] += 1. / SQR(1e-8 + vdistByBone[j][i]);
      }
   }

   for( i = 0; i < nv; ++i )
   {
      // Get areas
      for( j = 0; j < edges[i].size(); ++j )
      {
         uint nj  = (j+1) % edges[i].size();
         Vec3f v1 = surface->vertex(edges[i][j])  - surface->vertex(i);
         Vec3f v2 = surface->vertex(edges[i][nj]) - surface->vertex(i);
         D[i] += v1.cross(v2).length();
      }
      D[i] = 1.0 / (1e-10 + D[i]);

      // Get laplacian.
      double sum = 0.;
      for( j = 0; j < edges[i].size(); ++j )
      {
         uint nj  = (j+1) % edges[i].size();
         uint pj  = (uint)(j+edges[i].size()-1) % edges[i].size();

         Vec3f v1 = surface->vertex(i)           - surface->vertex(edges[i][pj]);
         Vec3f v2 = surface->vertex(edges[i][j]) - surface->vertex(edges[i][pj]);
         Vec3f v3 = surface->vertex(i)           - surface->vertex(edges[i][nj]);
         Vec3f v4 = surface->vertex(edges[i][j]) - surface->vertex(edges[i][nj]);

         double cot1 = (v1.dot(v2)) / (1e-6 + (v1.cross(v2)).length());
         double cot2 = (v3.dot(v4)) / (1e-6 + (v3.cross(v4)).length());
         sum += (cot1 + cot2);

         if( edges[i][j] <= i ) //check for triangular here because sum should be computed regardless
         {
            A[i].push_back( std::make_pair(edges[i][j], -cot1 - cot2) );
         }
      }

      A[i].push_back( std::make_pair(i, sum + H[i] / D[i]) );
      sort(A[i].begin(), A[i].end());
   }

   printf( "solving weights...\n" );

   SPDMatrix Am(A);
   LLTMatrix *Ainv = Am.factor();
   if( Ainv == NULL )
      return;

   // Compute weights for each bone.
   Vector<Vec4i> bonesID( nv, Vec4i(-1) );
   Vector<Vec4f> weights( nv, Vec4f::zero() );
   Vector<uchar> bonesQty( nv, 0 );

   for( j = 0; j < numBones; ++j )
   {
      vector<double> rhs(nv, 0.0);
      for( i = 0; i < verticesByBone[j].size(); ++i )
      {
         uint v = verticesByBone[j][i];
         rhs[v] = H[v] / D[v];
      }

      Ainv->solve(rhs);
      for( i = 0; i < nv; ++i )
      {
         // Clip just in case.
         if( rhs[i] > 1.0 ) rhs[i] = 1.0;
         if( rhs[i] > 1e-6 && rhs[i] > weights[i].x * threshold )
         {
            for( uint b = 0; b < 4 ; ++b )
            {
               if( weights[i](b) < rhs[i] )
               {
                  if( bonesID[i](b) != -1 )
                  {
                     switch( b )
                     {
                        case 0:
                        {
                           weights[i](3) = weights[i](2);
                           weights[i](2) = weights[i](1);
                           weights[i](1) = weights[i](0);
                           bonesID[i](3) = bonesID[i](2);
                           bonesID[i](2) = bonesID[i](1);
                           bonesID[i](1) = bonesID[i](0);
                        } break;
                        case 1:
                        {
                           weights[i](3) = weights[i](2);
                           weights[i](2) = weights[i](1);
                           bonesID[i](3) = bonesID[i](2);
                           bonesID[i](2) = bonesID[i](1);
                        } break;
                        case 2:
                        {
                           weights[i](3) = weights[i](2);
                           bonesID[i](3) = bonesID[i](2);
                        } break;
                     }
                  }
                  weights[i](b) = (float)rhs[i];
                  bonesID[i](b) = j;
                  break;
               }
            }
         }
      }
   }

   Vector<uint> nw(5,0);

   // Compute normalized weights.
   for( i = 0; i < nv; ++i )
   {
      uint b;
      for( b = 1; b < 4; ++b )
      {
         if( weights[i](b) < weights[i].x * threshold )
         {
            switch( b )
            {
               case 1:
               {
                  weights[i](1) = 0.0f;
                  weights[i](2) = 0.0f;
                  weights[i](3) = 0.0f;
                  bonesID[i](1) = -1;
                  bonesID[i](2) = -1;
                  bonesID[i](3) = -1;
               } break;
               case 2:
               {
                  weights[i](2) = 0.0f;
                  weights[i](3) = 0.0f;
                  bonesID[i](2) = -1;
                  bonesID[i](3) = -1;
               } break;
               case 3:
               {
                  weights[i](3) = 0.0f;
                  bonesID[i](3) = -1;
               } break;
            }
            break;
         }
      }

      bonesQty[i] = b;
      float sum = 0.0f;
      for( j = 0; j < b; ++j )
      {
         sum += weights[i](j);
      }
      for( j = 0; j < b; ++j )
      {
         weights[i](j) /= sum;
      }
      nw[b]++;
   }

   // Add the weights to the Surface.
   surface->reserveWeights(nv);
   for( i = 0; i < nv; ++i )
   {
      surface->addWeights( bonesQty[i], bonesID[i], weights[i] );
   }

   delete Ainv;

   // STATS.
   for( i = 0; i < nw.size(); ++i )
   {
      if( nw[i] != 0 )
      {
         printf("%d/%d vertices with %d weights\n", nw[i], nv, i);
      }
   }
   j = 0;
   for( i = 0; i < 4; ++i )
   {
      printf( "w: %d %f\n", bonesID[j](i), weights[j](i) );
   }
}

//------------------------------------------------------------------------------
//!
void
Pinocchio::recomputeVertexWeights(
   MeshGeometry* geom,
   Skeleton*     skeleton,
   int           weightsAttrID,
   int           bonesAttrID
)
{
   // Triangle mesh?
   if( geom->primitiveSize() != 3 ) return;

   // 1. Compute indirection mapping.
   const float error  = 1.0f/1024.0f;
   const float error2 = error*error;
   uint numVD = geom->numVertices();

   Vector<uint> indirect(numVD);
   Vector<uint> ids;
   Grid<uint> grid( numVD*2, 1.0f/128.0f);

   StdErr << "# vd: " << numVD << nl;
   for( uint i = 0; i < numVD; ++i )
   {
      indirect[i] = i;
      // Search if vertex already exist.
      Vec3f p  =  geom->position(i);
      Vec3i c0 = grid.cellCoord( p-error );
      Vec3i c1 = grid.cellCoord( p+error );

      for( int x = c0.x; x <= c1.x; ++x )
      {
         for( int y = c0.y; y <= c1.y; ++y )
         {
            for( int z = c0.z; z <= c1.z; ++z )
            {
               Grid<uint>::Link* l = grid.cell( Vec3i(x,y,z) );
               for( ; l; l = l->_next )
               {
                  // Have we found the same vertex?
                  if( sqrLength( geom->position( l->_obj )-p ) < error2 )
                  {
                     indirect[i] = indirect[l->_obj];
                  }
               }
            }
         }
      }
      // Not found?
      if( indirect[i] == i )
      {
         grid.add( grid.cellID( p ), i );
         indirect[i] = (uint)ids.size();
         ids.pushBack( i );
      }
   }

   StdErr << "# of v: " << ids.size() << nl;

   // 2. Compute neighbors.
   uint nv = (uint)ids.size();
   Vector< Vector< Pair<uint,uint> > > neighbors( nv );
   uint numT = geom->numPrimitives();

   for( uint i = 0; i < numT*3; i+=3 )
   {
      uint i0 = indirect[geom->indices()[i]];
      uint i1 = indirect[geom->indices()[i+1]];
      uint i2 = indirect[geom->indices()[i+2]];

      neighbors[i0].pushBack( Pair<uint,uint>(i1,i2) );
      neighbors[i1].pushBack( Pair<uint,uint>(i2,i0) );
      neighbors[i2].pushBack( Pair<uint,uint>(i0,i1) );
   }
   // Sorts neighbors.
   for( uint i = 0; i < nv; ++i )
   {
      uint nn = (uint)neighbors[i].size();
      for( uint j = 1; j < nn; ++j )
      {
         for( uint k = j+1; k < nn; ++k )
         {
            if( neighbors[i][k].first == neighbors[i][j-1].second )
            {
               CGM::swap( neighbors[i][j], neighbors[i][k] );
               break;
            }
         }
      }
   }

   // 3. Compute acceleration structure for ray tracing.
   BIH bih;
   Vector<AABBoxf> bboxes( numT, AABBoxf::empty() );
   Vector<AABBoxf*> bboxesPtr( numT );
   Vector<Vec3f> center( numT );

   const uint32_t* id = geom->indices();
   for( uint i = 0; i < numT; ++i )
   {
      Vec3f v0 = geom->position( *(id++) );
      Vec3f v1 = geom->position( *(id++) );
      Vec3f v2 = geom->position( *(id++) );

      bboxes[i] |= v0;
      bboxes[i] |= v1;
      bboxes[i] |= v2;
      bboxes[i].grow( 1.0f/1024.0f );

      bboxesPtr[i] = &(bboxes[i]);
      center[i]    = ( v0 + v1 + v2 ) * (1.0f/3.0f);
   }
   bih.create( bboxesPtr, center, NULL, 2 );

   // 4. Prepare vertex data.

   // Precompute start/end point of each bone in global space.
   uint numBones = skeleton->numBones();
   Vector<BoneStruct> bonePts( numBones );
   for( uint i = 0; i < numBones; ++i )
   {
      Reff localToGlobalRef = skeleton->bone(i).globalToLocalReferential().getInversed();
      bonePts[i]._start     = localToGlobalRef.position();
      bonePts[i]._end       = localToGlobalRef.toMatrix() * skeleton->bone(i).endPoint();
   }

   // Prepare per vertex data.
   Vector< Vector<uint> > verticesByBone( numBones );
   Vector< Vector<float> > vdistByBone( numBones );
   Vector<float> boneDists( numBones );
   uint canSeeResults[2] = {0, 0};

   // TODO: Bone glow and others.
#define BONETECH 1
#if BONETECH == 1
   // Default technique.
   for( uint i = 0; i < ids.size(); ++i )
   {
      Vec3f pos = geom->position(ids[i]);
      Vec3f normal = geom->normal( ids[i] );

      // Compute vertex to bone distances.
      float minDist = 1e10;
      for( uint j = 0; j < numBones; ++j )
      {
         boneDists[j] = CGM::sqrt( distsqToSeg( pos, bonePts[j]._start, bonePts[j]._end ) );
         minDist      = CGM::min( minDist, boneDists[j] );
      }

      // Compute nearest bone(s).
      for( uint j = 0; j < numBones; ++j )
      {
         // The reason we don't just pick the closest bone is so that if two are
         // equally close, both are factored in.
         if( boneDists[j] < minDist * 1.0001f )
         {
            Vec3f p  = projToSeg( pos, bonePts[j]._start, bonePts[j]._end );
            bool vis = canSee( geom, bih, p, pos ) && vectorInCone( pos - p, normal );

            if( vis )
            {
               verticesByBone[j].pushBack(i);
               vdistByBone[j].pushBack( boneDists[j] );
            }

            if( vis )  ++canSeeResults[1];
            else       ++canSeeResults[0];
         }
      }
   }
#endif
#if BONETECH == 2
   // All bones and not only the nearest.
   for( uint i = 0; i < ids.size(); ++i )
   {
      Vec3f pos    = geom->position(ids[i]);
      Vec3f normal = geom->normal( ids[i] );

      // Compute nearest bone(s).
      for( uint j = 0; j < numBones; ++j )
      {
         Vec3f p  = projToSeg( pos, bonePts[j]._start, bonePts[j]._end );
         bool vis = vectorInCone( pos - p, normal ) && canSee( geom, bih, p, pos );

         if( vis )
         {
            verticesByBone[j].pushBack(i);
            vdistByBone[j].pushBack( length(pos-p) );
         }

         if( vis )  ++canSeeResults[1];
         else       ++canSeeResults[0];
      }
   }
#endif
#if BONETECH == 3
   // Nearest visible.
   for( uint i = 0; i < ids.size(); ++i )
   {
      Vec3f pos    = geom->position(ids[i]);
      Vec3f normal = geom->normal( ids[i] );

      // Compute nearest bone(s).
      float minDist = 1e10;
      uint minBone  = 1000;
      for( uint j = 0; j < numBones; ++j )
      {
         Vec3f p    = projToSeg( pos, bonePts[j]._start, bonePts[j]._end );
         float dist = length(pos-p);
         if( dist < minDist )
         {
            bool vis = vectorInCone( pos - p, normal ) && canSee( geom, bih, p, pos );
            if( vis )
            {
               minDist = dist;
               minBone = j;
            }
            if( vis )  ++canSeeResults[1];
            else       ++canSeeResults[0];
         }
      }
      if( minBone < 1000 )
      {
         verticesByBone[minBone].pushBack(i);
         vdistByBone[minBone].pushBack( minDist );
      }
   }
#endif
   printf( "CanSee: %d f, %d t\n", canSeeResults[0], canSeeResults[1] );

   // 5. Preparing matrices.
   // We have -Lw+Hw=HI, same as (H-L)w=HI, with (H-L)=DA (with D=diag(1./area))
   // so w = A^-1 (HI/D).
   vector<vector<pair<int, double> > > A(nv);
   vector<double> D(nv, 0.), H(nv, 0.);

   for( uint j = 0; j < numBones; ++j )
   {
      for( uint i = 0; i < verticesByBone[j].size(); ++i )
      {
         H[verticesByBone[j][i]] += 1. / SQR(1e-8 + vdistByBone[j][i]);
      }
   }

   for( uint i = 0; i < nv; ++i )
   {
      Vec3f pos = geom->position(ids[i]);
      uint nn   = (int)neighbors[i].size();
      // Get areas
      for( uint j = 0; j < nn; ++j )
      {
         Vec3f v1 = geom->position(ids[neighbors[i][j].first])  - pos;
         Vec3f v2 = geom->position(ids[neighbors[i][j].second]) - pos;
         D[i] += v1.cross(v2).length();
      }
      D[i] = 1.0 / (1e-10 + D[i]);

      // Get laplacian.
      double sum = 0.;
      for( uint j = 0; j < nn; ++j )
      {
         uint pj  = (j+nn-1) % nn;
         Vec3f posj  = geom->position(ids[neighbors[i][j].first]);
         Vec3f posnj = geom->position(ids[neighbors[i][j].second]);
         Vec3f pospj = geom->position(ids[neighbors[i][pj].first]);
         Vec3f v1 = pos  - pospj;
         Vec3f v2 = posj - pospj;
         Vec3f v3 = pos  - posnj;
         Vec3f v4 = posj - posnj;

         double cot1 = (v1.dot(v2)) / (1e-6 + (v1.cross(v2)).length());
         double cot2 = (v3.dot(v4)) / (1e-6 + (v3.cross(v4)).length());
         sum += (cot1 + cot2);

         if( neighbors[i][j].first <= i ) //check for triangular here because sum should be computed regardless
         {
            A[i].push_back( std::make_pair(neighbors[i][j].first, -cot1 - cot2) );
         }
      }

      A[i].push_back( std::make_pair(i, sum + H[i] / D[i]) );
      sort(A[i].begin(), A[i].end());
   }


   // 6. Solving weights.
   const float threshold = 0.05f;

   StdErr<< "solving weights...\n";

   SPDMatrix Am(A);
   LLTMatrix *Ainv = Am.factor();
   if( Ainv == NULL ) return;
   // Compute weights for each bone.
   Vector<Vec4i> bonesID( nv, Vec4i(-1) );
   Vector<Vec4f> weights( nv, Vec4f::zero() );

   for( uint j = 0; j < numBones; ++j )
   {
      vector<double> rhs(nv, 0.0);
      for( uint i = 0; i < verticesByBone[j].size(); ++i )
      {
         uint v = verticesByBone[j][i];
         rhs[v] = H[v] / D[v];
      }

      Ainv->solve(rhs);
      for( uint i = 0; i < nv; ++i )
      {
         // Clip just in case.
         if( rhs[i] > 1.0 ) rhs[i] = 1.0;
         if( rhs[i] > 1e-6 && rhs[i] > weights[i].x * threshold )
         {
            for( uint b = 0; b < 4 ; ++b )
            {
               if( weights[i](b) < rhs[i] )
               {
                  if( bonesID[i](b) != -1 )
                  {
                     switch( b )
                     {
                        case 0:
                        {
                           weights[i](3) = weights[i](2);
                           weights[i](2) = weights[i](1);
                           weights[i](1) = weights[i](0);
                           bonesID[i](3) = bonesID[i](2);
                           bonesID[i](2) = bonesID[i](1);
                           bonesID[i](1) = bonesID[i](0);
                        } break;
                        case 1:
                        {
                           weights[i](3) = weights[i](2);
                           weights[i](2) = weights[i](1);
                           bonesID[i](3) = bonesID[i](2);
                           bonesID[i](2) = bonesID[i](1);
                        } break;
                        case 2:
                        {
                           weights[i](3) = weights[i](2);
                           bonesID[i](3) = bonesID[i](2);
                        } break;
                     }
                  }
                  weights[i](b) = (float)rhs[i];
                  bonesID[i](b) = j;
                  break;
               }
            }
         }
      }
   }

   // 7. Compute normalized weights.
   Vector<uint> nw(5,0);
   for( uint i = 0; i < nv; ++i )
   {
      // Eliminate to low weights.
      uint b;
      for( b = 1; b < 4; ++b )
      {
         if( weights[i](b) < weights[i].x * threshold )
         {
            switch( b )
            {
               case 1:
               {
                  weights[i](1) = 0.0f;
                  weights[i](2) = 0.0f;
                  weights[i](3) = 0.0f;
                  bonesID[i](1) = 0;
                  bonesID[i](2) = 0;
                  bonesID[i](3) = 0;
               } break;
               case 2:
               {
                  weights[i](2) = 0.0f;
                  weights[i](3) = 0.0f;
                  bonesID[i](2) = 0;
                  bonesID[i](3) = 0;
               } break;
               case 3:
               {
                  weights[i](3) = 0.0f;
                  bonesID[i](3) = 0;
               } break;
            }
            break;
         }
      }

      // Normalized weights.
      float sum   = 0.0f;
      for( uint j = 0; j < b; ++j ) sum += weights[i](j);
      for( uint j = 0; j < b; ++j ) weights[i](j) /= sum;
      nw[b]++;
   }

   delete Ainv;

   // STATS.
   for( uint i = 0; i < nw.size(); ++i )
   {
      if( nw[i] != 0 )
      {
         printf("%d/%d vertices with %d weights\n", nw[i], nv, i);
      }
   }
   uint j = 0;
   for( uint i = 0; i < 4; ++i )
   {
      printf( "w: %d %f\n", bonesID[j](i), weights[j](i) );
   }

   // Transfer the bones data to the mesh geometry.
   float* bonesPtr    = geom->vertices() + geom->attributeOffset( bonesAttrID );
   float* weightsPtr  = geom->vertices() + geom->attributeOffset( weightsAttrID );
   size_t stride      = geom->vertexStride();
   for( uint i = 0; i < numVD; ++i, bonesPtr += stride, weightsPtr += stride )
   {
      uint id = indirect[i];
      bonesPtr[0]   = float(bonesID[id].x);
      bonesPtr[1]   = float(bonesID[id].y);
      bonesPtr[2]   = float(bonesID[id].z);
      bonesPtr[3]   = float(bonesID[id].w);
      weightsPtr[0] = weights[id].x;
      weightsPtr[1] = weights[id].y;
      weightsPtr[2] = weights[id].z;
      weightsPtr[3] = weights[id].w;
   }
}



NAMESPACE_END
