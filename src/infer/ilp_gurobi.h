/*
 * ilp_gurobi.h
 *
 *  Created on: Sep 29, 2014
 *      Author: varun
 */

#ifndef ILP_GUROBI_H_
#define ILP_GUROBI_H_

#include "sat.h"
#include "gurobi_c++.h"

using namespace std;

typedef hash_map<GroundPredicate*, long double, HashGroundPredicate,
    EqualGroundPredicate> PredicateValue;
typedef hash_map<GroundPredicate*, GRBVar, HashGroundPredicate,
    EqualGroundPredicate> PredicateGRBVar;
typedef hash_map<GroundClause*, GRBVar, HashGroundClause,
    EqualGroundClause> ClauseGRBVar;

class ILP_Gurobi : public SAT {
 public:
  ILP_Gurobi(VariableState* state, int seed, const bool& trackClauseTrueCnts):SAT(state, seed, trackClauseTrueCnts){
    domain_ = (Domain*) state_->getDomain();
  }

  void init(){
  }

  void infer(){
    GRBEnv env = GRBEnv();
    //env.set(GRB_IntParam_OutputFlag, 0);
    //env.set(GRB_IntParam_Threads,1);
    //env.set(GRB_DoubleParam_FeasibilityTol,1e-9);
    //env.set(GRB_DoubleParam_IntFeasTol,1e-9);
    //env.set(GRB_DoubleParam_MIPGap,1e-9);
    GRBModel *model = new GRBModel(env);
    Array<int>* clauseIndices = new Array<int>;
    for (int clauseIdx = 0; clauseIdx < state_->getNumClauses(); clauseIdx++)
    {
      clauseIndices->append(clauseIdx);
    }
    PredicateValue *predVals = new PredicateValue;
    hash_map<int, bool>* addedClauses = new hash_map<int, bool>;
    long double maximizedCost;
    maximizedCost = gurobi_maximize(model, clauseIndices, state_,
                                    true, predVals, addedClauses);

    copyValsFromGurobiToState(model);

    delete model;
    delete predVals;
    delete addedClauses;
    delete clauseIndices;
  }

 protected:

  /* Perform maximization of the current active network with Gurobi.
   *
   * Inputs:
   * model          Gurobi model.
   * clauseIndices  Clauses to be added.
   * state          VariableState structure.
   * optimize       Boolean value which indicates if the maximization has
   *                to be performed or not. If it is false, just the model is
   *                constructed and the return values in maximizedCost,
   *                predVals are invalid.
   *
   * Outputs:
   * predVals       Will be filled with variable assignments after maximization.
   * addedClauses   Contains the indices of clauses that are already added to
   *                the model.
   */
  long double gurobi_maximize(GRBModel *model, Array<int>* clauseIndices,
                                          VariableState* state, bool optimize,
                                          PredicateValue* predVals, hash_map<int, bool>* addedClauses)
  {
    long double maximizedCost = 0;

    try {
      GRBEnv env = model->getEnv();
      PredicateGRBVar predVars;
      ClauseGRBVar clauseVars;

      // Create variables
      for (int atomIdx = 0; atomIdx < state->getNumAtoms(); atomIdx++)
      {
        GroundPredicate* gp = state->getGndPred(atomIdx);
        stringstream temp;
        temp << "pred_" << atomIdx;
        string predVarString = temp.str();

        try
        {
          predVars[gp] = model->getVarByName(predVarString);
          //Reset upper and lower bounds before fixing variables
          predVars[gp].set(GRB_DoubleAttr_LB, 0.0);
          predVars[gp].set(GRB_DoubleAttr_UB, 1.0);
        }
        catch (GRBException e)
        {
          //it is not already present in the model, so add it.
          predVars[gp] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, predVarString);
        }
      }

      // Add all clause variables first to update the model only once
      for (int clauseIdx = 0;
          clauseIndices != NULL && clauseIdx < clauseIndices->size();
          clauseIdx++)
      {
        stringstream temp;
        temp << "clauseVar_" << (*clauseIndices)[clauseIdx];
        string clauseVarString = temp.str();
        GroundClause *c = state->getGndClause((*clauseIndices)[clauseIdx]);

        if (!c->isHardClause() && c->getWt() != 0)
        {
          //only for the active clauses.
          //hard clauses don't need any variables.
          try
          {
            clauseVars[c] = model->getVarByName(clauseVarString);
          }
          catch (GRBException e)
          {
            clauseVars[c] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                          clauseVarString);
          }
        }
      }

      // Integrate new variables
      model->update();

      GRBQuadExpr obj;
      try
      {
        obj = model->getObjective();
      }
      catch (GRBException e)
      {
        obj = 0;
      }

