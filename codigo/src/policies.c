#include "agent.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
typedef struct {const AgRules *r;const AgObservation *o;AgBudget b;AgMetrics *m;double deadline;int root;} Context;
static bool exhausted(Context *c){
    if(c->m->evaluated>=c->b.evaluations||(c->deadline>0&&ag_now()>=c->deadline)){c->m->truncated=true;return true;}
    return false;
}
static void generated(Context *c,unsigned depth){c->m->generated++;if(depth==0)c->m->root_generated++;}
static bool evaluate(Context *c,const AgObservation *o,AgAction a,int mode,AgEvaluation *e,unsigned depth){
    if(!ag_evaluate(c->r,o,a,mode,e,c->m)){c->m->invalid++;return false;}
    c->m->evaluated++;if(depth==0)c->m->root_evaluated++;return true;
}
static void key(const AgEvaluation *e,bool alternative,double q[4]){
    /* Quantize only probability at 1e-12 so distribution summation order cannot
       manufacture a strict priority difference between equivalent ablations. */
    q[0]=alternative?e->expected_reward:round(e->success*1e12)/1e12;
    q[1]=e->growth;q[2]=e->cash_delta;q[3]=e->expected_score;
}
static bool greater(const double a[4],const double b[4]){for(int j=0;j<4;j++)if(a[j]!=b[j])return a[j]>b[j];return false;}
typedef struct {AgAction a;double q[4];} Candidate;
typedef struct {Context *c;AgDecision *d;int mode;bool filter,alternative,greedy,random,materialize;
    uint64_t legal;double best[4];Candidate *all;size_t count,capacity;} Immediate;
static bool immediate_visit(AgAction a,void *ptr){
    Immediate *s=ptr;Context *c=s->c;if(exhausted(c))return false;generated(c,0);
    if(!ag_legal(c->r,c->o,a)){c->m->invalid++;return true;}
    if(s->filter&&ag_equivalent_swap(c->r,c->o,a)){c->m->eliminated_before++;return true;}
    if(s->random){s->legal++;if(ag_random(&c->b.rng)%s->legal==0){s->d->action=a;s->d->found=true;}return true;}
    AgEvaluation e;if(!evaluate(c,c->o,a,s->mode,&e,0))return true;
    double q[4];key(&e,s->alternative,q);
    if(s->greedy){q[0]=e.expected_reward+0.1*(e.cash_delta+e.energy_delta);q[1]=q[2]=q[3]=0;}
    if(s->materialize){
        if(s->count==s->capacity){size_t n=s->capacity?s->capacity*2:32;Candidate *a2=ag_allocate(c->m,n*sizeof *a2);if(s->all)memcpy(a2,s->all,s->count*sizeof *a2);ag_release(c->m,s->all);s->all=a2;s->capacity=n;}
        s->all[s->count].a=a;memcpy(s->all[s->count++].q,q,sizeof q);return true;
    }
    if(s->d->found)c->m->defeated_after++;
    if(!s->d->found||greater(q,s->best)){s->d->found=true;s->d->action=a;s->d->evaluation=e;memcpy(s->best,q,sizeof q);}
    return true;
}
static void immediate(Context *c,const char *name,AgDecision *d){
    Immediate s={.c=c,.d=d,.mode=AG_FULL};
    s.filter=!strcmp(name,"hs");s.alternative=!strcmp(name,"hs_reward_first");
    s.greedy=!strcmp(name,"greedy");s.random=!strcmp(name,"random");s.materialize=!strcmp(name,"lex_simple");
    if(!strcmp(name,"hs_no_aggregation"))s.mode=AG_NO_AGGREGATION;
    if(!strcmp(name,"hs_no_dependencies"))s.mode=AG_NO_DEPENDENCIES;
    if(!strcmp(name,"hs_simplified"))s.mode=AG_SIMPLIFIED;
    /* The dependency/aggregation ablations otherwise retain the original filter. */
    if(!strncmp(name,"hs_",3)&&strcmp(name,"hs_no_filter"))s.filter=true;
    ag_generate(c->r,c->o,immediate_visit,&s);
    if(s.materialize){
        for(size_t i=0;i<s.count;i++){
            if(d->found)c->m->defeated_after++;
            if(!d->found||greater(s.all[i].q,s.best)){d->found=true;d->action=s.all[i].a;memcpy(s.best,s.all[i].q,sizeof s.best);}
        }
        ag_release(c->m,s.all);
    }
    /* Re-read the selected witness for controls that did not retain it. This
       additional evaluation is counted and fits the budget by reserving one. */
    if((s.random||s.materialize)&&d->found){ag_evaluate(c->r,c->o,d->action,AG_FULL,&d->evaluation,c->m);c->m->evaluated++;c->m->root_evaluated++;}
    d->search_complete=!c->m->truncated;
}
/* Iterative-deepening expectimax: opponent uniformly samples legal actions.
   Alpha-beta: deterministic adversarial minimax only. Oracle: exhaustive
   finite-horizon expectiminimax (min opponent), including chance outcomes. */
