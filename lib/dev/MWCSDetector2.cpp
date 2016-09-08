#pragma ident "$Id$"

/**
 * @file MWCSDetector2.cpp
 * This is a class to detect cycle slips using the Melbourne-Wubbena
 * combination.
 */

//============================================================================
//
//  This file is part of GPSTk, the GPS Toolkit.
//
//  The GPSTk is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Lesser General Public License as published
//  by the Free Software Foundation; either version 2.1 of the License, or
//  any later version.
//
//  The GPSTk is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with GPSTk; if not, write to the Free Software Foundation,
//  Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
//
//  HuiWei - Wuhan University, 2016
//
//  Revisions
//
//  2016/04/13
//
//  Test this programs,  shjzhang
//
//============================================================================


#include "MWCSDetector2.hpp"



namespace gpstk
{
      
      using namespace std;
      
      // Returns a string identifying this object.
      std::string MWCSDetector2::getClassName() const
      { return "MWCSDetector2"; }
      
      
      
      /* Common constructor
       *
       * @param mLambdas      Maximum deviation allowed before declaring
       *                      cycle slip (in number of Melbourne-Wubbena
       *                      wavelenghts).
       * @param dtMax         Maximum interval of time allowed between two
       *                      successive epochs, in seconds.
       */
      MWCSDetector2::MWCSDetector2( const double& mLambdas,
                                 const double& dtMax,
                                 const bool& use )
      : obsType(TypeID::MWubbena), lliType1(TypeID::LLI1),
      lliType2(TypeID::LLI2), resultType1(TypeID::CSL1),
      resultType2(TypeID::CSL2), useLLI(use)
      {
            setDeltaTMax(dtMax);
            setMaxNumLambdas(mLambdas);
      }
      
      
      
      /* Returns a satTypeValueMap object, adding the new data generated
       * when calling this object.
       *
       * @param epoch     Time of observations.
       * @param gData     Data object holding the data.
       * @param epochflag Epoch flag.
       */
      satTypeValueMap& MWCSDetector2::Process( const CommonTime& epoch,
                                             satTypeValueMap& gData,
                                             const short& epochflag )
      throw(ProcessingException)
      {
            
            try
            {
                  
                  double value1(0.0);
                  double lli1(0.0);
                  double lli2(0.0);
                  
                  SatIDSet satRejectedSet;
                  
                  // Loop through all the satellites
                  satTypeValueMap::iterator it;
                  for (it = gData.begin(); it != gData.end(); ++it)
                  {
                        
                        try
                        {
                              // Try to extract the values
                              value1 = (*it).second(obsType);
                        }
                        catch(...)
                        {
                              // If some value is missing, then schedule this satellite
                              // for removal
                              satRejectedSet.insert( (*it).first );
                              continue;
                        }
                        
                        if (useLLI)
                        {
                              try
                              {
                                    // Try to get the LLI1 index
                                    lli1  = (*it).second(lliType1);
                              }
                              catch(...)
                              {
                                    // If LLI #1 is not found, set it to zero
                                    // You REALLY want to have BOTH LLI indexes properly set
                                    lli1 = 0.0;
                              }
                              
                              try
                              {
                                    // Try to get the LLI2 index
                                    lli2  = (*it).second(lliType2);
                              }
                              catch(...)
                              {
                                    // If LLI #2 is not found, set it to zero
                                    // You REALLY want to have BOTH LLI indexes properly set
                                    lli2 = 0.0;
                              }
                        }
                        
                        // If everything is OK, then get the new values inside the
                        // structure. This way of computing it allows concatenation of
                        // several different cycle slip detectors
                        (*it).second[resultType1] += getDetection( epoch,
                                                                  (*it).first,
                                                                  (*it).second,
                                                                  epochflag,
                                                                  value1,
                                                                  lli1,
                                                                  lli2 );
                        if ( (*it).second[resultType1] > 1.0 )
                        {
                              (*it).second[resultType1] = 1.0;
                        }
                        
                        // We will mark both cycle slip flags
                        (*it).second[resultType2] = (*it).second[resultType1];
                        
                  }
                  
                  // Remove satellites with missing data
                  gData.removeSatID(satRejectedSet);
                  
                  return gData;
                  
            }
            catch(Exception& u)
            {
                  // Throw an exception if something unexpected happens
                  ProcessingException e( getClassName() + ":"
                                        + u.what() );
                  
                  GPSTK_THROW(e);
                  
            }
            
      }  // End of method 'MWCSDetector2::Process()'
      
      
      
      /* Method to set the maximum interval of time allowed between two
       * successive epochs.
       *
       * @param maxDelta      Maximum interval of time, in seconds
       */
      MWCSDetector2& MWCSDetector2::setDeltaTMax(const double& maxDelta)
      {
            
            // Don't allow delta times less than or equal to 0
            if (maxDelta > 0.0)
            {
                  deltaTMax = maxDelta;
            }
            else
            {
                  deltaTMax = 61.0;
            }
            
            return (*this);
            
      }  // End of method 'MWCSDetector2::setDeltaTMax()'
      
      
      
      /* Method to set the maximum deviation allowed before declaring
       * cycle slip (in number of Melbourne-Wubbena wavelenghts).
       *
       * @param mLambdas     Maximum deviation allowed before declaring
       *                     cycle slip (in number of Melbourne-Wubbena
       *                     wavelenghts).
       */
      MWCSDetector2& MWCSDetector2::setMaxNumLambdas(const double& mLambdas)
      {
            
            // Don't allow number of lambdas less than or equal to 0
            if (mLambdas > 0.0)
            {
                  maxNumLambdas = mLambdas;
            }
            else
            {
                  maxNumLambdas = 10.0;
            }
            
            return (*this);
            
      }  // End of method 'MWCSDetector2::setMaxNumLambdas()'
      
      
      
