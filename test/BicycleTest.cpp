/*
 * BicycleTest.cpp
 *
 *  Created on: Dec 2, 2013
 *      Author: sam
 */

#include "BicycleTest.h"

BicycleProjector::BicycleProjector()
{
  /**
   * Number of inputs | Tiling type | Number of intervals | Number of tilings
   *              5   |     1D      |       8             |       8
   *                  |     1D      |       2             |       4
   *                  |     2D      |       4             |       4
   *                  |     2D + 1  |       4             |       4
   */
  nbTiles = 5 * (8 + 4 + 4 + 4);
  memory = 5 * (8 * 8 + 2 * 4 + 4 * 4 * 4 + 4 * 4 * 4) * 9/*nbActions*/* 8/*to hash*/;
  vector = new SVector<double>(memory + 1/*bias unit*/, nbTiles + 1/*bias unit*/);
  hashing = new MurmurHashing(memory);
  tiles = new Tiles<double>(hashing);
}

BicycleProjector::~BicycleProjector()
{
  delete vector;
  delete hashing;
  delete tiles;
}

const Vector<double>* BicycleProjector::project(const Vector<double>* x, int h1)
{
  vector->clear();
  if (x->empty())
    return vector;

  int h2 = 0;
  // IRdistance
  for (int i = 0; i < x->dimension(); i++)
  {
    tiles->tiles1(vector, 8, memory, x->getEntry(i) * 8, h1, h2++);
    tiles->tiles1(vector, 4, memory, x->getEntry(i) * 2, h1, h2++);

    int j = (i + 1) % x->dimension();
    tiles->tiles2(vector, 4, memory, x->getEntry(i) * 4, x->getEntry(j) * 4, h1, h2++);

    j = (i + 2) % x->dimension();
    tiles->tiles2(vector, 4, memory, x->getEntry(i) * 4, x->getEntry(j) * 4, h1, h2++);
  }

  vector->setEntry(vector->dimension() - 1, 1.0f);
  return vector;
}

const Vector<double>* BicycleProjector::project(const Vector<double>* x)
{
  vector->clear();
  if (x->empty())
    return vector;

  int h2 = 0;
  for (int i = 0; i < x->dimension(); i++)
  {
    tiles->tiles1(vector, 8, memory, x->getEntry(i) * 8, h2++);
    tiles->tiles1(vector, 4, memory, x->getEntry(i) * 2, h2++);

    int j = (i + 1) % x->dimension();
    tiles->tiles2(vector, 4, memory, x->getEntry(i) * 4, x->getEntry(j) * 4, h2++);

    j = (i + 2) % x->dimension();
    tiles->tiles2(vector, 4, memory, x->getEntry(i) * 4, x->getEntry(j) * 4, h2++);
  }

  vector->setEntry(vector->dimension() - 1, 1.0f);
  return vector;
}

double BicycleProjector::vectorNorm() const
{
  return nbTiles + 1;
}

int BicycleProjector::dimension() const
{
  return vector->dimension();
}

void BicycleTest::testBicycleBalance()
{
  Probabilistic::srand(0);
  RLProblem<double>* problem = new RandlovBike;
  Projector<double>* projector = new BicycleProjector;
  StateToStateAction<double>* toStateAction = new StateActionTilings<double>(projector,
      problem->getDiscreteActions());
  Trace<double>* e = new ATrace<double>(projector->dimension());
  double alpha = 0.1 / projector->vectorNorm();
  double gamma = 0.99;
  double lambda = 0.95;
  Sarsa<double>* sarsa = new SarsaTrue<double>(alpha, gamma, lambda, e);
  double epsilon = 0.01;
  Policy<double>* acting = new EpsilonGreedy<double>(sarsa, problem->getDiscreteActions(), epsilon);
  OnPolicyControlLearner<double>* control = new SarsaControl<double>(acting, toStateAction, sarsa);

  RLAgent<double>* agent = new LearnerAgent<double>(control);
  Simulator<double>* sim = new Simulator<double>(agent, problem, 100000, 255, 1);
  sim->run();
  control->persist("visualization/bicycle_balance.dat");
  control->reset();
  control->resurrect("visualization/bicycle_balance.dat");
  sim->runEvaluate(10);

  delete problem;
  delete projector;
  delete toStateAction;
  delete e;
  delete sarsa;
  delete acting;
  delete control;
  delete agent;
  delete sim;
}

void BicycleTest::run()
{
  testBicycleBalance();
}

RLLIB_TEST_MAKE(BicycleTest)