typedef struct {Context *c;const AgObservation *o;unsigned remaining,depth;int mode;
    double alpha,beta,best,sum;uint64_t legal;AgAction action;bool complete;} SearchFrame;
static double search(Context *,const AgObservation *,unsigned,unsigned,int,double,double,bool *);
static bool search_visit(AgAction a,void *ptr){
    SearchFrame *f=ptr;Context *c=f->c;if(exhausted(c)){f->complete=false;return false;}generated(c,f->depth);
    AgEvaluation e;if(!evaluate(c,f->o,a,AG_FULL,&e,f->depth))return true;
    double v=0;
    for(unsigned k=0;k<e.outcomes;k++)if(e.outcome[k].probability>0){
        AgObservation child=*f->o;ag_apply(c->r,&child,a,&e,e.outcome[k].value,c->m,f->depth+1);
        bool complete=true;double x=search(c,&child,f->remaining-1,f->depth+1,f->mode,f->alpha,f->beta,&complete);
        if(!complete){f->complete=false;return false;}v+=e.outcome[k].probability*x;
    }
    bool maximize=f->o->turn==c->root;
    if(!f->legal||(maximize?v>f->best:v<f->best)){f->best=v;f->action=a;}
    f->sum+=v;f->legal++;
    if(f->mode==1){
        if(maximize){if(v>f->alpha)f->alpha=v;}else if(v<f->beta)f->beta=v;
        if(f->alpha>=f->beta){c->m->cutoffs++;return false;}
    }
    return true;
}
static double search(Context *c,const AgObservation *o,unsigned remaining,unsigned depth,int mode,double alpha,double beta,bool *complete){
    if(!remaining||o->plies==0)return ag_utility(c->r,o,c->root);
    SearchFrame f={.c=c,.o=o,.remaining=remaining,.depth=depth,.mode=mode,.alpha=alpha,.beta=beta,.complete=true};
    ag_generate(c->r,o,search_visit,&f);*complete=f.complete;
    if(!f.legal)return ag_utility(c->r,o,c->root);
    return mode==0&&o->turn!=c->root?f.sum/f.legal:f.best;
}
static void tree_policy(Context *c,const char *name,AgDecision *d){
    int mode=!strcmp(name,"alphabeta")?1:!strcmp(name,"oracle")?2:0;
    if(mode==1&&!ag_known_deterministic(c->r)){d->found=false;return;}
    unsigned max_depth=!strcmp(name,"oracle")?(unsigned)c->o->plies:c->b.depth;
    if(max_depth>(unsigned)c->o->plies)max_depth=(unsigned)c->o->plies;
    for(unsigned depth=1;depth<=max_depth;depth++){
        SearchFrame f={.c=c,.o=c->o,.remaining=depth,.depth=0,.mode=mode,.alpha=-DBL_MAX,.beta=DBL_MAX,.complete=true};
        ag_generate(c->r,c->o,search_visit,&f);
        if(!f.complete){if(!d->found&&f.legal){d->found=true;d->action=f.action;}break;}
        if(f.legal){d->found=true;d->action=f.action;c->m->completed_depth=depth;}
    }
    d->search_complete=c->m->completed_depth==(unsigned)c->o->plies;
}
/* Open-loop UCT: nodes represent action histories. Every iteration resamples
   outcomes with a planner-only RNG; stochastic samples are never cached as truth.
   In this model outcomes affect reward only, so a history has stable legality. */
