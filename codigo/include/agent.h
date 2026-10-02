#ifndef HS_AGENT_H
#define HS_AGENT_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#define AG_ITEMS 24
#define AG_INV 12
#define AG_CHANCE 8
#define AG_OUTCOMES (1u<<AG_CHANCE)
#define AG_TYPES 4
#define AG_PLIES 20
enum { AG_PASSIVE=1, AG_CONSUME=2, AG_ETERNAL=4, AG_NEEDS_SALE=8, AG_HELD=16 };
typedef struct {
    char name[32];int type,flags,price,sale,energy_cost,cash_cost;
    double base,add,mult,bonus,prob_low,prob_high;
    int repeat,mult_type,condition_type,condition_count,cash_step;
    double cash_factor,energy_gain,cash_gain,growth;
    uint32_t requires;
} AgItem;
typedef struct {
    char name[32];AgItem item[AG_ITEMS];int item_count,capacity,max_use,rounds,players;
    double target,goal_reward,pair_bonus,energy_refresh,cash_refresh;
    bool preparations;
} AgRules;
typedef struct {int inventory[AG_INV],count;double cash,energy,growth,reward;int goals;} AgPlayer;
/* No environment seed, hidden probability, RNG tape or opponent policy in observations. */
typedef struct {AgPlayer player[2];uint32_t market;int turn,plies;} AgObservation;
enum {AG_NONE,AG_BUY,AG_SELL,AG_REPLACE,AG_SWAP};
typedef struct {uint32_t use;int prep,x,y;} AgAction;
typedef struct {double value,probability;} AgOutcome;
typedef struct {
    bool valid,exact_model;double success,expected_score,expected_reward,cash_delta,energy_delta,growth;
    double constant,coeff[AG_CHANCE],probability[AG_CHANCE];int source[AG_CHANCE],repeat_index[AG_CHANCE],chance_count;
    AgOutcome outcome[AG_OUTCOMES];unsigned outcomes;
} AgEvaluation;
typedef struct {
    uint64_t generated,invalid,evaluated,eliminated_before,defeated_after,future_states;
    uint64_t probability_terms,outcomes_enumerated,rollouts,cutoffs;
    uint64_t root_generated,root_evaluated;unsigned temporal_depth,completed_depth;
    size_t heap_live,heap_peak;double elapsed_ns;bool truncated;
} AgMetrics;
typedef struct {uint64_t evaluations;double milliseconds;unsigned depth,iterations;uint64_t rng;} AgBudget;
typedef struct {AgAction action;AgEvaluation evaluation;AgMetrics metrics;bool found,model_exact,search_complete;} AgDecision;
enum {AG_FULL,AG_NO_AGGREGATION,AG_NO_DEPENDENCIES,AG_SIMPLIFIED};
typedef bool (*AgVisitor)(AgAction action,void *context);
double ag_now(void);
uint64_t ag_random(uint64_t *rng);
double ag_uniform(uint64_t *rng);
void *ag_allocate(AgMetrics *m,size_t bytes);
void ag_release(AgMetrics *m,void *ptr);
AgItem ag_item(const char *name,int type,double base);
void ag_generate(const AgRules *r,const AgObservation *o,AgVisitor visit,void *context);
bool ag_legal(const AgRules *r,const AgObservation *o,AgAction a);
bool ag_equivalent_swap(const AgRules *r,const AgObservation *o,AgAction a);
bool ag_evaluate(const AgRules *r,const AgObservation *o,AgAction a,int mode,AgEvaluation *e,AgMetrics *m);
void ag_apply(const AgRules *r,AgObservation *o,AgAction a,const AgEvaluation *e,double score,AgMetrics *search_metrics,unsigned depth);
double ag_utility(const AgRules *r,const AgObservation *o,int root);
double ag_sample_model(const AgEvaluation *e,uint64_t *rng);
double ag_sample_environment(const AgEvaluation *e,const double true_probability[AG_ITEMS],uint64_t world_seed,int ply);
bool ag_known_deterministic(const AgRules *r);
bool ag_renewal_applicable(const AgRules *r,const AgObservation *o);
AgDecision ag_decide(const AgRules *r,const AgObservation *o,const char *policy,AgBudget budget);
bool ag_case(const char *name,unsigned variant,AgRules *r,AgObservation *o);
bool ag_load(FILE *in,AgRules *r,AgObservation *o);
void ag_save(FILE *out,const AgRules *r,const AgObservation *o);
void ag_print_action(FILE *out,AgAction a);
#endif
