#include <boost/bind.hpp>
#include "structs/utils.hpp"

#include <ompl/geometric/planners/prm/PRM.h>
#include <ompl/geometric/planners/rrt/RRT.h>

GlobalParameters globalParameters;

#include <flann/flann.h>

#include <ompl/control/planners/rrt/RRT.h>
#include <ompl/control/planners/est/EST.h>
#include <ompl/control/planners/kpiece/KPIECE1.h>
#include <ompl/control/planners/syclop/SyclopRRT.h>
#include <ompl/control/planners/syclop/SyclopEST.h>
#include <ompl/control/planners/pdst/PDST.h>

#include "structs/filemap.hpp"

#include "domains/DynamicCarPlanning.hpp"
#include "domains/KinematicCarPlanning.hpp"
#include "domains/blimp.hpp"
#include "domains/quadrotor.hpp"
#include "domains/carsetup.hpp"
#include "domains/straightline.hpp"
#include "domains/hovercraft.hpp"
#include "domains/linkage.hpp"

#include "planners/gust.hpp"
#include "planners/RRT.hpp"
#include "planners/KPIECE.hpp"


void doBenchmarkRun(BenchmarkData benchmarkData, const FileMap &params) {
  auto planner = params.stringVal("Planner");

  ompl::base::PlannerPtr plannerPointer;
  if(planner.compare("RRT") == 0) {
    plannerPointer = ompl::base::PlannerPtr(new ompl::control::RRT( benchmarkData.simplesetup->getSpaceInformation()));
  } else if(planner.compare("KPIECE") == 0) {
    plannerPointer = ompl::base::PlannerPtr(new ompl::control::KPIECE1(benchmarkData.simplesetup->getSpaceInformation()));
  } else if(planner.compare("gust") == 0) {
    plannerPointer = ompl::base::PlannerPtr(new ompl::control::gust(benchmarkData.simplesetup->getSpaceInformation(), params));
  }
    else {
    fprintf(stderr, "unrecognized planner\n");
    return;
  }

  //allow unpenalized time for precomputation -- which is logged to the output file
  plannerPointer->setProblemDefinition(benchmarkData.simplesetup->getProblemDefinition());
  plannerPointer->solve(0);

  if(params.boolVal("AddIntermediateStates") && plannerPointer->params().hasParam("intermediate_states")) {
    plannerPointer->params().setParam("intermediate_states", "true");
  }
  benchmarkData.benchmark->addPlanner(plannerPointer);
  ompl::tools::Benchmark::Request req;
  req.maxTime = params.doubleVal("Timeout");
  req.maxMem = params.doubleVal("Memory");
  req.runCount = params.doubleVal("Runs");
  req.displayProgress = true;
  req.saveConsoleOutput = false;

  benchmarkData.benchmark->benchmark(req);
  benchmarkData.benchmark->saveResultsToFile(params.stringVal("Output").c_str());

  // If there were multiple solutions being logged to global parameters, append them to the output file
  // for some reason the OMPL benchmarking doesn't include all the solutions added, just the final one
  if(globalParameters.solutionStream.solutions.size() > 0) {
    std::ofstream outfile;
    outfile.open(params.stringVal("Output").c_str(), std::ios_base::app);

    outfile << "Solution Stream\n";

    for(const auto &solution : globalParameters.solutionStream.solutions) {
      outfile << solution.second << " " << solution.first.value() << "\n";
    }
    outfile.close();
  }	
}

int main(int argc, char *argv[]) {
  FileMap params;
  if(argc > 1) {
    params.append(argv[1]);
  } else {
    params.append(std::cin);
  }

  srand(params.integerVal("Seed"));
  flann::seed_random(params.integerVal("Seed"));
  ompl::RNG::setSeed(params.integerVal("Seed"));

  if(params.exists("NumControls"))
    howManyControls = params.integerVal("NumControls");
  auto domain = params.stringVal("Domain");
  if(domain.compare("Blimp") == 0) {
    auto benchmarkData = blimpBenchmark(params);
    streamPoint = stream3DPoint;
    doBenchmarkRun(benchmarkData, params);
  } else if(domain.compare("Quadrotor") == 0) {
    auto benchmarkData = quadrotorBenchmark(params);
    streamPoint = stream3DPoint;
    doBenchmarkRun(benchmarkData, params);
  } else if(domain.compare("KinematicCar") == 0) {
    auto benchmarkData = carBenchmark<ompl::app::KinematicCarPlanning>(params);
    streamPoint = stream2DPoint2;
    doBenchmarkRun(benchmarkData, params);
  } else if(domain.compare("DynamicCar") == 0) {
    auto benchmarkData = carBenchmark<ompl::app::DynamicCarPlanning>(params);
    streamPoint = stream2DPoint;
    doBenchmarkRun(benchmarkData, params);
  } else if(domain.compare("StraightLine") == 0) {
    auto benchmarkData = straightLineBenchmark(params);
    streamPoint = stream2DPoint2;
    streamLine = stream2DLine2;
    doBenchmarkRun(benchmarkData, params);
  }
  else if(domain.compare("Hovercraft") == 0) {
    auto benchmarkData = hovercraftBenchmark(params);
    streamPoint = stream2DPoint;
    doBenchmarkRun(benchmarkData, params);
  }
  else {
    fprintf(stderr, "unrecognized domain\n");
  }
  return 0;
}