      for (int clauseIdx = 0;
          clauseIndices != NULL && clauseIdx < clauseIndices->size();
          clauseIdx++)
      {
        stringstream temp;
        temp << "clause_" << (*clauseIndices)[clauseIdx];
        string clauseString = temp.str();
        GRBLinExpr clauseObj = 0;
        GroundClause *c = state->getGndClause((*clauseIndices)[clauseIdx]);

        if (c->getWt() != 0)
        {
          try
          {
            //Gurobi's get constraint by name is buggy. For example, if we
            //have already added c_1,c_3,c_2,c_4 in that order, it can't find
            //c_2 for some reason.
            model->getConstrByName(clauseString);
          }
          catch (GRBException e)
          {
            //Added clauses keeps track of already added clauses
            if (addedClauses->find((*clauseIndices)[clauseIdx]) == addedClauses->end())
            {
              for (int predIdx = 0; predIdx < c->getNumGroundPredicates();
                  predIdx++)
              {
                GroundPredicate *gp = (GroundPredicate*) c->getGroundPredicate(predIdx, (GroundPredicateHashArray*) state->getGndPredHashArrayPtr());
                if (c->getGroundPredicateSense(predIdx))
                {
                  clauseObj += predVars[gp];
                }
                else
                {
                  clauseObj += (1.0 - predVars[gp]);
                }
              }
              if (c->isHardClause() && c->getWt() > 0)
              {  //Hard clause
                model->addConstr(clauseObj >= 1.0, clauseString);
              }
              else if (c->isHardClause() && c->getWt() < 0)
              {  //Hard clause
                model->addConstr(1.0 - clauseObj >= 1.0, clauseString);
              }
              else if (c->getWt() > 0)
              {  //Pos clause
                model->addConstr(clauseObj - clauseVars[c] >= 0.0, clauseString);
                obj += c->getWt() * clauseVars[c];
              }
              else if (c->getWt() < 0)
              {  //Neg clause
                model->addConstr(clauseObj - (c->getNumGroundPredicates() * clauseVars[c]) <= 0.0,
                                 clauseString);
                obj += c->getWt() * clauseVars[c];
              }
            }
            (*addedClauses)[(*clauseIndices)[clauseIdx]] = true;
          }
        }
      }

      model->update();

      // Set objective
      model->setObjective(obj, GRB_MAXIMIZE);

      if (optimize)
      {
        // Optimize model
        model->optimize();

        int optimstatus = model->get(GRB_IntAttr_Status);
        if (optimstatus == GRB_OPTIMAL)
        {
          maximizedCost = model->get(GRB_DoubleAttr_ObjVal);
          cout << "Gurobi Obj: " << maximizedCost << endl;
          cout << "Num solutions: " << model->get(GRB_IntAttr_SolCount) << endl;
        }
        else
        {
          cout << "Infeasible Solution" << endl;
          exit(1);
        }

        // Select the solution with the maximum number of true predicates
        int bestSolNum = 0;
        int numPosInBestSol = 0;
        for (int solNum = 0; solNum < model->get(GRB_IntAttr_SolCount); solNum++)
        {
          env.set(GRB_IntParam_SolutionNumber,solNum);
          int numPosInSol = 0;
          // Get the current assignment for variables in the maximized state
          for (int atomIdx = 0; atomIdx < state->getNumAtoms(); atomIdx++)
          {
            GroundPredicate* gp = state->getGndPred(atomIdx);
            numPosInSol += predVars[gp].get(GRB_DoubleAttr_Xn) > 0.5 ? 1 : 0;
          }
          if (numPosInSol > numPosInBestSol)
          {
            numPosInBestSol = numPosInSol;
            bestSolNum = solNum;
          }
        }

        env.set(GRB_IntParam_SolutionNumber,bestSolNum);
        // Get the current assignment for variables in the maximized state
        for (int atomIdx = 0; atomIdx < state->getNumAtoms(); atomIdx++)
        {
          GroundPredicate* gp = state->getGndPred(atomIdx);
          (*predVals)[gp] = predVars[gp].get(GRB_DoubleAttr_Xn) > 0.5 ? 1 : 0;
        }
      }

    }
    catch (GRBException e)
    {
      cout << "Error code = " << e.getErrorCode() << endl;
      cout << e.getMessage() << endl;
    }
    catch (...)
    {
      cout << "Exception during optimization" << endl;
    }

    return maximizedCost;

  }

  void copyValsFromGurobiToState(GRBModel *model)
  {
    //Copy values from gurobi to state_
    for (int atomIdx = 0; atomIdx < state_->getNumAtoms(); atomIdx++)
    {
      stringstream temp;
      temp << "pred_" << atomIdx;
      string predVarString = temp.str();
      GRBVar x = model->getVarByName(predVarString);
      // the double value can sometimes be slightly disturbed from 1 or 0
      if (x.get(GRB_DoubleAttr_X) > 0.5)
      {
        state_->setValueOfAtom(atomIdx + 1, true, false, -1);
      }
      else
      {
        state_->setValueOfAtom(atomIdx + 1, false, false, -1);
      }
    }
    state_->saveLowState();
    //state_->saveLowStateToDB();
  }

 private:
  Domain* domain_;
};

#endif /* ILP_GUROBI_H_ */
