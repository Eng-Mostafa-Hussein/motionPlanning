#pragma once
//#define STREAM_GRAPHICS

#include "ompl/control/planners/PlannerIncludes.h"
#include "ompl/datastructures/NearestNeighbors.h"
#include "ompl/base/goals/GoalSampleableRegion.h"
#include "ompl/tools/config/SelfConfig.h"
#include "../samplers/guststatesampler.hpp"
#include <limits>

namespace ompl {

namespace control {

class gust : public ompl::control::RRT {
public:

	/** \brief Constructor */
	gust(const SpaceInformationPtr &si, const FileMap &params) :
		ompl::control::RRT(si), gustsampler_(NULL), params(params) {

		setName("gust");

		Planner::declareParam<double>("state_radius", this, &gust::ignoreSetterDouble, &gust::getStateRadius);
		Planner::declareParam<double>("alpha", this, &gust::ignoreSetterDouble, &gust::getAlpha);
		Planner::declareParam<double>("b", this, &gust::ignoreSetterDouble, &gust::getB);
		Planner::declareParam<double>("prm_size", this, &gust::ignoreSetterUnsigedInt, &gust::getPRMSize);
		Planner::declareParam<double>("num_prm_edges", this, &gust::ignoreSetterUnsigedInt, &gust::getNumPRMEdges);
        Planner::declareParam<double>("delta", this, &gust::ignoreSetterDouble, &gust::getdelta);
        Planner::declareParam<double>("beta", this, &gust::ignoreSetterDouble, &gust::getbeta);
        Planner::declareParam<int>("usesplit", this, &gust::ignoreSetterUnsigedInt , &gust::getusesplit);


		//Obviously this isn't really a parameter but I have no idea how else to get it into the output file through the benchmarker
		Planner::declareParam<double>("sampler_initialization_time", this, &gust::ignoreSetterDouble, &gust::getSamplerInitializationTime);
	}

	virtual ~gust() {}

	void ignoreSetterDouble(double) const {}
	void ignoreSetterUnsigedInt(unsigned int) const {}
    void ignoreSetterInt(int) const {}

    double getdelta() const {
        return params.doubleVal("Delta");
    }
    double getbeta() const {
        return params.doubleVal("Beta");
    }
    unsigned int getusesplit() const {
        return params.boolVal("UseSplit");
    }

    double getStateRadius() const {
		return params.doubleVal("StateRadius");
	}

	double getAlpha() const {
		return params.doubleVal("Alpha");
	}
	double getB() const {
		return params.doubleVal("B");
	}
	unsigned int getPRMSize() const {
		return params.integerVal("PRMSize");
	}
	unsigned int getNumPRMEdges() const {
		return params.integerVal("NumEdges");
	}
	double getSamplerInitializationTime() const {
		return samplerInitializationTime;
	}

