#pragma ident "$Id: ReceiverAttData.cpp 2897 2012-08-10 20:08:46Z shjzhang $"
//only for GOCE ATTITUDE
/**
 * @file ReceiverAttData.cpp
 * Encapsulate satellite bias file data, including I/O
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
//  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//  
//  Shoujian Zhang, Wuhan University, 2012
//
//============================================================================

#include <iostream>
#include <fstream>
#include "StringUtils.hpp"
#include "LEOReciverAttData.hpp"

using namespace std;
namespace gpstk
{
      //read GOCE attitude file
        void LEOReciverAtt::ReadLEOatt( string filename,
                                        vector<LEOatt> &vLEOatt)
      {
            
            LEOatt goceattx;
            
            vLEOatt.clear();
            
            ifstream inpf(filename.c_str());
            if(!inpf)
            {
                  FileMissingException fme("Could not open file :" + filename);
                  GPSTK_THROW(fme);
            }
            bool ok(true);
            string line;
            
            while( !inpf.eof() && inpf.good() )
            {
                  if( inpf.eof() ) break;
                  
                  if( inpf.bad() ) { ok = false; break; }
                  
                  getline(inpf,line);
                  istringstream is(line);
                  is>>goceattx.second
                  >>goceattx.q1
                  >>goceattx.q2
                  >>goceattx.q3
                  >>goceattx.q4;

                  vLEOatt.push_back(goceattx);
                  
            }
            
            cout<<"The length of file "<<filename<<" is "
            <<vLEOatt.size()<<endl;
            
            inpf.close();
            
            if( !ok )
            {
                  FileMissingException fme( filename + " is corrupted or in wrong format");
                  GPSTK_THROW(fme);
            }

      } //end of read GOCEattitude file
      
      
      //read GOCE attitude file
        void LEOReciverAtt::ReadLEOatt2(CommonTime time1,
                                        CommonTime time2,
                                        string filename,
                                        vector<LEOatt> &vLEOattnew)
      {
            
            
            vector<LEOatt> vLEOatt;
            vLEOattnew.clear();
            //read file
            ReadLEOatt(filename,vLEOatt);
            
            int length=vLEOatt.size();
            
            // time to cut
            double t1,t2;
            
            time1.get(wday,wsod,wfsod);
            t1=double(wday-AttJDaystart-0.5)*86400+wsod+wfsod+Atttimestart;
            time2.get(wday,wsod,wfsod);
            t2=double(wday-AttJDaystart-0.5)*86400+wsod+wfsod+Atttimestart;
            
            for (int i=0; i<length-1;i++)
            {
                  
                  if((vLEOatt[i].second > (t1-100))
                     &&(vLEOatt[i].second < (t2+100)))
                  vLEOattnew.push_back(vLEOatt[i]);
                  
            }
            
            
      } //end of read GOCEattitude file
      
      
      
      
      
      
      //get GOCE attitude value at time ttag   vGOCEattag
        void LEOReciverAtt::GetLEOattime(CommonTime time,
                                         vector<LEOatt> vLEOatt,
                                         LEOatt &LEOatttag)
      
      {
            // time to inply
            double ttag;
            time.get(wday,wsod,wfsod);
            ttag=double(wday-AttJDaystart-0.5)*86400+wsod+wfsod+Atttimestart;
            
            //*******************
            // choose 9 point for lagrangepoly ,change it as you need;
            int N=9;
            int mid=floor(N/2);
            //*******************
            
            
            vector<double> times(N), Q1(N), Q2(N), Q3(N), Q4(N);
            double dt,err,Qx[4];
            int line,lengtht;
            
            
            //cout<<"ttag"<<ttag<<endl;
            dt=vLEOatt[1].second-vLEOatt[0].second;
            line=floor((ttag-vLEOatt[0].second)/round(dt));
            
            lengtht=vLEOatt.size();
            
            if ((ttag<vLEOatt[N].second )
                ||(ttag > vLEOatt[lengtht-N].second ) )
            {
                  cout<<setprecision(16);
                  cout<<"The GOCE attitude inpoly time:"<<ttag<<" out of file "
                  <<"the time should in "<<vLEOatt[N].second
                  << " and " <<vLEOatt[lengtht-N].second<<endl;
                  return ;
            }
            
            
            // read data for interpoly
            
            for(int i = 0; i < N; i ++)
            {
                  
                  times[i]=vLEOatt[line-mid+i].second;
                  Q1[i]=vLEOatt[line-mid+i].q1;
                  Q2[i]=vLEOatt[line-mid+i].q2;
                  Q3[i]=vLEOatt[line-mid+i].q3;
                  Q4[i]=vLEOatt[line-mid+i].q4;
                  
                  
                  // comtinue quater
                  //        if  Qtn(i,1:3)*Qtn(i-1,1:3)'<0
                  //        Qtn(i,1:4)=-Qtn(i,1:4);
                  
                  if(i> 0)
                  {
                        err=Q1[i]*Q1[i-1]+Q2[i]*Q2[i-1]+Q3[i]*Q3[i-1];
                        if(err<0)
                        {
                              Q1[i]=-Q1[i];
                              Q2[i]=-Q2[i];
                              Q3[i]=-Q3[i];
                              Q4[i]=-Q4[i];
                        }
                        err=0;
                  }
                  
            }
            
            Qx[0]=LagrangeInterpolation(times,Q1,ttag,err);
            Qx[1]=LagrangeInterpolation(times,Q2,ttag,err);
            Qx[2]=LagrangeInterpolation(times,Q3,ttag,err);
            Qx[3]=LagrangeInterpolation(times,Q4,ttag,err);
     
            // unit Quart at time ttag
            Qx2Unitx(4 ,Qx);
            
            LEOatttag.second=ttag;
            LEOatttag.q1=Qx[0];
            LEOatttag.q2=Qx[1];
            LEOatttag.q3=Qx[2];
            LEOatttag.q4=Qx[3];
            
            
            
            
      }//end of get GOCE attitude value at time ttag   LEOatttag
      
      
      //get GOCE reciver offset value at in ICEF
      
        void LEOReciverAtt::LEOroffsetvt(CommonTime time,
                                         vector<LEOatt> vLEOatt1,
                                         vector<LEOatt> vLEOatt2,
                                         Triple &offsetRecivert)
      {
            //get GOCE attitude value at time ttag   vGOCEattag
            LEOatt  vLEOatttag1, vLEOatttag2;
            
            GetLEOattime(time,vLEOatt1,vLEOatttag1);
            GetLEOattime(time,vLEOatt2,vLEOatttag2);
     
            
            double Qx[4];
            Vector<double> offtmp(3);
            Matrix<double> Rx1,Rx2,Rx;
            Rx1.resize(3,3,0.0);
            Rx2.resize(3,3,0.0);
            Rx.resize(3,3,0.0);
            
            //prapare data
            //  get QX from file1 ,and translate to Rx1
            Qx[0]=vLEOatttag1.q1;
            Qx[1]=vLEOatttag1.q2;
            Qx[2]=vLEOatttag1.q3;
            Qx[3]=vLEOatttag1.q4;
          
            Q2Rotation(Qx,Rx1);
     
            //  get QX from file2 ,and translate to Rx2
            Qx[0]=vLEOatttag2.q1;
            Qx[1]=vLEOatttag2.q2;
            Qx[2]=vLEOatttag2.q3;
            Qx[3]=vLEOatttag2.q4;
            Q2Rotation(Qx, Rx2);
     
            //here maby change right *  or left *
            Rx=Rx2 * Rx1;
            
            // read offsetReciver in SRF
            offtmp[0]=offsetReciver[0];
            offtmp[1]=offsetReciver[1];
            offtmp[2]=offsetReciver[2];
           
            offtmp=Rx*offtmp;
               
            offsetRecivert[0]=offtmp[0];
            offsetRecivert[1]=offtmp[1];
            offsetRecivert[2]=offtmp[2];
            
      }//end of get GOCE reciver offset value at in ICEF
      
      void LEOReciverAtt::LEOroffsetvt(CommonTime time,
                                       vector<LEOatt> vLEOatt1,
                                       Matrix<double> C2T,
                                       Triple &offsetRecivert)
      {
            //get GOCE attitude value at time ttag   vGOCEattag
            LEOatt  vLEOatttag1;
            
            GetLEOattime(time,vLEOatt1,vLEOatttag1);


            double Qx[4];
            Vector<double> offtmp(3);
            Matrix<double> Rx1,Rx;
            Rx1.resize(3,3,0.0);
            Rx.resize(3,3,0.0);
            
            //prapare data
            //  get QX from file1 ,and translate to Rx1
            // usually SCC file,SRF TO IRF
            Qx[0]=vLEOatttag1.q1;
            Qx[1]=vLEOatttag1.q2;
            Qx[2]=vLEOatttag1.q3;
            Qx[3]=vLEOatttag1.q4;
            
            Q2Rotation(Qx,Rx1);
            
            //  C2T
            //give CR2T by file.irf to ECEF
            // get ReferenceSystem.hpp by UTC
            
            //here maby change right *  or left *
            Rx=C2T * Rx1;
            
            // read offsetReciver in SRF
            offtmp[0]=offsetReciver[0];
            offtmp[1]=offsetReciver[1];
            offtmp[2]=offsetReciver[2];
            
            offtmp=Rx*offtmp;
            
            offsetRecivert[0]=offtmp[0];
            offsetRecivert[1]=offtmp[1];
            offsetRecivert[2]=offtmp[2];
            
      }//end of get GOCE reciver offset value at in ICEF




} // namespace

//------------------------------------------------------------------------------------