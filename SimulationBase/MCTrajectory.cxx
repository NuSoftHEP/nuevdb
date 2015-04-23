////////////////////////////////////////////////////////////////////////
/// \file  MCTrajectory.cxx
/// \brief Container of trajectory information for a particle
///
/// \author  seligman@nevis.columbia.edu
////////////////////////////////////////////////////////////////////////

#include "cetlib/exception.h"

#include "SimulationBase/MCTrajectory.h"

#include <TLorentzVector.h>

#include <cmath>
#include <deque>
#include <iterator>
#include <vector>

namespace simb {

  // Nothing special need be done for the default constructor or destructor.
  MCTrajectory::MCTrajectory() 
    : ftrajectory()
  {}

  //----------------------------------------------------------------------------
  MCTrajectory::MCTrajectory( const TLorentzVector& position, 
			      const TLorentzVector& momentum )
  {
    ftrajectory.push_back( value_type( position, momentum ) );
  }

  //----------------------------------------------------------------------------
  const TLorentzVector& MCTrajectory::Position( const size_type index ) const
  {
    const_iterator i = ftrajectory.begin();
    std::advance(i,index);
    return (*i).first;
  }

  //----------------------------------------------------------------------------
  const TLorentzVector& MCTrajectory::Momentum( const size_type index ) const
  {
    const_iterator i = ftrajectory.begin();
    std::advance(i,index);
    return (*i).second;
  }

  //----------------------------------------------------------------------------
  double MCTrajectory::TotalLength() const
  {
    const int N = size();
    if(N < 2) return 0;

    // We take the sum of the straight lines between the trajectory points
    double dist = 0;
    for(int n = 0; n < N-1; ++n){
      dist += (Position(n+1)-Position(n)).Vect().Mag();
    }

    return dist;
  }

  //----------------------------------------------------------------------------
  std::ostream& operator<< ( std::ostream& output, const MCTrajectory& list )
  {
    // Determine a field width for the voxel number.
    MCTrajectory::size_type numberOfTrajectories = list.size();
    int numberOfDigits = (int) std::log10( (double) numberOfTrajectories ) + 1;

    // A simple header.
    output.width( numberOfDigits );
    output << "#" << ": < position (x,y,z,t), momentum (Px,Py,Pz,E) >" << std::endl; 

    // Write each trajectory point on a separate line.
    MCTrajectory::size_type nTrajectory = 0;
    for ( MCTrajectory::const_iterator trajectory = list.begin(); trajectory != list.end(); ++trajectory, ++nTrajectory )
      {
	output.width( numberOfDigits );
	output << nTrajectory << ": " 
	       << "< (" << (*trajectory).first.X() 
	       << "," << (*trajectory).first.Y() 
	       << "," << (*trajectory).first.Z() 
	       << "," << (*trajectory).first.T() 
	       << ") , (" << (*trajectory).second.Px() 
	       << "," << (*trajectory).second.Py() 
	       << "," << (*trajectory).second.Pz() 
	       << "," << (*trajectory).second.E() 
	       << ") >" << std::endl;
      }

    return output;
  }

  //----------------------------------------------------------------------------
  void MCTrajectory::Sparsify(double margin)
  {
    // This is a divide-and-conquer algorithm. If the straight line between two
    // points is close enough to all the intermediate points, then just keep
    // the endpoints. Otherwise, divide the range in two and try again.

    // We keep the ranges that need checking in "toCheck". If a range is good
    // as-is, we put just the starting point in "done". The end-point will be
    // taken care of by the next range.

    // Need at least three points to think of removing one
    if(size() <= 2) return;

    // Deal in terms of distance-squared to save some sqrts
    margin *= margin;

    // Deque because we add things still to check on the end, and pop things
    // we've checked from the front.
    std::deque<std::pair<int, int> > toCheck;
    // Start off by trying to replace the whole trajectory with just the
    // endpoints.
    toCheck.push_back(std::make_pair(0, size()-1));

    std::vector<int> done;

    while(!toCheck.empty()){
      const int loIdx = toCheck.front().first;
      const int hiIdx = toCheck.front().second;
      toCheck.pop_front();

      // Should never have been given a degenerate range
      if(hiIdx < loIdx+2)
	throw cet::exception("MCTrajectory") << "Degnerate range in Sparsify method";

      const TVector3 loVec = at(loIdx).first.Vect();
      const TVector3 hiVec = at(hiIdx).first.Vect();

      const TVector3 dir = (hiVec-loVec).Unit();

      // Are all the points in between close enough?
      bool ok = true;
      for(int i = loIdx+1; i < hiIdx; ++i){
	const TVector3 toHere = at(i).first.Vect()-loVec;
	// Perpendicular distance^2 from the line joining loVec to hiVec
	const double impact = (toHere-dir.Dot(toHere)*dir).Mag2();
	if(impact > margin){ok = false; break;}
      }

      if(ok){
        // These points adequately represent this range
        done.push_back(loIdx);
      }
      else{
        // Split in half
        const int midIdx = (loIdx+hiIdx)/2;
        // Should never have a range this small
        if(midIdx == loIdx)
	  throw cet::exception("MCTrajectory") << "Midpoint in sparsification is same as lowpoint";
        if(midIdx == hiIdx)
	  throw cet::exception("MCTrajectory") << "Midpoint in sparsification is same as hipoint";

        // The range can be small enough that upon splitting, the new ranges
        // are degenerate, and should just be written out straight away. Check
        // for those cases.

        if(midIdx == loIdx+1){
          done.push_back(loIdx);
        }
        else{
          toCheck.push_back(std::make_pair(loIdx, midIdx));
        }

        if(midIdx == hiIdx-1){
          done.push_back(midIdx);
        }
        else{
          toCheck.push_back(std::make_pair(midIdx, hiIdx));
        }
      }
    } // end while

    // We end up with them in a somewhat-randomized order
    std::sort(done.begin(), done.end());

    // Look up the trajectory points at the stored indices, write them to a new
    // trajectory
    const unsigned int I = done.size();
    list_type newTraj;
    newTraj.reserve(I+1);
    for(unsigned int i = 0; i < I; ++i) newTraj.push_back(at(done[i]));
    // Remember to add the very last point in
    newTraj.push_back(*rbegin());

    // Replace trajectory with new version
    ftrajectory = newTraj;
  }

} // namespace sim