	/** \brief Continue solving for some amount of time. Return true if solution was found. */
	virtual base::PlannerStatus solve(const base::PlannerTerminationCondition &ptc)
	{
		int counter=0;
        int region_id=0;
		checkValidity();
		base::Goal* goal = pdef_->getGoal().get();
		base::GoalSampleableRegion *goal_s = dynamic_cast<base::GoalSampleableRegion *>(goal);

		while(const base::State *st = pis_.nextStart()) {
			Motion *motion = new Motion(siC_);
			si_->copyState(motion->state, st);
			siC_->nullControl(motion->control);
			nn_->add(motion);
		}

		if(nn_->size() == 0) {
			OMPL_ERROR("%s: There are no valid initial states!", getName().c_str());
			return base::PlannerStatus::INVALID_START;
		}

		if(!gustsampler_) {
			auto start = clock();

			gustsampler_ = new ompl::base::guststatesampler((ompl::base::SpaceInformation *)siC_, pdef_->getStartState(0), pdef_->getGoal(),
			        params);
			gustsampler_->initialize();

			samplerInitializationTime = (double)(clock() - start) / CLOCKS_PER_SEC;
		}
		if(!controlSampler_)
			controlSampler_ = siC_->allocDirectedControlSampler();

		OMPL_INFORM("%s: Starting planning with %u states already in data structure", getName().c_str(), nn_->size());

		Motion *solution  = NULL;
		Motion *approxsol = NULL;
		double  approxdif = std::numeric_limits<double>::infinity();

		Motion      *rmotion = new Motion(siC_);
		base::State  *rstate = rmotion->state;
		Control       *rctrl = rmotion->control;
		base::State  *xstate = si_->allocState();


		while(ptc == false)
		{
            region_id=-1;
			counter++;
            gustsampler_->sample(rstate);
            region_id=gustsampler_->current_region_id;

            #ifdef STREAM_GRAPHICS
                streamPoint(rmotion->state, 0, 1, 0, 1);
            #endif

			/* find closest state in the tree */
			Motion *nmotion = nn_->nearest(rmotion);
			/* sample a random control that attempts to go towards the random state, and also sample a control duration */
			unsigned int cd = controlSampler_->sampleTo(rctrl, nmotion->control, nmotion->state, rmotion->state);

			if(addIntermediateStates_)
			{

				// this code is contributed by Jennifer Barry
				std::vector< base::State* > pstates;
				cd = siC_->propagateWhileValid(nmotion->state, rctrl, cd, pstates, true);

				if(cd >= siC_->getMinControlDuration())
                {
					Motion *lastmotion = nmotion;
					bool solved = false;
					size_t p = 0;
					for(; p < pstates.size(); ++p)
					{
						gustsampler_->reached(pstates[p]);

                    #ifdef STREAM_GRAPHICS
						streamPoint(pstates[p], 1, 0, 0, 1);
                    #endif

						/* create a motion */
						Motion *motion = new Motion();
						motion->state = pstates[p];
						//we need multiple copies of rctrl
						motion->control = siC_->allocControl();
						siC_->copyControl(motion->control, rctrl);
						motion->steps = 1;
						motion->parent = lastmotion;
						lastmotion = motion;
						nn_->add(motion);
						double dist = 0.0;
						solved = goal->isSatisfied(motion->state, &dist);
						if(solved) {
							approxdif = dist;
							solution = motion;
							break;
						}
						if(dist < approxdif) {
							approxdif = dist;
							approxsol = motion;
						}
					}

					//free any states after we hit the goal
					while(++p < pstates.size())
						si_->freeState(pstates[p]);
					if(solved)
						break;
				}
                else
					for(size_t p = 0 ; p < pstates.size(); ++p)
						si_->freeState(pstates[p]);
			}
			else
			{
				if(cd >= siC_->getMinControlDuration()) {
					/* create a motion */
					Motion *motion = new Motion(siC_);

					si_->copyState(motion->state, rmotion->state);
					siC_->copyControl(motion->control, rctrl);
					motion->steps = cd;
					motion->parent = nmotion;

					gustsampler_->reached(motion->state);

                    #ifdef STREAM_GRAPHICS
					    streamPoint(nmotion->state, 1, 0, 0, 1);
					    streamPoint(motion->state, 1, 0, 0, 1);
                    #endif
					nn_->add(motion);
					double dist = 0.0;
					bool solv = goal->isSatisfied(motion->state, &dist);
					if(solv) {
						approxdif = dist;
						solution = motion;
						break;
					}
					if(dist < approxdif) {
						approxdif = dist;
						approxsol = motion;
					}
				}
			}
            /*Split region calling*/
            if (region_id>=0 )
                gustsampler_->split_regions(region_id);
		}

		bool solved = false;
		bool approximate = false;
		if(solution == NULL) {
			solution = approxsol;
			approximate = true;
		}

		if(solution != NULL) {
			lastGoalMotion_ = solution;

			/* construct the solution path */
			std::vector<Motion *> mpath;
			while(solution != NULL) {
				mpath.push_back(solution);
				solution = solution->parent;
			}

			/* set the solution path */
			PathControl *path = new PathControl(si_);
			for(int i = mpath.size() - 1 ; i >= 0 ; --i)
				if(mpath[i]->parent)
					path->append(mpath[i]->state, mpath[i]->control, mpath[i]->steps * siC_->getPropagationStepSize());
				else
					path->append(mpath[i]->state);
			solved = true;
			pdef_->addSolutionPath(base::PathPtr(path), approximate, approxdif, getName());
		}

		if(rmotion->state)
			si_->freeState(rmotion->state);
		if(rmotion->control)
			siC_->freeControl(rmotion->control);
		delete rmotion;
		si_->freeState(xstate);

		OMPL_INFORM("%s: Created %u states", getName().c_str(), nn_->size());

		std::cout<<"No of regions  = "<<gustsampler_->get_region_size()<<std::endl;

		return base::PlannerStatus(solved, approximate);

	}

	virtual void clear() {
		RRT::clear();
	}
protected:

	ompl::base::guststatesampler *gustsampler_;
	const FileMap &params;
	double samplerInitializationTime = 0;
};

}
}