      /* Returns a gnnsRinex object, adding the new data generated when
       * calling this object.
       *
       * @param gData    Data object holding the data.
       */
      gnssRinex& MWCSDetector2::Process(gnssRinex& gData)
      throw(ProcessingException)
      {
            
            try
            {
                  
                  Process(gData.header.epoch, gData.body, gData.header.epochFlag);
                  
                  return gData;
                  
            }
            catch(Exception& u)
            {
                  // Throw an exception if something unexpected happens
                  ProcessingException e( getClassName() + ":"
                                        + u.what() );
                  
                  GPSTK_THROW(e);
                  
            }
            
      }  // End of method 'MWCSDetector2::Process()'
      
      
      
      /* Method that implements the Melbourne-Wubbena cycle slip
       *  detection algorithm
       *
       * @param epoch     Time of observations.
       * @param sat       SatID.
       * @param tvMap     Data structure of TypeID and values.
       * @param epochflag Epoch flag.
       * @param mw        Current MW observation value.
       * @param lli1      LLI1 index.
       * @param lli2      LLI2 index.
       */
      double MWCSDetector2::getDetection( const CommonTime& epoch,
                                        const SatID& sat,
                                        typeValueMap& tvMap,
                                        const short& epochflag,
                                        const double& mw,
                                        const double& lli1,
                                        const double& lli2 )
      {
            
            bool reportCS(false);
            
            // Difference between current and former epochs, in sec
            double currentDeltaT(0.0);
            
            // Difference between current and former MW values
            double currentBias(0.0);
            
            // Limit to declare cycle slip based on lambdas (LambdaLW = 0.862 m)
            double lambdaLimit(maxNumLambdas*0.862);
            
            double tempLLI1(0.0);
            double tempLLI2(0.0);
            
            
            // Get the difference between current epoch and former epoch,
            // in seconds
            currentDeltaT = ( epoch - MWData[sat].formerEpoch );
           
            // Store current epoch as former epoch
            MWData[sat].formerEpoch = epoch;
            
            // Difference between current value of MW and average value
            currentBias = std::abs(mw - MWData[sat].meanMW);

            // Increment window size
            ++MWData[sat].windowSize;
            
            
            
            
            
            // Check if receiver already declared cycle slip or if too much time
            // has elapsed
            // Note: If tvMap(lliType1) or tvMap(lliType2) don't exist, then 0
            // will be used and those tests will pass
            if ( (tvMap(lliType1)==1.0) ||
                (tvMap(lliType1)==3.0) ||
                (tvMap(lliType1)==5.0) ||
                (tvMap(lliType1)==7.0) )
            {
                  tempLLI1 = 1.0;
            }
            
            if ( (tvMap(lliType2)==1.0) ||
                (tvMap(lliType2)==3.0) ||
                (tvMap(lliType2)==5.0) ||
                (tvMap(lliType2)==7.0) )
            {
                  tempLLI2 = 1.0;
            }
            
            //    if ( (epochflag==1)  ||
            //         (epochflag==6)  ||
            //         (tempLLI1==1.0) ||
            //         (tempLLI2==1.0) ||
            //         (currentDeltaT > deltaTMax) )
            //
            /**
             *  here we don't consider the epoch flag from RINEX,
             *  which means we don't treat the epoch flag as true.
             */
            if ( (tempLLI1==1.0) ||
                (tempLLI2==1.0) ||
                (currentDeltaT > deltaTMax) )
            {
                  
                  // We reset the filter with this
                  MWData[sat].windowSize = 1;
                  
                  reportCS = true;                // Report cycle slip
            }
            
            if (MWData[sat].windowSize > 1)
            {
                  
                  // Test for current bias bigger than lambda limit and for
                  // current bias squared bigger than sigma squared limit
                  // currentBias<4*sqrt(MWData[sat].varMW);
                  
                  lambdaLimit=4*std::sqrt(MWData[sat].varMW);

                  if ( currentBias > lambdaLimit )
                  {
                        
                        // We reset the filter with this
                        MWData[sat].windowSize = 1;
                        
                        reportCS = true;                // Report cycle slip
                        
                  }
            }

            
            // Let's prepare for the next time
            // If a cycle-slip happened or just starting up
            if (MWData[sat].windowSize < 2)
            {
                  MWData[sat].meanMW = mw;
                  //give varMW=1.0 as init
                  MWData[sat].varMW = 0.25*0.25;
            }
            else
            {
                  // MW bias from the mean value
                  double mwBias(mw - MWData[sat].meanMW);
                  double size( static_cast<double>(MWData[sat].windowSize) );
                  
                  // Compute average
                  MWData[sat].meanMW += mwBias / size;
                  
                  // Compute variance 
                  // Var(i) = Var(i-1) + [ ( mw(i) - meanMW)^2/(i)- 1*Var(i-1) ]/(i);
                  //acoording to Equations3.2,zhang ,2009. change 1/i to 1/(i^2)*( mw(i) - meanMW)^2
                  // not used yet
                  MWData[sat].varMW  += ( mwBias*mwBias -MWData[sat].varMW ) / size;
                  
            }
            
            
            if (reportCS)
            {
                  return 1.0;
            }
            else
            {
                  return 0.0;
            }
            
      }  // End of method 'MWCSDetector2::getDetection()'
      
      
}  // End of namespace gpstk