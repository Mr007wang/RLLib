/*
 * Copyright 2013 Saminda Abeyruwan (saminda@cs.miami.edu)
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
 *
 * Mcar3D.h
 *
 *  Created on: Jun 29, 2012
 *      Author: sam
 */

#ifndef MCAR3D_H_
#define MCAR3D_H_

#include "RL.h"
/******************************************************************************
 *  Author: Sam Abeyruwan
 *
 *  Based on MontainCar3DSym.cc, created by Matthew Taylor
 *           (Based on MountainCar.cc, created by Adam White,
 *            created on March 29 2007.)
 *
 *    Episodic Task
 *    Reward: -1 per step
 *    Actions: Discrete
 *          0 - coast
 *          1 - left
 *          2 - right
 *          3 - down
 *          4 - up
 *
 *    State: 3D Continuous
 *          car's x-position (-1.2 to .6)
 *          car's y-position (-1.2 to .6)
 *          car's x-velocity (-.07 to .07)
 *          car's y-velocity (-.07 to .07)
 *
 ******************************************************************************
 */

template<class T>
class MCar3D: public RLProblem<T>
{
    typedef RLProblem<T> Base;
  protected:
    float xposition;
    float yposition;
    float xvelocity;
    float yvelocity;

    float offset;
    float targetPosition;

    Range<float>* positionRange;
    Range<float>* velocityRange;

  public:
    MCar3D() :
        RLProblem<T>(4, 5, 1), xposition(0), yposition(0), xvelocity(0), yvelocity(0), offset(0), targetPosition(
            0.5), positionRange(new Range<float>(-1.2, 0.5)), velocityRange(
            new Range<float>(-0.07, 0.07))
    {

      for (int a = 0; a < Base::discreteActions->dimension(); a++)
        Base::discreteActions->push_back(a, a);
      // not used
      Base::continuousActions->push_back(0, 0.0);

      for (int i = 0; i < this->dimension(); i++)
        Base::resolutions->at(i) = 6.0;
    }

    virtual ~MCar3D()
    {
      delete positionRange;
      delete velocityRange;
    }

  private:

    void set_initial_position_random()
    {
      xposition = positionRange->min()
          + Probabilistic::nextFloat() * ((positionRange->max() - 0.2) - positionRange->min());
      yposition = positionRange->min()
          + Probabilistic::nextFloat() * ((positionRange->max() - 0.2) - positionRange->min());
      xvelocity = 0.0;
      yvelocity = 0.0;
    }

    void set_initial_position_at_bottom()
    {
      xposition = 0.; //-M_PI / 6.0 + offset;
      yposition = 0.; //-M_PI / 6.0 + offset;
      xvelocity = 0.;
      yvelocity = 0.;
    }

    void update_velocity(const Action<double>* act)
    {

      switch (act->id())
      {
      case 0:
        xvelocity += cos(3 * xposition) * (-0.0025);
        yvelocity += cos(3 * yposition) * (-0.0025);
        break;
      case 1:
        xvelocity += -0.001 + cos(3 * xposition) * (-0.0025);
        yvelocity += cos(3 * yposition) * (-0.0025);
        break;
      case 2:
        xvelocity += +0.001 + cos(3 * xposition) * (-0.0025);
        yvelocity += cos(3 * yposition) * (-0.0025);
        break;
      case 3:
        xvelocity += cos(3 * xposition) * (-0.0025);
        yvelocity += -0.001 + cos(3 * yposition) * (-0.0025);
        break;
      case 4:
        xvelocity += cos(3 * xposition) * (-0.0025);
        yvelocity += +0.001 + cos(3 * yposition) * (-0.0025);
        break;
      }

      //xvelocity *= get_gaussian(1.0,std_dev_eff);
      //yvelocity *= get_gaussian(1.0,std_dev_eff);
      xvelocity = velocityRange->bound(xvelocity);
      yvelocity = velocityRange->bound(yvelocity);
    }

    void update_position()
    {
      xposition += xvelocity;
      yposition += yvelocity;
      xposition = positionRange->bound(xposition);
      yposition = positionRange->bound(yposition);
    }

    void updateRTStep()
    {
      DenseVector<T>& vars = *Base::output->o_tp1;
      vars[0] = (xposition - positionRange->min()) * Base::resolutions->at(0) / positionRange->length();
      vars[1] = (yposition - positionRange->min()) * Base::resolutions->at(1) / positionRange->length();
      vars[2] = (xvelocity - velocityRange->min()) * Base::resolutions->at(2) / velocityRange->length();
      vars[3] = (yvelocity - velocityRange->min()) * Base::resolutions->at(3) / velocityRange->length();

      Base::observations->at(0) = xposition;
      Base::observations->at(1) = yposition;
      Base::observations->at(2) = xvelocity;
      Base::observations->at(3) = yvelocity;

      Base::output->updateRTStep(r(), z(), endOfEpisode());
    }

  public:
    void initialize()
    {
//      set_initial_position_at_bottom();
      set_initial_position_random();
      updateRTStep();
    }

    void step(const Action<double>* a)
    {
      update_velocity(a);
      update_position();
      updateRTStep();
    }

    bool endOfEpisode() const
    {
      return ((xposition >= targetPosition) && (yposition >= targetPosition));
    }

    float r() const
    {
      return -1.0;
    }

    float z() const
    {
      return 0;
    }

};

#endif /* MCAR3D_H_ */
