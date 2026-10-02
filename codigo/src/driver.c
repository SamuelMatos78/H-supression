#include "agent.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
static bool policy_valid(const char *name){const char *names[]={"hs","greedy","lex_simple","random","expectimax","alphabeta","mcts","oracle","hs_no_aggregation","hs_no_dependencies","hs_reward_first","hs_no_filter","hs_simplified","renewal_exact"};for(unsigned j=0;j<sizeof names/sizeof *names;j++)if(!strcmp(name,names[j]))return true;return false;}
static void metrics(const AgMetrics *m){
    printf("\"metrics\":{\"generated\":%llu,\"invalid_candidates\":%llu,\"fully_evaluated\":%llu,\"eliminated_before\":%llu,\"defeated_after\":%llu,\"future_states\":%llu,\"temporal_depth\":%u,\"completed_depth\":%u,\"probability_terms\":%llu,\"outcomes_enumerated\":%llu,\"rollouts\":%llu,\"cutoffs\":%llu,\"root_generated\":%llu,\"root_evaluated\":%llu,\"peak_owned_heap_bytes\":%zu,\"elapsed_ns\":%.0f,\"truncated\":%s}",
    (unsigned long long)m->generated,(unsigned long long)m->invalid,(unsigned long long)m->evaluated,(unsigned long long)m->eliminated_before,(unsigned long long)m->defeated_after,(unsigned long long)m->future_states,m->temporal_depth,m->completed_depth,(unsigned long long)m->probability_terms,(unsigned long long)m->outcomes_enumerated,(unsigned long long)m->rollouts,(unsigned long long)m->cutoffs,(unsigned long long)m->root_generated,(unsigned long long)m->root_evaluated,m->heap_peak,m->elapsed_ns,m->truncated?"true":"false");
}
static void decision(const AgDecision *d,const char *policy,int ply,int seat){
    printf("{\"type\":\"decision\",\"policy\":\"%s\",\"ply\":%d,\"seat\":%d,\"action\":",policy,ply,seat);ag_print_action(stdout,d->action);
    printf(",\"success_probability\":%.17g,\"growth\":%.17g,\"cash_delta\":%.17g,\"expected_score\":%.17g,\"exact_probability_model\":%s,\"search_complete\":%s,",d->evaluation.success,d->evaluation.growth,d->evaluation.cash_delta,d->evaluation.expected_score,d->model_exact?"true":"false",d->search_complete?"true":"false");metrics(&d->metrics);
}
typedef struct {const AgRules *r;const AgObservation *o;} Audit;
static bool audit_action(AgAction a,void *ptr){Audit *s=ptr;AgEvaluation e;bool valid=ag_evaluate(s->r,s->o,a,AG_FULL,&e,NULL);printf("{\"type\":\"candidate\",\"action\":");ag_print_action(stdout,a);printf(",\"valid\":%s,\"swap_equivalent\":%s",valid?"true":"false",ag_equivalent_swap(s->r,s->o,a)?"true":"false");if(valid)printf(",\"p_goal\":%.17g,\"growth\":%.17g,\"cash_delta\":%.17g,\"energy_delta\":%.17g,\"expected_score\":%.17g,\"constant\":%.17g,\"chance_terms\":%d,\"outcomes\":%u",e.success,e.growth,e.cash_delta,e.energy_delta,e.expected_score,e.constant,e.chance_count,e.outcomes);puts("}");return true;}
int main(int argc,char **argv){
    if(argc>=2&&!strcmp(argv[1],"sizes")){printf("{\"observation_bytes\":%zu,\"evaluation_bytes\":%zu,\"decision_bytes\":%zu}\n",sizeof(AgObservation),sizeof(AgEvaluation),sizeof(AgDecision));return 0;}
    AgRules rules;AgObservation state;
    if(argc==4&&!strcmp(argv[1],"dump")){if(!ag_case(argv[2],(unsigned)strtoul(argv[3],NULL,10),&rules,&state))return 2;ag_save(stdout,&rules,&state);return 0;}
    if(argc<4){fputs("usage: dump CASE VAR | audit FILE POLICY | decide FILE POLICY EVALS MS DEPTH ITER POLICYSEED | match FILE LEFT RIGHT WORLDSEED EVALS MS DEPTH ITER POLICYSEED\n",stderr);return 2;}
    FILE *f=fopen(argv[2],"r");if(!f){perror(argv[2]);return 2;}bool loaded=ag_load(f,&rules,&state);fclose(f);if(!loaded){fputs("invalid configuration\n",stderr);return 2;}
    if((!strcmp(argv[3],"renewal_exact")||(argc==11&&!strcmp(argv[4],"renewal_exact")))&&!ag_renewal_applicable(&rules,&state)){
        puts("{\"type\":\"unsupported\",\"reason\":\"renewal_exact requires free nonnegative renewable effects and no shared market\"}");return 0;
    }
    if(!strcmp(argv[1],"audit")&&argc==4){
        if(!policy_valid(argv[3]))return 2;
        Audit audit={&rules,&state};ag_generate(&rules,&state,audit_action,&audit);
        AgBudget b={0,0,3,256,1729};AgDecision d=ag_decide(&rules,&state,argv[3],b);decision(&d,argv[3],0,state.turn);puts("}");return 0;
    }
    if(!strcmp(argv[1],"decide")&&argc==9){
        if(!policy_valid(argv[3]))return 2;
        AgBudget b={strtoull(argv[4],NULL,10),strtod(argv[5],NULL),(unsigned)strtoul(argv[6],NULL,10),(unsigned)strtoul(argv[7],NULL,10),strtoull(argv[8],NULL,10)};
        if(!strcmp(argv[3],"alphabeta")&&!ag_known_deterministic(&rules)){puts("{\"type\":\"unsupported\",\"reason\":\"alpha-beta requires a deterministic model\"}");return 0;}
        AgDecision d=ag_decide(&rules,&state,argv[3],b);decision(&d,argv[3],0,state.turn);puts("}");return 0;
    }
    if(strcmp(argv[1],"match")||argc!=11)return 2;
    const char *policies[2]={argv[3],argv[4]};if(!policy_valid(policies[0])||!policy_valid(policies[1]))return 2;
    if((!strcmp(policies[0],"alphabeta")||!strcmp(policies[1],"alphabeta"))&&!ag_known_deterministic(&rules)){puts("{\"type\":\"unsupported\",\"reason\":\"alpha-beta requires a deterministic model\"}");return 0;}
    uint64_t world=strtoull(argv[5],NULL,10);AgBudget budget={strtoull(argv[6],NULL,10),strtod(argv[7],NULL),(unsigned)strtoul(argv[8],NULL,10),(unsigned)strtoul(argv[9],NULL,10),strtoull(argv[10],NULL,10)};
    double true_p[AG_ITEMS]={0};uint64_t environment_rng=world^UINT64_C(0x91bc3d53a7);
    for(int i=0;i<rules.item_count;i++){const AgItem *it=&rules.item[i];true_p[i]=it->prob_low==it->prob_high?it->prob_low:it->prob_low+(it->prob_high-it->prob_low)*ag_uniform(&environment_rng);}
    int invalid[2]={0},ply=0;
    while(state.plies>0){
        int actor=state.turn;AgBudget b=budget;b.rng^=(uint64_t)(ply+1)*UINT64_C(0x519f37ad);
        AgDecision d=ag_decide(&rules,&state,policies[actor],b);AgEvaluation e;
        if(!ag_evaluate(&rules,&state,d.action,AG_FULL,&e,NULL)){invalid[actor]++;d.action=(AgAction){0,AG_NONE,-1,-1};if(!ag_evaluate(&rules,&state,d.action,AG_FULL,&e,NULL))return 3;}
        double observed=ag_sample_environment(&e,true_p,world,ply);
        decision(&d,policies[actor],ply,actor);printf(",\"observed_score\":%.17g,\"cash_before\":%.17g,\"energy_before\":%.17g",observed,state.player[actor].cash,state.player[actor].energy);
        ag_apply(&rules,&state,d.action,&e,observed,NULL,0);
        printf(",\"cash_after\":%.17g,\"energy_after\":%.17g,\"inventory_after\":%d}\n",state.player[actor].cash,state.player[actor].energy,state.player[actor].count);ply++;
    }
    double a=state.player[0].reward+0.1*(state.player[0].cash+state.player[0].energy),b=state.player[1].reward+0.1*(state.player[1].cash+state.player[1].energy);
    int winner=fabs(a-b)<1e-9?-1:a>b?0:1;struct rusage usage;getrusage(RUSAGE_SELF,&usage);
    printf("{\"type\":\"game\",\"policy0\":\"%s\",\"policy1\":\"%s\",\"winner\":%d,\"utility\":[%.17g,%.17g],\"reward\":[%.17g,%.17g],\"cash\":[%.17g,%.17g],\"energy\":[%.17g,%.17g],\"goals\":[%d,%d],\"invalid_actions\":[%d,%d],\"process_peak_rss_kib\":%ld,\"plies\":%d}\n",policies[0],policies[1],winner,a,b,state.player[0].reward,state.player[1].reward,state.player[0].cash,state.player[1].cash,state.player[0].energy,state.player[1].energy,state.player[0].goals,state.player[1].goals,invalid[0],invalid[1],usage.ru_maxrss,ply);
    return 0;
}