typedef struct {AgAction *a;size_t n,cap;} Moves;
typedef struct {Context *c;const AgObservation *o;Moves *v;unsigned depth;} Collect;
static bool collect_visit(AgAction a,void *ptr){
    Collect *s=ptr;if(exhausted(s->c))return false;generated(s->c,s->depth);
    if(!ag_legal(s->c->r,s->o,a)){s->c->m->invalid++;return true;}
    if(s->v->n==s->v->cap){size_t cap=s->v->cap?s->v->cap*2:16;AgAction *b=ag_allocate(s->c->m,cap*sizeof *b);if(s->v->a)memcpy(b,s->v->a,s->v->n*sizeof *b);ag_release(s->c->m,s->v->a);s->v->a=b;s->v->cap=cap;}
    s->v->a[s->v->n++]=a;return true;
}
typedef struct {AgAction a;Moves moves;size_t next;int first,sibling,parent;unsigned visits;double total;bool initialized;} Node;
static void mcts(Context *c,AgDecision *d){
    unsigned capacity=16;
    Node *nodes=ag_allocate(c->m,(size_t)capacity*sizeof *nodes);nodes[0].first=-1;nodes[0].parent=-1;unsigned count=1;
    for(unsigned iter=0;iter<c->b.iterations&&!exhausted(c);iter++){
        AgObservation state=*c->o;int index=0,path[AG_PLIES+1];unsigned path_n=1,depth=0;path[0]=0;bool expanded=false;
        while(state.plies>0&&!exhausted(c)){
            Node *node=&nodes[index];
            if(!node->initialized){Collect collect={c,&state,&node->moves,depth};ag_generate(c->r,&state,collect_visit,&collect);node->initialized=true;
                for(size_t j=node->moves.n;j>1;j--){size_t k=ag_random(&c->b.rng)%j;AgAction tmp=node->moves.a[j-1];node->moves.a[j-1]=node->moves.a[k];node->moves.a[k]=tmp;}}
            if(!node->moves.n||exhausted(c))break;
            int chosen=-1;AgAction action;
            if(node->next<node->moves.n){
                if(count==capacity){unsigned next=capacity*2;Node *grown=ag_allocate(c->m,(size_t)next*sizeof *grown);memcpy(grown,nodes,(size_t)count*sizeof *grown);ag_release(c->m,nodes);nodes=grown;capacity=next;node=&nodes[index];}
                action=node->moves.a[node->next++];chosen=(int)count++;Node *child=&nodes[chosen];child->a=action;child->first=-1;child->parent=index;child->sibling=node->first;node->first=chosen;expanded=true;
            }else{
                double best=-DBL_MAX;double sign=state.turn==c->root?1:-1;
                for(int j=node->first;j>=0;j=nodes[j].sibling){Node *child=&nodes[j];double mean=child->visits?child->total/child->visits:0;
                    double uct=sign*mean/100+sqrt(2*log(node->visits+1.0)/(child->visits+1e-9));if(uct>best){best=uct;chosen=j;}}
                if(chosen<0)break;
                action=nodes[chosen].a;
            }
            AgEvaluation e;if(!evaluate(c,&state,action,AG_FULL,&e,depth))abort();
            ag_apply(c->r,&state,action,&e,ag_sample_model(&e,&c->b.rng),c->m,++depth);index=chosen;path[path_n++]=index;
            if(expanded)break;
        }
        /* Uniform legal rollout, with all model evaluations charged. */
        while(state.plies>0&&!exhausted(c)){
            Moves v={0};Collect collect={c,&state,&v,depth};ag_generate(c->r,&state,collect_visit,&collect);
            if(!v.n||exhausted(c)){ag_release(c->m,v.a);break;}AgAction a=v.a[ag_random(&c->b.rng)%v.n];ag_release(c->m,v.a);
            AgEvaluation e;if(!evaluate(c,&state,a,AG_FULL,&e,depth))abort();ag_apply(c->r,&state,a,&e,ag_sample_model(&e,&c->b.rng),c->m,++depth);
        }
        double reward=ag_utility(c->r,&state,c->root);
        for(unsigned j=0;j<path_n;j++){nodes[path[j]].visits++;nodes[path[j]].total+=reward;}c->m->rollouts++;
    }
    unsigned visits=0;double best=-DBL_MAX;
    for(int j=nodes[0].first;j>=0;j=nodes[j].sibling){double mean=nodes[j].visits?nodes[j].total/nodes[j].visits:-DBL_MAX;
        if(!d->found||nodes[j].visits>visits||(nodes[j].visits==visits&&mean>best)){d->found=true;d->action=nodes[j].a;visits=nodes[j].visits;best=mean;}}
    for(unsigned j=0;j<count;j++)ag_release(c->m,nodes[j].moves.a);
    ag_release(c->m,nodes);
    d->search_complete=false;
}
/* Specialized control, NOT HS: free nonnegative renewable effects make the
   full current set pointwise dominant at every turn, for any remaining horizon. */
