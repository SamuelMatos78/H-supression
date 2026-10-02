#include "agent.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
static AgBudget unlimited={0,0,3,128,12345};
static bool same(AgAction a,AgAction b){return a.prep==b.prep&&a.x==b.x&&a.y==b.y&&a.use==b.use;}
static void close_to(double a,double b){CHECK(fabs(a-b)<1e-9);}
static void effects(void){
    AgRules r={.item_count=3,.capacity=3,.max_use=3,.rounds=1,.players=1,.target=30,.preparations=true};AgObservation o={.plies=1};
    o.player[0].count=3;o.player[0].inventory[0]=0;o.player[0].inventory[1]=1;o.player[0].inventory[2]=2;o.player[0].energy=10;
    r.item[0]=ag_item("A",0,12);r.item[1]=ag_item("B",1,0);r.item[1].mult=3;r.item[1].mult_type=0;r.item[2]=ag_item("addition",0,4);
    AgAction a={7,AG_NONE,-1,-1};AgEvaluation e;CHECK(ag_evaluate(&r,&o,a,AG_FULL,&e,NULL));close_to(e.expected_score,40);
    a=(AgAction){7,AG_SWAP,1,2};CHECK(ag_evaluate(&r,&o,a,AG_FULL,&e,NULL));close_to(e.expected_score,48);CHECK(!ag_equivalent_swap(&r,&o,a));
    r.item[1].mult=1;r.item[1].cash_step=2;CHECK(!ag_equivalent_swap(&r,&o,a));r.item[1].cash_step=0;CHECK(ag_equivalent_swap(&r,&o,a));r.item[1].mult=3;
    r.item[2].requires=1u;CHECK(ag_legal(&r,&o,a));r.item[0].sale=6;a=(AgAction){4,AG_SELL,0,-1};CHECK(!ag_legal(&r,&o,a));
    r.item[2].requires=0;r.item[2].flags=AG_NEEDS_SALE;r.item[2].energy_gain=20;CHECK(ag_evaluate(&r,&o,a,AG_FULL,&e,NULL));close_to(e.energy_delta,20);
    a.prep=AG_NONE;CHECK(!ag_legal(&r,&o,a));r.item[0].flags=AG_ETERNAL;a.prep=AG_SELL;CHECK(!ag_legal(&r,&o,a));
}
static void aggregation(void){
    AgRules r={.item_count=4,.capacity=4,.max_use=4,.rounds=1,.players=1,.target=42};AgObservation o={.plies=1};o.player[0].count=4;
    for(int i=0;i<4;i++){o.player[0].inventory[i]=i;r.item[i]=ag_item("trial",0,0);r.item[i].bonus=20;r.item[i].prob_low=r.item[i].prob_high=0.2;}
    o.player[0].growth=2;AgAction a={15,AG_NONE,-1,-1};AgEvaluation h,x;AgMetrics hm={0},xm={0};
    CHECK(ag_evaluate(&r,&o,a,AG_FULL,&h,&hm));CHECK(ag_evaluate(&r,&o,a,AG_NO_AGGREGATION,&x,&xm));close_to(h.success,0.1808);close_to(h.expected_score,18);close_to(h.success,x.success);CHECK(h.outcomes==5&&x.outcomes==16);CHECK(hm.probability_terms<xm.probability_terms);
    r.item[0].prob_low=0.1;r.item[0].prob_high=0.9;CHECK(ag_evaluate(&r,&o,a,AG_FULL,&h,NULL));CHECK(!h.exact_model);close_to(h.probability[0],0.5);
}
static void configuration_bounds(void){
    AgRules r;AgObservation o;CHECK(ag_case("branching",12,&r,&o));
    for(int i=0;i<9;i++){r.item[i].bonus=1;r.item[i].prob_low=r.item[i].prob_high=0.5;}
    FILE *f=tmpfile();CHECK(f);ag_save(f,&r,&o);rewind(f);CHECK(!ag_load(f,&r,&o));fclose(f);
}
static void policy_identity(void){
    const char *names[]={"cards","economy","stochastic","unknown","trap"};
    for(unsigned n=0;n<5;n++)for(unsigned v=0;v<4;v++){
        AgRules r;AgObservation o;CHECK(ag_case(names[n],v,&r,&o));AgObservation original=o;
        AgDecision h=ag_decide(&r,&o,"hs",unlimited),l=ag_decide(&r,&o,"lex_simple",unlimited),f=ag_decide(&r,&o,"hs_no_filter",unlimited),a=ag_decide(&r,&o,"hs_no_aggregation",unlimited);
        CHECK(h.found&&ag_legal(&r,&o,h.action));CHECK(same(h.action,l.action)&&same(h.action,f.action)&&same(h.action,a.action));
        CHECK(!memcmp(&o,&original,sizeof o));CHECK(h.metrics.future_states==0&&h.metrics.temporal_depth==0);CHECK(h.metrics.generated==h.metrics.invalid+h.metrics.evaluated+h.metrics.eliminated_before);CHECK(h.metrics.defeated_after+1==h.metrics.evaluated);
        CHECK(h.metrics.evaluated<=f.metrics.evaluated);
        AgDecision again=ag_decide(&r,&o,"hs",(AgBudget){0,0,3,128,987654});CHECK(same(h.action,again.action));
    }
}
static void temporal_counterexample(void){
    AgRules r;AgObservation o;CHECK(ag_case("tiny",0,&r,&o));AgDecision h=ag_decide(&r,&o,"hs",unlimited),q=ag_decide(&r,&o,"oracle",unlimited),b=ag_decide(&r,&o,"alphabeta",unlimited);
    CHECK(h.action.use==1u);CHECK(q.action.use==2u);CHECK(same(q.action,b.action));CHECK(q.metrics.future_states>0&&q.search_complete);CHECK(q.metrics.temporal_depth==3);
    AgDecision e=ag_decide(&r,&o,"expectimax",unlimited);CHECK(same(e.action,q.action));
    for(int policy=0;policy<2;policy++){
        AgObservation episode=o;
        while(episode.plies){AgDecision z=ag_decide(&r,&episode,policy?"oracle":"hs",unlimited);AgEvaluation ev;CHECK(ag_evaluate(&r,&episode,z.action,AG_FULL,&ev,NULL));ag_apply(&r,&episode,z.action,&ev,ev.constant,NULL,0);}
        close_to(ag_utility(&r,&episode,0),policy?130.2:23);
    }
    CHECK(ag_case("tiny_chance",0,&r,&o));q=ag_decide(&r,&o,"oracle",unlimited);CHECK(q.action.use==2u&&q.search_complete);
}
static void future_battles(void){
    for(int n=2;n<=8;n+=2){
        AgRules r;AgObservation o;CHECK(ag_case("future_renewal",(unsigned)n,&r,&o));CHECK(o.plies==20);
        AgDecision h=ag_decide(&r,&o,"hs",unlimited),x=ag_decide(&r,&o,"renewal_exact",unlimited);
        CHECK(same(h.action,x.action));CHECK(h.action.use==((1u<<n)-1));CHECK(h.metrics.future_states==0&&x.metrics.evaluated==1);
        CHECK(h.evaluation.outcomes==(1u<<n));close_to(h.evaluation.expected_score,3*n+0.5*((1u<<n)-1));
        CHECK(ag_case("future_deterministic",(unsigned)n,&r,&o));CHECK(ag_known_deterministic(&r));CHECK(ag_renewal_applicable(&r,&o));
        CHECK(ag_case("future_shared",(unsigned)n,&r,&o));CHECK(!ag_renewal_applicable(&r,&o));
        h=ag_decide(&r,&o,"hs",(AgBudget){1024,0,20,4096,1729});CHECK(h.found&&h.metrics.evaluated<=1024&&h.metrics.future_states==0);
    }
    /* Exhaustive tiny temporal oracle confirms the specialized control without
       relying on its implementation or its asymptotic claim. */
    AgRules r;AgObservation o;CHECK(ag_case("future_renewal",2,&r,&o));r.players=1;r.rounds=3;o.plies=3;
    AgDecision x=ag_decide(&r,&o,"renewal_exact",unlimited),q=ag_decide(&r,&o,"oracle",unlimited);
    CHECK(q.search_complete&&same(x.action,q.action));
    CHECK(ag_case("future_deterministic",2,&r,&o));o.plies=4;
    q=ag_decide(&r,&o,"oracle",unlimited);AgDecision b=ag_decide(&r,&o,"alphabeta",(AgBudget){0,0,4,128,7});
    CHECK(q.search_complete&&b.search_complete&&same(q.action,b.action));CHECK(b.metrics.cutoffs>0);
}
static void matches_and_io(void){
    const char *names[]={"cards","economy","stochastic","unknown","trap"};const char *policies[]={"hs","greedy","lex_simple","random","expectimax","mcts","hs_no_dependencies","hs_reward_first","hs_simplified"};
    for(unsigned k=0;k<5;k++)for(unsigned j=0;j<9;j++){
        AgRules r,read;AgObservation o,copy;CHECK(ag_case(names[k],j,&r,&o));FILE *f=tmpfile();CHECK(f);ag_save(f,&r,&o);rewind(f);CHECK(ag_load(f,&read,&copy));fclose(f);CHECK(!memcmp(&o,&copy,sizeof o));
        uint64_t rng=91;while(o.plies>0){AgDecision d=ag_decide(&read,&o,policies[j],(AgBudget){512,0,3,128,27});CHECK(d.found);CHECK(d.metrics.evaluated<=512);AgEvaluation e;CHECK(ag_evaluate(&read,&o,d.action,AG_FULL,&e,NULL));ag_apply(&read,&o,d.action,&e,ag_sample_model(&e,&rng),NULL,0);CHECK(o.player[0].cash>=0&&o.player[1].cash>=0&&o.player[0].energy>=0&&o.player[1].energy>=0);}
    }
}
int main(void){effects();aggregation();configuration_bounds();future_battles();policy_identity();temporal_counterexample();matches_and_io();puts("PASS: composite effects, dependencies, ordered multipliers, sale/consumption, aggregation, HS identity, oracle counterexamples, immutable observations, finite budgets and complete episodes");return 0;}