bool ag_renewal_applicable(const AgRules *r,const AgObservation *o){
    if(r->preparations||o->market||r->pair_bonus<0||r->goal_reward<0)return false;
    for(int p=0;p<r->players;p++)if(o->player[p].count>r->max_use||o->player[p].growth<0)return false;
    for(int i=0;i<r->item_count;i++){
        const AgItem *it=&r->item[i];
        if(it->flags||it->energy_cost||it->cash_cost||it->base<0||it->add<0||it->mult!=1||it->bonus<0||it->requires||it->condition_type>=0||it->cash_step||it->cash_gain||it->energy_gain||it->growth||it->prob_low!=it->prob_high)return false;
    }
    return true;
}
static void renewal_exact(Context *c,AgDecision *d){
    if(!ag_renewal_applicable(c->r,c->o))return;
    AgAction a={0,AG_NONE,-1,-1};const AgPlayer *p=&c->o->player[c->root];
    for(int i=0;i<p->count;i++)a.use|=1u<<p->inventory[i];
    generated(c,0);d->found=evaluate(c,c->o,a,AG_FULL,&d->evaluation,0);d->action=a;d->search_complete=d->found;
}
AgDecision ag_decide(const AgRules *r,const AgObservation *o,const char *policy,AgBudget budget){
    double start=ag_now();AgDecision d={0};d.action=(AgAction){0,AG_NONE,-1,-1};
    if(!budget.evaluations)budget.evaluations=UINT64_MAX;
    Context c={r,o,budget,&d.metrics,budget.milliseconds>0?start+budget.milliseconds*1e6:0,o->turn};
    bool tree=!strcmp(policy,"expectimax")||!strcmp(policy,"alphabeta")||!strcmp(policy,"oracle"),mc=!strcmp(policy,"mcts");
    if((tree||mc)&&c.b.evaluations!=UINT64_MAX)c.b.evaluations--;
    if(!strcmp(policy,"renewal_exact"))renewal_exact(&c,&d);else if(tree)tree_policy(&c,policy,&d);else if(mc)mcts(&c,&d);else {
        /* Reserve a final witness evaluation for the two materialized controls. */
        bool reserve=!strcmp(policy,"lex_simple")||!strcmp(policy,"random");if(reserve&&c.b.evaluations!=UINT64_MAX)c.b.evaluations--;
        immediate(&c,policy,&d);
    }
    if(!d.found){d.action=(AgAction){0,AG_NONE,-1,-1};d.found=ag_legal(r,o,d.action);d.metrics.truncated=true;}
    /* Search retains a root action, not its local evaluation. Witness construction
       is charged separately and budget reservation avoids granting free work. */
    if(tree||mc||!d.evaluation.valid){ag_evaluate(r,o,d.action,AG_FULL,&d.evaluation,&d.metrics);d.metrics.evaluated++;d.metrics.root_evaluated++;}
    d.model_exact=d.evaluation.exact_model&&strcmp(policy,"hs_no_dependencies")&&strcmp(policy,"hs_simplified");d.metrics.elapsed_ns=ag_now()-start;
    if(d.metrics.heap_live)abort();
    return d;
}
